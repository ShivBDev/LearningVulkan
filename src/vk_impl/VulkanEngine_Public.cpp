#include "VulkanEngine.hpp"
#include "../Logging.hpp"

void VulkanEngine::RenderScene() {
  uint32_t imgIdx = VkQueue_GetNextImg();
  VkQueue_SubmitBuf(vk_cmd_bufs[imgIdx]);
  VkQueue_Present(imgIdx);
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
  CreateCommandBuffer();
  RecordCommandBuffers();
  CreateShaders();
  Logging::Log("Vulkan Core Initialized.");
}

VulkanEngine::~VulkanEngine() {
  Logging::Log("Beginning Vulkan Core Teardown...");
  if (vk_instance == nullptr) { return; }

  vkDestroyShaderModule(vk_device, vk_vert_shader_module, nullptr);
  vkDestroyShaderModule(vk_device, vk_frag_shader_module, nullptr);
  Logging::Debug("Vulkan Shaders Destroyed!");

  for(VkFramebuffer const & fBuf : vk_frame_buffers) {
    vkDestroyFramebuffer(vk_device, fBuf, nullptr);
  }
  Logging::Debug("Vulkan Frame Buffer Destroyed!");
  vkDestroyRenderPass(vk_device, vk_render_pass, nullptr);
  Logging::Debug("Vulkan Render Pass Destroyed!");

  vkDestroySemaphore(vk_device, vk_present_complete_semaphore, nullptr);
  vkDestroySemaphore(vk_device, vk_render_complete_semaphore, nullptr);
  Logging::Debug("Vulkan Queue Semaphores Destroyed!");

  vkFreeCommandBuffers(vk_device, vk_cmd_pool, uint32_t(vk_cmd_bufs.size()), vk_cmd_bufs.data());
  Logging::Debug("Vulkan Command Buffers Destroyed!");
  vkDestroyCommandPool(vk_device, vk_cmd_pool, nullptr);
  Logging::Debug("Vulkan Command Pool Destroyed!");

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