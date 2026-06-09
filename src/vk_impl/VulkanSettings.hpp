#ifndef vulkan_settings_hpp
#define vulkan_settings_hpp
#include <vulkan/vulkan.h>
#include <map>
#include <vector>

namespace {
  // Vulkan Instance
  constexpr uint32_t const __vk_application_version { VK_MAKE_VERSION(0, 0, 1) };
  constexpr uint32_t const __vk_engine_version { VK_MAKE_VERSION(0, 0, 1) };
  constexpr VkApplicationInfo const __vk_application_info {
    .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
    .pNext = nullptr,
    .pApplicationName = __app_name,
    .applicationVersion = __vk_application_version,
    .pEngineName = __engine_name,
    .engineVersion = __vk_engine_version,
    .apiVersion = VK_API_VERSION_1_4
  };
  std::vector<const char *> const __vk_layers {
    "VK_LAYER_KHRONOS_validation"
  };
  std::vector<const char *> const __vk_extensions {
    VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME,
    VK_KHR_SURFACE_EXTENSION_NAME,
    "VK_EXT_metal_surface",
    VK_EXT_DEBUG_UTILS_EXTENSION_NAME
  };
  VkInstanceCreateInfo const __vk_instance_create_info {
    .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
    .pNext = nullptr,
    .flags = VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR,
    .pApplicationInfo = &__vk_application_info,
    .enabledLayerCount = uint32_t(__vk_layers.size()),
    .ppEnabledLayerNames = __vk_layers.data(),
    .enabledExtensionCount = uint32_t(__vk_extensions.size()),
    .ppEnabledExtensionNames = __vk_extensions.data()
  };
  // Debug Callback
  VkDebugUtilsMessageSeverityFlagsEXT __vk_msg_severities = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
    VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
    VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
    VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
  std::map<VkDebugUtilsMessageSeverityFlagBitsEXT const, const char *> __vk_msg_severity_str {
    { VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT, "Debug" },
    { VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT, "Log"},
    { VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT, "Warning"},
    { VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT, "Error"}
  };
  VkDebugUtilsMessageTypeFlagsEXT __vk_msg_types = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
    VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
    VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
  std::map<VkDebugUtilsMessageTypeFlagsEXT const, const char *> __vk_msg_type_str {
    { VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT, "General"},
    { VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT, "Validation"},
    { VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT, "Performance"}
  };
  // Logical Device Creation
  std::vector<const char *> const __vk_device_extensions {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME,
    VK_KHR_SHADER_DRAW_PARAMETERS_EXTENSION_NAME,
    "VK_KHR_portability_subset"
  };
  
  // Swap Chain
  constexpr VkPresentModeKHR const __vk_target_present_mode = VK_PRESENT_MODE_MAILBOX_KHR;
  constexpr VkPresentModeKHR const __vk_default_present_mode = VK_PRESENT_MODE_FIFO_KHR;
  constexpr VkFormat const __vk_target_surface_format = VK_FORMAT_B8G8R8A8_SRGB;
  constexpr VkColorSpaceKHR const __vk_target_color_space { VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
  // Swap Chain - Image View
    constexpr VkImageAspectFlags const __vk_img_view_default_aspect_flags { VK_IMAGE_ASPECT_COLOR_BIT };
    constexpr VkImageViewType const __vk_img_view_default_type { VK_IMAGE_VIEW_TYPE_2D };
    constexpr uint32_t const __vk_img_view_default_layer_count { 1 };
    constexpr uint32_t const __vk_img_view_default_mip_levels { 1 };
    constexpr VkComponentMapping const __vk_img_view_create_info_components {
      .r = VK_COMPONENT_SWIZZLE_IDENTITY,
      .g = VK_COMPONENT_SWIZZLE_IDENTITY,
      .b = VK_COMPONENT_SWIZZLE_IDENTITY,
      .a = VK_COMPONENT_SWIZZLE_IDENTITY
    };
}
#endif