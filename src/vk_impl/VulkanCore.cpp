#include "../shared.hpp"
#include "../Logging.hpp"
#include "VulkanCore.hpp"
#include "VulkanSettings.hpp"
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
    if(result != VK_SUCCESS)
    { throw Logging::Error("Failed to Create Image View"); }
    return imgView;
  }
} // namespace

VulkanCore::VulkanCore() {
}

VulkanCore::~VulkanCore() {
  Logging::Log("Beginning Vulkan Core Teardown...");
  if (vk_instance == nullptr) { return; }

  vkFreeCommandBuffers(vk_logical_device, vk_cmd_pool, uint32_t(vk_cmd_bufs.size()), vk_cmd_bufs.data());
  vkDestroyCommandPool(vk_logical_device, vk_cmd_pool, nullptr);
  Logging::Debug("Vulkan Command Pool Destroyed!");

  for(VkImageView const & imgView : swap_chain_image_views) {
    vkDestroyImageView(vk_logical_device, imgView, nullptr);
  }
  vkDestroySwapchainKHR(vk_logical_device, swap_chain, nullptr);
  Logging::Debug("Vulkan Swap Chain Destroyed!");

  vkDestroyDevice(vk_logical_device, nullptr);
  Logging::Debug("Vulkan Logical Device Destroyed.");

  PFN_vkDestroySurfaceKHR vkDestroySurface = nullptr;
  vkDestroySurface = PFN_vkDestroySurfaceKHR(vkGetInstanceProcAddr(vk_instance, "vkDestroySurfaceKHR"));
  if(!vkDestroySurface) {
    Logging::Warn("Failed to find addr for vkDestroySurfaceKHR");
  } else {
    vkDestroySurface(vk_instance, vk_surface, nullptr);
  }
  vk_surface = nullptr;
  Logging::Debug("Vulkan Surface KHR Destroyed.");

  // Teardown Debug Messenger
  PFN_vkDestroyDebugUtilsMessengerEXT vkDestroyDbgUtilsMessenger = nullptr;
  vkDestroyDbgUtilsMessenger = PFN_vkDestroyDebugUtilsMessengerEXT(vkGetInstanceProcAddr(vk_instance, "vkDestroyDebugUtilsMessengerEXT"));
  if (!vkDestroyDbgUtilsMessenger) {
    Logging::Warn("Failed to find addr for vkDestroyDebugUtilsMessengerEXT");
  } else {
    vkDestroyDbgUtilsMessenger(vk_instance, vk_dbgMessenger, nullptr);
  }
  vk_dbgMessenger = nullptr;
  Logging::Debug("Vulkan Debug Messenger Destroyed.");

  // Teardown Instance
  vkDestroyInstance(vk_instance, nullptr);
  vk_instance = nullptr;
  glfw_window = nullptr;
  Logging::Debug("Vulkan Instance Destroyed.");
  Logging::Log("Vulkan Core Teardown Complete.");
}

bool VulkanCore::Initialized() {
  return vk_instance != nullptr &&
    vk_dbgMessenger != nullptr;
}

void VulkanCore::Init(GLFWwindow* _glfw_window) {
  Logging::Log("Initializing Vulkan Core...");
  glfw_window = _glfw_window;
  CreateVkInst();
  CreateDebugCallback();
  CreateSurface();
  physical_devices.Init(vk_instance, vk_surface);
  queue_family = physical_devices.SelectDevice(VK_QUEUE_GRAPHICS_BIT, true);
  CreateDevice();
  CreateSwapChain();
  CreateCommandBuffer();
  Logging::Log("Vulkan Core Initialized.");
}

void VulkanCore::RenderScene() {

}

void VulkanCore::CreateVkInst() {
  Logging::Debug("Creating Vulkan Instance...");
  VkResult vkResult = vkCreateInstance(&__vk_instance_create_info, nullptr, &vk_instance);
  if(vkResult != VK_SUCCESS) {
      throw Logging::Error(std::format("Failed to crete Vk Instance: {}!", uint32_t(vkResult)));
      vk_instance = nullptr;
  }
  Logging::Debug("Vulkan Instance Created.");
}

void VulkanCore::CreateDebugCallback() {
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
  if (result != VK_SUCCESS) {
    vk_dbgMessenger = nullptr;
    throw Logging::Error("Failed to init Vk Debug Utils Messenger!");
  }
  Logging::Debug("Vulkan Debug Callback Created.");
}

void VulkanCore::CreateSurface() {
  Logging::Debug("Creating Vulkan Surface with Glfw Window...");
  VkResult result = glfwCreateWindowSurface(vk_instance, glfw_window, nullptr, &vk_surface);
  if(result != VK_SUCCESS) {
    vk_surface = nullptr;
    throw Logging::Error("Failed to init Glfw/Vk Surface!");
  }
  Logging::Debug("Vulkan Surface with Glfw Window Created.");
}

void VulkanCore::CreateDevice() {
  Logging::Debug("Creating Vulkan Logical Device...");
  std::vector<float> queuePriorities { 1.0f };
  VkDeviceQueueCreateInfo const __vk_device_queue_create_info {
    .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
    .pNext = nullptr,
    .flags = 0,
    .queueFamilyIndex = queue_family,
    .queueCount = 1,
    .pQueuePriorities = &queuePriorities[0]
  };
  // Geometry Shaders not supported w/ MoltenVK
  // Use compute shaders and indirect render
  PhysicalDevice const & selectedDevice = physical_devices.Selected();
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

  VkResult result = vkCreateDevice(selectedDevice.vk_device, &deviceCreateInfo, nullptr, &vk_logical_device);
  if (result != VK_SUCCESS) 
  { throw Logging::Error("Failed to create logical device from physical device!"); }
  Logging::Debug("Vulkan Logical Device Created.");
}

void VulkanCore::CreateSwapChain() {
  Logging::Debug("Creating Vulkan Swap Chain...");
  PhysicalDevice const& selectedDevice = physical_devices.Selected();

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
    .pQueueFamilyIndices = &queue_family,
    .preTransform = surfaceCapabilities.currentTransform,
    .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
    .presentMode = presentMode,
    .clipped = VK_TRUE
  };

  VkResult result = vkCreateSwapchainKHR(vk_logical_device, &swapChainCreateInfo, nullptr, &swap_chain);
  if (result != VK_SUCCESS)
  { throw Logging::Error("Failed to create Vulkan Swapchain!"); }

  uint32_t swapChainImgCt { 0 };
  result = vkGetSwapchainImagesKHR(vk_logical_device, swap_chain, &swapChainImgCt, nullptr);
  if(result != VK_SUCCESS || swapChainImgCt != imageCount)
  { throw Logging::Error(std::format("Swapchain has {} Images, Expected {}!", swapChainImgCt, imageCount)); }
  Logging::Debug(std::format("Swap Chain Image Count: {}", swapChainImgCt));

  swap_chain_images.resize(swapChainImgCt);
  swap_chain_image_views.resize(swapChainImgCt);
  result = vkGetSwapchainImagesKHR(vk_logical_device, swap_chain, &swapChainImgCt, swap_chain_images.data());
  if (result != VK_SUCCESS)
  { throw Logging::Error("Failed to get Vulkan Swapchain Images!"); }

  for(uint32_t idx { 0 }; idx < swapChainImgCt; idx++) {
    swap_chain_image_views[idx] = VK_CreateImageView(vk_logical_device, swap_chain_images[idx], surfaceFormat.format);
  }

  Logging::Debug("Vulkan Swap Chain Created.");
}

void VulkanCore::CreateCommandBuffer() {
  Logging::Debug("Creating Vulkan Command Pool...");
  VkCommandPoolCreateInfo const cmdPoolCreateInfo {
    .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
    .pNext = nullptr,
    .flags = 0,
    .queueFamilyIndex = queue_family
  };
  VkResult result = vkCreateCommandPool(vk_logical_device, &cmdPoolCreateInfo, nullptr, &vk_cmd_pool);
  if(result != VK_SUCCESS)
  { throw Logging::Error("Failed to create Vulkan Command Pool!"); }
  Logging::Debug("Vulkan Command Pool Created.");

  Logging::Debug("Creating Vulkan Command Buffers...");
  uint32_t cmdBufCt = uint32_t(swap_chain_images.size());
  vk_cmd_bufs.resize(cmdBufCt);
  VkCommandBufferAllocateInfo const cmdBufAllocInfo {
    .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
    .pNext = nullptr,
    .commandPool = vk_cmd_pool,
    .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
    .commandBufferCount = cmdBufCt
  };
  result = vkAllocateCommandBuffers(vk_logical_device, &cmdBufAllocInfo, vk_cmd_bufs.data());
  if(result != VK_SUCCESS)
  { throw Logging::Error("Failed to create Vulkan Command Buffer!"); }
  Logging::Debug("Vulkan Command Buffers Created.");
}
