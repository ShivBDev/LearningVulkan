#ifndef vulkan_utils_hpp
#define vulkan_utils_hpp
#include <vulkan/vulkan.h>
#include <string>

void Vk_CheckResult(VkResult _result, std::string _msg);

#endif