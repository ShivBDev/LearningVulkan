#ifndef vulkan_core_hpp
#define vulkan_core_hpp

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include "VulkanDevices.hpp"

class VulkanCore {
  public:
    VulkanCore();
    ~VulkanCore();
    bool Initialized();
    void Init(GLFWwindow* _glfw_window);
    void RenderScene();
  private:
    void CreateVkInst();
    VkInstance vk_instance { nullptr };

    void CreateDebugCallback();
    VkDebugUtilsMessengerEXT vk_dbgMessenger { nullptr };

    void CreateSurface();
    GLFWwindow* glfw_window { nullptr };
    VkSurfaceKHR vk_surface { nullptr };

    void CreateDevice();
    VulkanPhysicalDevices physical_devices {};
    uint32_t queue_family {};
    VkDevice vk_logical_device {};

    void CreateSwapChain();
    VkSwapchainKHR swap_chain {};
    std::vector<VkImage> swap_chain_images {};
    std::vector<VkImageView> swap_chain_image_views {};

    void CreateCommandBuffer();
    VkCommandPool vk_cmd_pool {};
    std::vector<VkCommandBuffer> vk_cmd_bufs {};
    
    bool use_compute_shaders { false };
};

#endif