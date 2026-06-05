#include "../shared.hpp"
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
      layers = std::make_unique<std::vector<const char*>>();
      layers->push_back("VK_LAYER_KHRONOS_validation");
    }
    if (extensions == nullptr) {
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
  if (vk_instance == nullptr) { return; }

  PFN_vkDestroySurfaceKHR vkDestroySurface = nullptr;
  vkDestroySurface = PFN_vkDestroySurfaceKHR(vkGetInstanceProcAddr(vk_instance, "vkDestroySurfaceKHR"));
  if(!vkDestroySurface) {
    printf("Failed to find addr for vkDestroySurfaceKHR");
  } else {
    vkDestroySurface(vk_instance, vk_surface, nullptr);
  }
  vk_surface = nullptr;

  // Teardown Debug Messenger
  PFN_vkDestroyDebugUtilsMessengerEXT vkDestroyDbgUtilsMessenger = nullptr;
  vkDestroyDbgUtilsMessenger = PFN_vkDestroyDebugUtilsMessengerEXT(vkGetInstanceProcAddr(vk_instance, "vkDestroyDebugUtilsMessengerEXT"));
  if (!vkDestroyDbgUtilsMessenger) {
    printf("Failed to find addr for vkDestroyDebugUtilsMessengerEXT");
  } else {
    vkDestroyDbgUtilsMessenger(vk_instance, vk_dbgMessenger, nullptr);
  }
  vk_dbgMessenger = nullptr;

  // Teardown Instance
  vkDestroyInstance(vk_instance, nullptr);
  vk_instance = nullptr;
  extensions = nullptr;
  layers = nullptr;
  glfw_window = nullptr;
}

bool VulkanCore::Initialized() {
  return vk_instance != nullptr &&
    vk_dbgMessenger != nullptr;
}

void VulkanCore::Init(GLFWwindow* _glfw_window) {
  glfw_window = _glfw_window;
  CreateVkInst();
  CreateDebugCallback();
  CreateSurface();
  physical_devices.Init(vk_instance, vk_surface);
  queue_family = physical_devices.SelectDevice(VK_QUEUE_GRAPHICS_BIT, true);
}

void VulkanCore::RenderScene() {

}

void VulkanCore::CreateVkInst() {
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
      printf("FAILED TO CREATE VK INSTANCE: %d\n", vkResult);
      vk_instance = nullptr;
  }
}

void VulkanCore::CreateDebugCallback() {
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
    printf("FAILED TO INIT VK DEBUG UTILS MESSENGER\n");
    return;
  }

  VkResult result = vkCreateDbgUtilsMessenger(vk_instance, &messengerCreateInfo, nullptr, &vk_dbgMessenger);
  if (result != VK_SUCCESS) {
    printf("FAILED TO INIT VK DEBUG UTILS MESSENGER\n");
    vk_dbgMessenger = nullptr;
    return;
  }
}

void VulkanCore::CreateSurface() {
  VkResult result = glfwCreateWindowSurface(vk_instance, glfw_window, nullptr, &vk_surface);
  if(result != VK_SUCCESS) {
    printf("FAILED TO INIT GLFW/VK SURFACE");
    vk_surface = nullptr;
  }
}
