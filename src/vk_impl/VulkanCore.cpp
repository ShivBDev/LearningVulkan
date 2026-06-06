#include "../shared.hpp"
#include "../Logging.h"
#include "VulkanCore.hpp"
#include <iostream>
#include <vector>
#include <map>

namespace {
  std::map<VkDebugUtilsMessageSeverityFlagBitsEXT const, const char *> msgSeverityStrs {
    { VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT, "Debug" },
    { VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT, "Log"},
    { VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT, "Warning"},
    { VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT, "Error"}
  };
  std::map<VkDebugUtilsMessageTypeFlagsEXT const, const char *> msgTypeStrs {
    { VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT, "General"},
    { VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT, "Validation"},
    { VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT, "Performance"}
  };

  static std::unique_ptr<std::vector<const char*>> layers { nullptr };
  static std::unique_ptr<std::vector<const char*>> extensions { nullptr };
  void SetupVulkanLayersAndExtensions() {
    if (layers == nullptr) {
      Logging::Debug("Setting up Vulkan Layers Vector.");
      layers = std::make_unique<std::vector<const char*>>();
      layers->push_back("VK_LAYER_KHRONOS_validation");
    }
    if (extensions == nullptr) {
      Logging::Debug("Setting up Vulkan Extensions Vector.");
      extensions = std::make_unique<std::vector<const char*>>();
      extensions->push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
      extensions->push_back(VK_KHR_SURFACE_EXTENSION_NAME);
      extensions->push_back("VK_EXT_metal_surface");
      extensions->push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }
  }

  static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback (
  VkDebugUtilsMessageSeverityFlagBitsEXT _severity,
  VkDebugUtilsMessageTypeFlagsEXT _type,
  const VkDebugUtilsMessengerCallbackDataEXT* _callbackData,
  void * _userData ) {
    printf("Dbg Callback: %s\nSeverity: %s\nType: %s\n",
      _callbackData->pMessage,
      msgSeverityStrs[_severity],
      msgTypeStrs[_type]);
    return VK_FALSE;
  }
}

VulkanCore::VulkanCore() {
  SetupVulkanLayersAndExtensions();
}

VulkanCore::~VulkanCore() {
  Logging::Log("Beginning Vulkan Core Teardown...");
  if (vk_instance == nullptr) { return; }

  vkDestroyDevice(vk_logical_device, nullptr);
  Logging::Debug("Vulkan Logical Device Destroyed.");

  PFN_vkDestroySurfaceKHR vkDestroySurface = nullptr;
  vkDestroySurface = PFN_vkDestroySurfaceKHR(vkGetInstanceProcAddr(vk_instance, "vkDestroySurfaceKHR"));
  if(!vkDestroySurface) {
    printf("Failed to find addr for vkDestroySurfaceKHR");
  } else {
    vkDestroySurface(vk_instance, vk_surface, nullptr);
  }
  vk_surface = nullptr;
  Logging::Debug("Vulkan Surface KHR Destroyed.");

  // Teardown Debug Messenger
  PFN_vkDestroyDebugUtilsMessengerEXT vkDestroyDbgUtilsMessenger = nullptr;
  vkDestroyDbgUtilsMessenger = PFN_vkDestroyDebugUtilsMessengerEXT(vkGetInstanceProcAddr(vk_instance, "vkDestroyDebugUtilsMessengerEXT"));
  if (!vkDestroyDbgUtilsMessenger) {
    printf("Failed to find addr for vkDestroyDebugUtilsMessengerEXT");
  } else {
    vkDestroyDbgUtilsMessenger(vk_instance, vk_dbgMessenger, nullptr);
  }
  vk_dbgMessenger = nullptr;
  Logging::Debug("Vulkan Debug Messenger Destroyed.");

  // Teardown Instance
  vkDestroyInstance(vk_instance, nullptr);
  vk_instance = nullptr;
  extensions = nullptr;
  layers = nullptr;
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
  //CreateDevice();
  Logging::Log("Vulkan Core Initialized.");
}

void VulkanCore::RenderScene() {

}

void VulkanCore::CreateVkInst() {
  Logging::Debug("Creating Vulkan Instance...");
  VkApplicationInfo appInfo {
    .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
    .pNext = nullptr,
    .pApplicationName = __app_name,
    .applicationVersion = VK_MAKE_VERSION(0, 0, 1),
    .pEngineName = __engine_name,
    .engineVersion = VK_MAKE_VERSION(0, 0, 1),
    .apiVersion = VK_API_VERSION_1_4
  };
  
  VkInstanceCreateInfo createInfo {
    .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
    .pNext = nullptr,
    .flags = VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR,
    .pApplicationInfo = &appInfo,
    .enabledLayerCount = uint32_t(layers->size()),
    .ppEnabledLayerNames = layers->data(),
    .enabledExtensionCount = uint32_t(extensions->size()),
    .ppEnabledExtensionNames = extensions->data()
  };

  VkResult vkResult = vkCreateInstance(&createInfo, nullptr, &vk_instance);
  if(vkResult != VK_SUCCESS) {
      throw Logging::Error(std::format("Failed to crete Vk Instance: {}!", uint32_t(vkResult)));
      vk_instance = nullptr;
  }
  Logging::Debug("Vulkan Instance Created.");
}

void VulkanCore::CreateDebugCallback() {
  Logging::Debug("Creating Vulkan Debug Callback...");
  VkDebugUtilsMessengerCreateInfoEXT messengerCreateInfo {
    .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
    .pNext = nullptr,
    .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
      VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
      VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
      VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
    .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
      VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
      VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
    .pfnUserCallback = &DebugCallback,
    .pUserData = nullptr
  };
  PFN_vkCreateDebugUtilsMessengerEXT vkCreateDbgUtilsMessenger = nullptr;
  vkCreateDbgUtilsMessenger = PFN_vkCreateDebugUtilsMessengerEXT(vkGetInstanceProcAddr(vk_instance, "vkCreateDebugUtilsMessengerEXT"));
  if(!vkCreateDbgUtilsMessenger) {
    throw Logging::Error("Failed to init Vk Debug Utils Messenger!");
  }
  VkResult result = vkCreateDbgUtilsMessenger(vk_instance, &messengerCreateInfo, nullptr, &vk_dbgMessenger);
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
  VkDeviceQueueCreateInfo deviceQueueCreateInfo {
    .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
    .pNext = nullptr,
    .flags = 0,
    .queueFamilyIndex = queue_family,
    .queueCount = 1,
    .pQueuePriorities = &queuePriorities[0]
  };
  std::vector<const char *> deviceExtensions {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME,
    VK_KHR_SHADER_DRAW_PARAMETERS_EXTENSION_NAME
  };

  // Geometry Shaders not supported w/ MoltenVK
  // Use compute shaders and indirect render
  PhysicalDevice const & selectedDevice = physical_devices.Selected();
  // if(selectedDevice.features.geometryShader == VK_FALSE)
  // { throw Logging::Error("Geometry Shader Not Supported!"); }
  if(selectedDevice.features.tessellationShader == VK_FALSE)
  { throw Logging::Error("Tessellation Shader Not Supportd!"); }

  VkPhysicalDeviceFeatures deviceFeatures {
    //.geometryShader = VK_TRUE,
    .tessellationShader = VK_TRUE
  };
  VkDeviceCreateInfo deviceCreateInfo {
    .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
    .pNext = nullptr,
    .flags = 0,
    .queueCreateInfoCount = 1,
    .pQueueCreateInfos = &deviceQueueCreateInfo,
    .enabledExtensionCount = uint32_t(deviceExtensions.size()),
    .ppEnabledExtensionNames = deviceExtensions.data(),
    .pEnabledFeatures = &deviceFeatures
  };

  VkResult result = vkCreateDevice(selectedDevice.vk_device, &deviceCreateInfo, nullptr, &vk_logical_device);
  if (result != VK_SUCCESS) 
  { throw Logging::Error("Failed to create logical device from physical device!"); }
  Logging::Debug("Vulkan Logical Device Created.");
}
