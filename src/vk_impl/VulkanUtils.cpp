#include "VulkanUtils.hpp"
#include "../Logging.hpp"

void Vk_CheckResult(VkResult _result, std::string _msg) {
  if (_result != VK_SUCCESS) { throw Logging::Error(_msg); }
}