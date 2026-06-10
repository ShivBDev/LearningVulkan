#include "../shared.hpp"
#include "../Logging.hpp"
#include "VulkanEngine.hpp"
#include "VulkanSettings.hpp"
#include "VulkanUtils.hpp"
#include <iostream>

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
  VkSurfaceFormatKHR surfaceFormat { VK_ChooseSurfaceFormatAndColorSpace(selectedDevice.surface_formats) };

  VkSwapchainCreateInfoKHR swapChainCreateInfo {
    .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
    .pNext = nullptr,
    .flags = 0,
    .surface = vk_surface,
    .minImageCount = imageCount,
    .imageFormat = surfaceFormat.format,
    .imageColorSpace = surfaceFormat.colorSpace,
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
    vk_swapchain_img_views[idx] = VK_CreateImageView(vk_device, vk_swapchain_imgs[idx], surfaceFormat.format);
  }
  Logging::Debug("Vulkan Swap Chain Created.");
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
  VkClearColorValue clearColor = { 0.137f, 0.902f, 0.698f, 0.0f };
  VkImageSubresourceRange const imgRange {
    .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
    .baseMipLevel = 0,
    .levelCount = 1,
    .baseArrayLayer = 0,
    .layerCount = 1
  };
  VkCommandBufferBeginInfo beginInfo {
    .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
    .pNext = nullptr,
    .flags = 0,
    .pInheritanceInfo = nullptr
  };
  for(uint32_t idx { 0 }; idx < vk_cmd_bufs.size(); idx++) {
    VkCommandBuffer const & cmdBuf { vk_cmd_bufs[idx] };
    VkResult result {};
    result = vkBeginCommandBuffer(cmdBuf, &beginInfo);
    Vk_CheckResult(result, "Failed to begin Command Buffer Record!");

    vkCmdClearColorImage(cmdBuf, vk_swapchain_imgs[idx], VK_IMAGE_LAYOUT_GENERAL, &clearColor, 1, &imgRange);

    result = vkEndCommandBuffer(cmdBuf);
    Vk_CheckResult(result, "Failed to end Command Buffer Record!");
  }
  Logging::Debug("Command Buffers Recorded.");
}

// VkQueue
void VulkanEngine::VkQueue_CreateQueue() {
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