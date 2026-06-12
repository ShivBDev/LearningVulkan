#include "VulkanEngine.hpp"
#include "../Logging.hpp"
#include <chrono>
#include <thread>

void VulkanEngine::__Print_Frame_Info(std::chrono::time_point<std::chrono::high_resolution_clock> _startTime) {
  auto currTime = std::chrono::high_resolution_clock::now();
  uint32_t duration = std::chrono::duration_cast<std::chrono::milliseconds>(
  currTime - _startTime).count();
  float fps = duration > 0 ? 1000.0f / duration : 0.0f;

  std::string msg = std::format(
  "Frame Info: | FPS: {:05.2f} | Semaphore Index: {} | Awaiting Cpu: {}",
  fps, curr_semaphore_idx, curr_semaphore_idx == 0 ? " True" : "False");
  if(!__prev_msg.empty()) {
    printf("\033[A\033[2K\r");
  }
  Logging::Debug(msg);
  __prev_msg = msg;
}

void VulkanEngine::__Cap_Frame_Rate(
      std::chrono::time_point<std::chrono::high_resolution_clock> __fstartTime,
      std::chrono::time_point<std::chrono::high_resolution_clock> __fendTime
    ) {
  uint32_t duration = std::chrono::duration_cast<std::chrono::milliseconds>(
    __fendTime - __fstartTime).count();
  uint32_t expFTime = __force_fps != -1 ? uint32_t(1000.0f / float(__force_fps)) : 0;
  if (expFTime > duration) {
    std::this_thread::sleep_for(
    std::chrono::milliseconds(expFTime - duration));
  };
}

void VulkanEngine::RenderScene() {
  std::chrono::time_point<std::chrono::high_resolution_clock> fstartTime = std::chrono::high_resolution_clock::now();

  if (curr_semaphore_idx == 0) { VkQueue_WaitIdle(); }
  uint32_t imgIdx = VkQueue_GetNextImg();
  VkQueue_SubmitBuf(vk_cmd_bufs[imgIdx]);
  VkQueue_Present(imgIdx);

  if (__force_fps != -1) {
    std::chrono::time_point<std::chrono::high_resolution_clock> fendTime = std::chrono::high_resolution_clock::now();
    __Cap_Frame_Rate(fstartTime, fendTime);
  }
  __Print_Frame_Info(fstartTime);

  curr_semaphore_idx = (curr_semaphore_idx + 1) % vk_render_complete_semaphores.size();
}

VulkanEngine::VulkanEngine(GLFWwindow* _glfw_window) {
  Logging::Log("Initializing Vulkan Engine...");
  glfw_window = _glfw_window;
  CreateVulkanInstance();
  CreateDbgCallback();
  CreateSurface();
  vk_physical_devices.Init(vk_instance, vk_surface);
  vk_queue_family = vk_physical_devices.SelectDevice(VK_QUEUE_GRAPHICS_BIT, true);
  CreateDevice();
  CreateSwapChain();
  CreateQueue();

  CreateSimpleRenderPass();
  CreateFrameBuffers();
  CreateShaders();
  CreatePipeline();
  CreateCommandBuffer();
  RecordCommandBuffers();
  Logging::Log("Vulkan Core Initialized.");
}

VulkanEngine::~VulkanEngine() {
  Logging::Log("Beginning Vulkan Core Teardown...");
  if (vk_instance == nullptr) { return; }
  VkQueue_WaitIdle();

  vkFreeCommandBuffers(vk_device, vk_cmd_pool, uint32_t(vk_cmd_bufs.size()), vk_cmd_bufs.data());
  Logging::Debug("Vulkan Command Buffers Destroyed!");
  vkDestroyCommandPool(vk_device, vk_cmd_pool, nullptr);
  Logging::Debug("Vulkan Command Pool Destroyed!");

  vkDestroyPipelineLayout(vk_device, vk_pipeline_layout, nullptr);
  Logging::Debug("Vulkan Pipeline Layout Destroyed!");
  vkDestroyPipeline(vk_device, vk_pipeline, nullptr);
  Logging::Debug("Vulkan Pipeline Destroyed!");

  vkDestroyShaderModule(vk_device, vk_vert_shader_module, nullptr);
  vkDestroyShaderModule(vk_device, vk_frag_shader_module, nullptr);
  Logging::Debug("Vulkan Shaders Destroyed!");

  for(VkFramebuffer const & fBuf : vk_frame_buffers) {
    vkDestroyFramebuffer(vk_device, fBuf, nullptr);
  }
  Logging::Debug("Vulkan Frame Buffer Destroyed!");
  vkDestroyRenderPass(vk_device, vk_render_pass, nullptr);
  Logging::Debug("Vulkan Render Pass Destroyed!");

  for(VkSemaphore& semaphore : vk_render_complete_semaphores) {
    vkDestroySemaphore(vk_device, semaphore, nullptr);
  }
  for(VkSemaphore& semaphore : vk_present_complete_semaphores) {
    vkDestroySemaphore(vk_device, semaphore, nullptr);
  }
  Logging::Debug("Vulkan Queue Semaphores Destroyed!");

  for(VkImageView const & imgView : vk_swapchain_img_views) {
    vkDestroyImageView(vk_device, imgView, nullptr);
  }
  vkDestroySwapchainKHR(vk_device, vk_swapchain, nullptr);
  Logging::Debug("Vulkan Swap Chain Destroyed!");

  vkDestroyDevice(vk_device, nullptr);
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