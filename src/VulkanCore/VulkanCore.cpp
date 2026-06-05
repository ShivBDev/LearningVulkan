#include "VulkanCore.hpp"
#include "../shared.hpp"
#include <iostream>
#include <vector>

VulkanCore::VulkanCore() {
  layers = std::make_unique<std::vector<const char*>>();
  layers->push_back("VK_LAYER_KHRONOS_validation");

  extensions = std::make_unique<std::vector<const char*>>();
  extensions->push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
  extensions->push_back(VK_KHR_SURFACE_EXTENSION_NAME);
  extensions->push_back("VK_EXT_metal_surface");
  extensions->push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
  extensions->push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
}


VulkanCore::~VulkanCore() {
  if (vk_instance == nullptr) { return; }
  vkDestroyInstance(vk_instance, nullptr);
  extensions = nullptr;
  layers = nullptr;
}
bool VulkanCore::Initialized() {
  return vk_instance != nullptr;
}

void VulkanCore::Init() {
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

void VulkanCore::RenderScene() {

}
