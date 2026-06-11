#include "VulkanEngine.hpp"
#include "Logging.hpp"
#include "VulkanUtils.hpp"

void VulkanEngine::__Record__Clr__Cmd__Bufs() {
  Logging::Debug("Recording Command Buffers...");
  VkClearColorValue const clearColor = { 0.137f, 0.902f, 0.698f, 0.0f };
  VkCommandBufferBeginInfo const beginInfo {
    .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
    .pNext = nullptr,
    .flags = 0,
    .pInheritanceInfo = nullptr
  };
  VkImageSubresourceRange const imgRange {
    .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
    .baseMipLevel = 0,
    .levelCount = 1,
    .baseArrayLayer = 0,
    .layerCount = 1
  };
  VkImageMemoryBarrier presentToClearBarrier {
    .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
    .pNext = nullptr,
    .srcAccessMask = VK_ACCESS_MEMORY_READ_BIT,
    .dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
    .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
    .newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
    .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
    .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
    .image = nullptr,
    .subresourceRange = imgRange
  };

  VkImageMemoryBarrier clearToPresentBarrier {
    .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
    .pNext = nullptr,
    .srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
    .dstAccessMask = VK_ACCESS_MEMORY_READ_BIT,
    .oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
    .newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
    .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
    .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
    .image = nullptr,
    .subresourceRange = imgRange
  };

  for(uint32_t idx { 0 }; idx < vk_cmd_bufs.size(); idx++) {
    VkCommandBuffer const & cmdBuf { vk_cmd_bufs[idx] };
    VkImage const & vkImg { vk_swapchain_imgs[idx] };
    presentToClearBarrier.image = vkImg;
    clearToPresentBarrier.image = vkImg;
    VkResult result {};
    result = vkBeginCommandBuffer(cmdBuf, &beginInfo);
    Vk_CheckResult(result, "Failed to begin Command Buffer Record!");
    vkCmdPipelineBarrier(
      cmdBuf, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
      0, // dep flags
      0, nullptr, // mem barriers
      0, nullptr, // buf mem barriers
      1, &presentToClearBarrier // img memory barriers
    );
    vkCmdClearColorImage(cmdBuf, vk_swapchain_imgs[idx], VK_IMAGE_LAYOUT_GENERAL, &clearColor, 1, &imgRange);
    vkCmdPipelineBarrier(
      cmdBuf, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
      0, // dep flags
      0, nullptr, // mem barriers
      0, nullptr, // buf mem barriers
      1, &clearToPresentBarrier // img memory barriers
    );
    result = vkEndCommandBuffer(cmdBuf);
    Vk_CheckResult(result, "Failed to end Command Buffer Record!");
  }
  Logging::Debug("Command Buffers Recorded.");
}