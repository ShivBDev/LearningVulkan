#include "../shared.hpp"
#include "../Logging.hpp"
#include "VulkanEngine.hpp"
#include "VulkanSettings.hpp"
#include "VulkanUtils.hpp"
#include "glslang/Include/glslang_c_interface.h"
#include <iostream>
#include <filesystem>
#include <fstream>

namespace {
  static VKAPI_ATTR VkBool32 VKAPI_CALL VK_DebugCallback (
  VkDebugUtilsMessageSeverityFlagBitsEXT _severity,
  VkDebugUtilsMessageTypeFlagsEXT _type,
  const VkDebugUtilsMessengerCallbackDataEXT* _callbackData,
  void * _userData ) {
    std::string msg = std::format("Dbg Callback | Severity: {} | Type: {}\n  Msg: {}",
      __vk_msg_severity_str[_severity],
      __vk_msg_type_str[_type],
      _callbackData->pMessage);
    switch(_severity) {
      case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT : // fall thru, both log and dbg go to our dbg
      case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT : Logging::Debug(msg); break;
      case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT : Logging::Warn(msg); break;
      case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT : throw Logging::Error(msg);
      default: Logging::Error(std::format("Unknown Msg Severity: {}", msg)); break;
    }
    return VK_FALSE;
  }

  uint32_t VK_ChooseImageCount(VkSurfaceCapabilitiesKHR const & _surfaceCapabilities) {
    uint32_t reqImgCt = _surfaceCapabilities.minImageCount + 1;
    uint32_t finalImgCt = (_surfaceCapabilities.maxImageCount > 0 && reqImgCt > _surfaceCapabilities.maxImageCount)
      ? _surfaceCapabilities.maxImageCount
      : reqImgCt;
    Logging::Debug(std::format("Surface Image Count: {}", finalImgCt));
    return finalImgCt;
  }

  VkPresentModeKHR VK_ChoosePresentMode(std::vector<VkPresentModeKHR> const & _presentModes) {
    auto const modeIter = std::find(_presentModes.begin(), _presentModes.end(), __vk_target_present_mode);
    VkPresentModeKHR mode = modeIter != _presentModes.end() ? *modeIter : __vk_default_present_mode;
    // TODO: Add logging, convert mode enum to str
    return mode;
  }

  VkSurfaceFormatKHR VK_ChooseSurfaceFormatAndColorSpace(std::vector<VkSurfaceFormatKHR> const & _surfaceFormats) {
    auto const sFmt = std::find_if(_surfaceFormats.begin(), _surfaceFormats.end(),
      [](VkSurfaceFormatKHR const & s) {
        return s.format == __vk_target_surface_format
          && s.colorSpace == __vk_target_color_space;
      });
    return sFmt != _surfaceFormats.end() ? *sFmt : _surfaceFormats.front();
  }

  VkImageView VK_CreateImageView(
    VkDevice _device,
    VkImage _img,
    VkFormat _fmt,
    VkImageAspectFlags _aspectFlags = __vk_img_view_default_aspect_flags,
    VkImageViewType _imgViewType = __vk_img_view_default_type,
    uint32_t _layerCount = __vk_img_view_default_layer_count,
    uint32_t _mipLevels = __vk_img_view_default_mip_levels)
  {
    VkImageViewCreateInfo imageViewCreateInfo {
      .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .image = _img,
      .viewType = _imgViewType,
      .format = _fmt ,
      .components = __vk_img_view_create_info_components,
      .subresourceRange {
        .aspectMask = _aspectFlags,
        .baseMipLevel = 0,
        .levelCount = _mipLevels,
        .baseArrayLayer = 0,
        .layerCount = _layerCount
      }
    };
    VkImageView imgView {};
    VkResult result = vkCreateImageView(_device, &imageViewCreateInfo, nullptr, &imgView);
    Vk_CheckResult(result, "Failed to Create Image View");
    return imgView;
  }

  bool ReadFile(std::filesystem::path const & _path, std::vector<char>& _dataOut, bool nullTerm = true) {
    std::error_code err {};
    uintmax_t fileSz { std::filesystem::file_size(_path, err) };
    if(err) {
      Logging::Error(std::format("File Size Check Err: {}", _path.string()));
      return false;
    }
    if(fileSz == 0) {
      Logging::Warn(std::format("File Empty: {}", _path.string()));
      _dataOut.clear();
      return true;
    }
    std::ifstream file { _path };
    if(file.fail()) {
      Logging::Error(std::format("File Open Fail: {}", _path.string()));
      return false;
    }
    _dataOut.resize(fileSz);
    file.read(_dataOut.data(), fileSz);
    if(nullTerm && _dataOut.back() != '\0') { 
      _dataOut.push_back('\0');
    }
    return true;
  }

  glslang_stage_t ShaderStageFromFilename(std::string const & _filename) {
    std::string ext { std::filesystem::path(_filename).extension().string() };
    if(ext == ".vert") { return GLSLANG_STAGE_VERTEX; }
    else if(ext == ".frag") { return GLSLANG_STAGE_FRAGMENT; }
    else if(ext == ".comp") { return GLSLANG_STAGE_COMPUTE; }
    else if(ext == ".geom") { return GLSLANG_STAGE_GEOMETRY; }
    else if(ext == ".tesc") { return GLSLANG_STAGE_TESSCONTROL; }
    else if(ext == ".tese") { return GLSLANG_STAGE_TESSEVALUATION; }
    throw Logging::Error(std::format("Unknown shader file extension: {}", ext));
  }

  // Preload from SPIR-V binaries
  VkShaderModule CreateShaderModuleFromBinary(VkDevice const & _device, std::string const & _filePath) {
    std::vector<char> shaderCode {};
    if(!ReadFile(_filePath, shaderCode)) {
      throw Logging::Error(std::format("Failed to read shader file: {}", _filePath));
    }
    VkShaderModule shaderModule {};
    VkShaderModuleCreateInfo shaderModuleCreateInfo {
      .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
      .pNext = nullptr, .flags = 0,
      .codeSize = shaderCode.size(),
      .pCode = reinterpret_cast<uint32_t const *>(shaderCode.data())
    };
    VkResult result = vkCreateShaderModule(_device, &shaderModuleCreateInfo, nullptr, &shaderModule);
    Vk_CheckResult(result, std::format("Failed to create shader module for file: {}", _filePath));
    return shaderModule;
  }
  // Compile from GLSL/HLSL source at runtime
  VkShaderModule CreateShaderModuleFromText(VkDevice const & _device, std::string const & _filePath) {
    std::vector<char> shaderSrc {};
    if(!ReadFile(_filePath, shaderSrc)) {
      throw Logging::Error(std::format("Failed to read shader file: {}", _filePath));
    }
    VkShaderModule shaderModule {};
    std::vector<uint32_t> shaderCode{};
    glslang_stage_t shaderStage { ShaderStageFromFilename(_filePath) };
    glslang_initialize_process();

    glslang_input_t shaderInput {
      .language = GLSLANG_SOURCE_GLSL,
      .stage = shaderStage,
      .client = GLSLANG_CLIENT_VULKAN,
      .client_version = GLSLANG_TARGET_VULKAN_1_4,
      .target_language = GLSLANG_TARGET_SPV,
      .target_language_version = GLSLANG_TARGET_SPV_1_5,
      .code = shaderSrc.data(),
      .default_version = 450,
      .default_profile = GLSLANG_NO_PROFILE,
      .force_default_version_and_profile = false,
      .forward_compatible = false,
      .messages = GLSLANG_MSG_DEFAULT_BIT
    };
    glslang_shader_t* shader = glslang_shader_create(&shaderInput);
    if(!shader) {
      throw Logging::Error(std::format("Failed to create glslang shader for file: {}", _filePath));
    }
    if(!glslang_shader_preprocess(shader, &shaderInput)){
      std::string infoLog = glslang_shader_get_info_log(shader);
      std::string debugLog = glslang_shader_get_info_debug_log(shader);
      throw Logging::Error(std::format(
        "GLSL Preprocess Failed for file: {}\nInfo Log: {}\nDebug Log: {}",
        _filePath, infoLog, debugLog));
    }
    if(!glslang_shader_parse(shader, &shaderInput)){
      std::string infoLog = glslang_shader_get_info_log(shader);
      std::string debugLog = glslang_shader_get_info_debug_log(shader);
      throw Logging::Error(std::format(
        "GLSL Parse Failed for file: {}\nInfo Log: {}\nDebug Log: {}",
        _filePath, infoLog, debugLog));
    }

    glslang_program_t* program = glslang_program_create();
    if(!program) {
      throw Logging::Error(std::format("Failed to create glslang program for file: {}", _filePath));
    }
    glslang_program_add_shader(program, shader);
    if(!glslang_program_link(program, GLSLANG_MSG_SPV_RULES_BIT | GLSLANG_MSG_VULKAN_RULES_BIT)) {
      std::string infoLog = glslang_program_get_info_log(program);
      std::string debugLog = glslang_program_get_info_debug_log(program);
      throw Logging::Error(std::format(
        "GLSL Link Failed for file: {}\nInfo Log: {}\nDebug Log: {}",
        _filePath, infoLog, debugLog));
    }
    glslang_program_SPIRV_generate(program, shaderStage);
    size_t spirvSize = glslang_program_SPIRV_get_size(program);
    shaderCode.resize(spirvSize);
    glslang_program_SPIRV_get(program, shaderCode.data());
    std::string spirvMsgs = glslang_program_SPIRV_get_messages(program);
    if(!spirvMsgs.empty()) {
      Logging::Warn(std::format("GLSL SPIR-V Generation Messages for file: {}\nMessages: {}", _filePath, spirvMsgs));
    }

    VkShaderModuleCreateInfo shaderModuleCreateInfo {
      .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
      .pNext = nullptr, .flags = 0,
      .codeSize = shaderCode.size() * sizeof(uint32_t),
      .pCode = shaderCode.data()
    };
    VkResult result = vkCreateShaderModule(_device, &shaderModuleCreateInfo, nullptr, &shaderModule);
    Vk_CheckResult(result, std::format("Failed to create shader module for file: {}", _filePath));

    glslang_program_delete(program);
    glslang_shader_delete(shader);
    
    glslang_finalize_process();
    return shaderModule;
  }

} // locals

void VulkanEngine::CreateVulkanInstance() {
  Logging::Debug("Creating Vulkan Instance...");
  VkResult result = vkCreateInstance(&__vk_instance_create_info, nullptr, &vk_instance);
  Vk_CheckResult(result, std::format("Failed to crete Vk Instance: {}!", uint32_t(result)));
  Logging::Debug("Vulkan Instance Created.");
}

void VulkanEngine::CreateDbgCallback() {
  Logging::Debug("Creating Vulkan Debug Callback...");
  VkDebugUtilsMessengerCreateInfoEXT const __vk_dbg_util_messenger_create_info {
    .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
    .pNext = nullptr,
    .messageSeverity = __vk_msg_types,
    .messageType = __vk_msg_severities,
    .pfnUserCallback = &VK_DebugCallback,
    .pUserData = nullptr
  };
  PFN_vkCreateDebugUtilsMessengerEXT vkCreateDbgUtilsMessenger = nullptr;
  vkCreateDbgUtilsMessenger = PFN_vkCreateDebugUtilsMessengerEXT(vkGetInstanceProcAddr(vk_instance, "vkCreateDebugUtilsMessengerEXT"));
  if(!vkCreateDbgUtilsMessenger) {
    throw Logging::Error("Failed to init Vk Debug Utils Messenger!");
  }
  VkResult result = vkCreateDbgUtilsMessenger(vk_instance, &__vk_dbg_util_messenger_create_info, nullptr, &vk_dbgMessenger);
  Vk_CheckResult(result, "Failed to init Vk Debug Utils Messenger!");
  Logging::Debug("Vulkan Debug Callback Created.");
}

void VulkanEngine::CreateSurface() {
  Logging::Debug("Creating Vulkan Surface with Glfw Window...");
  VkResult result = glfwCreateWindowSurface(vk_instance, glfw_window, nullptr, &vk_surface);
  Vk_CheckResult(result, "Failed to init Glfw/Vk Surface!");
  Logging::Debug("Vulkan Surface with Glfw Window Created.");
}

void VulkanEngine::CreateDevice() {
  Logging::Debug("Creating Vulkan Logical Device...");
  std::vector<float> queuePriorities { 1.0f };
  VkDeviceQueueCreateInfo const __vk_device_queue_create_info {
    .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
    .pNext = nullptr,
    .flags = 0,
    .queueFamilyIndex = vk_queue_family,
    .queueCount = 1,
    .pQueuePriorities = &queuePriorities[0]
  };
  // Geometry Shaders not supported w/ MoltenVK
  // Use compute shaders and indirect render
  PhysicalDevice const & selectedDevice = vk_physical_devices.Selected();
  if(selectedDevice.features.geometryShader == VK_FALSE)
  {
    Logging::Warn("Geometry Shader Not Supported, Use Compute Shader!");
    use_compute_shaders = true;
  }
  if(selectedDevice.features.tessellationShader == VK_FALSE)
  { throw Logging::Error("Tessellation Shader Not Supportd!"); }

  VkPhysicalDeviceFeatures const deviceFeatures {
    .geometryShader = use_compute_shaders ? VK_FALSE : VK_TRUE,
    .tessellationShader = VK_TRUE
  };

  VkDeviceCreateInfo const deviceCreateInfo {
    .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
    .pNext = nullptr,
    .flags = 0,
    .queueCreateInfoCount = 1,
    .pQueueCreateInfos = &__vk_device_queue_create_info,
    .enabledExtensionCount = uint32_t(__vk_device_extensions.size()),
    .ppEnabledExtensionNames = __vk_device_extensions.data(),
    .pEnabledFeatures = &deviceFeatures
  };

  VkResult result = vkCreateDevice(selectedDevice.vk_device, &deviceCreateInfo, nullptr, &vk_device);
  Vk_CheckResult(result, "Failed to create logical device from physical device!");
  Logging::Debug("Vulkan Logical Device Created.");
}

void VulkanEngine::CreateSwapChain() {
  Logging::Debug("Creating Vulkan Swap Chain...");
  PhysicalDevice const& selectedDevice = vk_physical_devices.Selected();

  VkSurfaceCapabilitiesKHR const & surfaceCapabilities = selectedDevice.surface_capabilities;
  uint32_t imageCount { VK_ChooseImageCount(surfaceCapabilities) };

  VkPresentModeKHR presentMode { VK_ChoosePresentMode(selectedDevice.present_modes) };
  vk_surface_format = VK_ChooseSurfaceFormatAndColorSpace(selectedDevice.surface_formats);

  VkSwapchainCreateInfoKHR swapChainCreateInfo {
    .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
    .pNext = nullptr,
    .flags = 0,
    .surface = vk_surface,
    .minImageCount = imageCount,
    .imageFormat = vk_surface_format.format,
    .imageColorSpace = vk_surface_format.colorSpace,
    .imageExtent = surfaceCapabilities.currentExtent,
    .imageArrayLayers = 1,
    .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
    .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
    .queueFamilyIndexCount = 1,
    .pQueueFamilyIndices = &vk_queue_family,
    .preTransform = surfaceCapabilities.currentTransform,
    .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
    .presentMode = presentMode,
    .clipped = VK_TRUE
  };

  VkResult result = vkCreateSwapchainKHR(vk_device, &swapChainCreateInfo, nullptr, &vk_swapchain);
  Vk_CheckResult(result, "Failed to create Vulkan Swapchain!");

  uint32_t swapChainImgCt { 0 };
  result = vkGetSwapchainImagesKHR(vk_device, vk_swapchain, &swapChainImgCt, nullptr);
  if(result != VK_SUCCESS || swapChainImgCt != imageCount)
  { throw Logging::Error(std::format("Swapchain has {} Images, Expected {}!", swapChainImgCt, imageCount)); }
  Logging::Debug(std::format("Swap Chain Image Count: {}", swapChainImgCt));

  vk_swapchain_imgs.resize(swapChainImgCt);
  vk_swapchain_img_views.resize(swapChainImgCt);
  result = vkGetSwapchainImagesKHR(vk_device, vk_swapchain, &swapChainImgCt, vk_swapchain_imgs.data());
  Vk_CheckResult(result, "Failed to get Vulkan Swapchain Images!");

  for(uint32_t idx { 0 }; idx < swapChainImgCt; idx++) {
    vk_swapchain_img_views[idx] = VK_CreateImageView(vk_device, vk_swapchain_imgs[idx], vk_surface_format.format);
  }
  Logging::Debug("Vulkan Swap Chain Created.");
}

void VulkanEngine::CreateQueue() {
  Logging::Debug("Creating Vulkan Queue and Semaphores...");
  vkGetDeviceQueue(vk_device, vk_queue_family, 0, &vk_queue);
  
  VkSemaphoreCreateInfo const semaphoreCreateInfo {
    .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
    .pNext = nullptr,
    .flags = 0
  };
  VkResult result = vkCreateSemaphore(vk_device, &semaphoreCreateInfo, nullptr, &vk_render_complete_semaphore);
  Vk_CheckResult(result, "Failed to create render complete semaphore!");

  result = vkCreateSemaphore(vk_device, &semaphoreCreateInfo, nullptr, &vk_present_complete_semaphore);
  Vk_CheckResult(result, "Failed to create present complete semaphore!");

  Logging::Debug("Vulkan Queue and Semaphores Created.");
}

void VulkanEngine::CreateSimpleRenderPass() {
  Logging::Debug("Creating Simple Render Pass...");
  VkAttachmentDescription attachDesc {
    .flags = 0,
    .format = vk_surface_format.format,
    .samples = VK_SAMPLE_COUNT_1_BIT,
    .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
    .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
    .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
    .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
    .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
    .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
  };
  VkAttachmentReference attachRef {
    .attachment = 0,
    .layout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
  };
  VkSubpassDescription subpassDesc {
    .flags = 0,
    .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
    .inputAttachmentCount = 0, .pInputAttachments = nullptr,
    .colorAttachmentCount = 1, .pColorAttachments = &attachRef,
    .pResolveAttachments = nullptr,
    .pDepthStencilAttachment = nullptr,
    .preserveAttachmentCount = 0, .pPreserveAttachments = nullptr
  };
  VkRenderPassCreateInfo renderPassCreateInfo {
    .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
    .pNext = nullptr,
    .flags = 0,
    .attachmentCount = 1, .pAttachments = &attachDesc, // attachments
    .subpassCount = 1, .pSubpasses = &subpassDesc, // subpasses
    .dependencyCount = 0, .pDependencies = nullptr // dependencies
  };
  VkResult result = vkCreateRenderPass(vk_device, &renderPassCreateInfo, nullptr, &vk_render_pass);
  Vk_CheckResult(result, "Failed to create Simple Render Pass!");
  Logging::Debug("Simple Render Pass Created.");
}

void VulkanEngine::CreateFrameBuffers() {
  Logging::Debug("Creating Vulkan Frame Buffers...");
  vk_frame_buffers.resize(vk_swapchain_imgs.size());

  int windowWidth { 0 }, windowHeight { 0 };
  glfwGetFramebufferSize(glfw_window, &windowWidth, &windowHeight);
  
  VkFramebufferCreateInfo frameBufferCreateInfo {
    .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
    .pNext = nullptr,
    .renderPass = vk_render_pass,
    .attachmentCount = 1,
    .pAttachments = nullptr,
    .width = uint32_t(windowWidth),
    .height = uint32_t(windowHeight),
    .layers = 1
  };
  VkResult result { VK_SUCCESS };
  for(uint32_t idx { 0 }; idx < vk_swapchain_imgs.size(); idx++) {
    frameBufferCreateInfo.pAttachments = &vk_swapchain_img_views[idx];
    result = vkCreateFramebuffer(vk_device, &frameBufferCreateInfo, nullptr, &vk_frame_buffers[idx]);
    Vk_CheckResult(result, "Error creating Vulkan Frame Buffer!"); 
  }

  Logging::Debug("Vulkan Frame Buffers Created.");
}

void VulkanEngine::CreateCommandBuffer() {
  Logging::Debug("Creating Vulkan Command Pool...");
  VkCommandPoolCreateInfo const cmdPoolCreateInfo {
    .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
    .pNext = nullptr,
    .flags = 0,
    .queueFamilyIndex = vk_queue_family
  };
  VkResult result = vkCreateCommandPool(vk_device, &cmdPoolCreateInfo, nullptr, &vk_cmd_pool);
  Vk_CheckResult(result, "Failed to create Vulkan Command Pool!");
  Logging::Debug("Vulkan Command Pool Created.");

  Logging::Debug("Creating Vulkan Command Buffers...");
  uint32_t cmdBufCt = uint32_t(vk_swapchain_imgs.size());
  vk_cmd_bufs.resize(cmdBufCt);
  VkCommandBufferAllocateInfo const cmdBufAllocInfo {
    .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
    .pNext = nullptr,
    .commandPool = vk_cmd_pool,
    .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
    .commandBufferCount = cmdBufCt
  };
  result = vkAllocateCommandBuffers(vk_device, &cmdBufAllocInfo, vk_cmd_bufs.data());
  Vk_CheckResult(result, "Failed to create Vulkan Command Buffer!");
  Logging::Debug("Vulkan Command Buffers Created.");
}

void VulkanEngine::RecordCommandBuffers() {
  Logging::Debug("Recording Command Buffers...");
  VkClearColorValue const clearColor = { 0.137f, 0.902f, 0.698f, 0.0f };
  VkClearValue const clearValue { .color = clearColor };
  
  VkCommandBufferBeginInfo const beginInfo {
    .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
    .pNext = nullptr,
    .flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT, // 0 for clear
    .pInheritanceInfo = nullptr
  };
  
  int windowWidth { 0 }, windowHeight { 0 };
  glfwGetFramebufferSize(glfw_window, &windowWidth, &windowHeight);
  VkRenderPassBeginInfo renderPassBeginInfo {
    .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
    .pNext = nullptr,
    .renderPass = vk_render_pass,
    .renderArea {
      .offset { .x = 0, .y = 0},
      .extent {
        .width = uint32_t(windowWidth),
        .height = uint32_t(windowHeight)
      }
    },
    .clearValueCount = 1,
    .pClearValues = &clearValue
  };

  for(uint32_t idx { 0 }; idx < vk_cmd_bufs.size(); idx++) {
    VkCommandBuffer const & cmdBuf { vk_cmd_bufs[idx] };
    VkImage const & vkImg { vk_swapchain_imgs[idx] };

    VkResult result {};
    result = vkBeginCommandBuffer(cmdBuf, &beginInfo);
    Vk_CheckResult(result, "Failed to begin Command Buffer Record!");

    renderPassBeginInfo.framebuffer = vk_frame_buffers[idx];
    vkCmdBeginRenderPass(cmdBuf, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdEndRenderPass(cmdBuf);

    result = vkEndCommandBuffer(cmdBuf);
    Vk_CheckResult(result, "Failed to end Command Buffer Record!");
  }
  Logging::Debug("Command Buffers Recorded.");
}

void VulkanEngine::CreateShaders() {
  Logging::Debug("Creating Shaders...");
  vk_vert_shader_module = CreateShaderModuleFromText(vk_device, "test.vert");
  vk_frag_shader_module = CreateShaderModuleFromText(vk_device, "test.frag");
  Logging::Debug("Shaders Created.");
}

// VkQueue
uint32_t VulkanEngine::VkQueue_GetNextImg() {
  uint32_t imgIdx { 0 };
  uint64_t timeout { std::numeric_limits<uint64_t>::max() };
  VkResult result = vkAcquireNextImageKHR(vk_device, vk_swapchain, timeout, vk_present_complete_semaphore, nullptr, &imgIdx);
  Vk_CheckResult(result, "Failed to Acquire Next Img from Vk Queue!");
  return imgIdx;
}

void VulkanEngine::VkQueue_SubmitBuf(VkCommandBuffer const & _cmdBuf, bool const _async) {
  VkPipelineStageFlags waitFlags { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
  VkSubmitInfo const submitInfo {
    .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
    .pNext = nullptr,
    .waitSemaphoreCount = uint32_t(_async ? 1 : 0),
    .pWaitSemaphores = _async ? &vk_present_complete_semaphore : nullptr,
    .pWaitDstStageMask = _async ? &waitFlags : nullptr,
    .commandBufferCount = 1,
    .pCommandBuffers = &_cmdBuf,
    .signalSemaphoreCount = uint32_t(_async ? 1 : 0),
    .pSignalSemaphores = _async ? &vk_render_complete_semaphore : nullptr
  };
  VkResult result = vkQueueSubmit(vk_queue, 1, &submitInfo, nullptr);
  Vk_CheckResult(result, "Failed to Submit Vk Queue!");
}

void VulkanEngine::VkQueue_Present(uint32_t const _imgIdx) {
  VkPresentInfoKHR const presentInfo {
    .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
    .pNext = nullptr,
    .waitSemaphoreCount = 1,
    .pWaitSemaphores = &vk_render_complete_semaphore,
    .swapchainCount = 1,
    .pSwapchains = &vk_swapchain,
    .pImageIndices = &_imgIdx
  };
  VkResult result = vkQueuePresentKHR(vk_queue, &presentInfo);
  Vk_CheckResult(result, "Failed to Present Vk Queue!");
}

void VulkanEngine::VkQueue_WaitIdle() {
  vkQueueWaitIdle(vk_queue);
}