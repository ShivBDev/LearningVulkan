#ifndef vulkan_engine_hpp
#define vulkan_engine_hpp

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include "VulkanDevices.hpp"

class VulkanEngine {
  public:
    VulkanEngine(GLFWwindow* _glfw_window);
    ~VulkanEngine();
    void RenderScene();
  private:
    void CreateVulkanInstance();
    void CreateDbgCallback();
    void CreateSurface();
    void CreateDevice();
    void CreateSwapChain();
    void CreateQueue();

    void CreateSimpleRenderPass();
    void CreateFrameBuffers();
    void CreateCommandBuffer();
    void RecordCommandBuffers();
    void CreateShaders();
  
    VkInstance vk_instance { nullptr };
    VkDebugUtilsMessengerEXT vk_dbgMessenger { nullptr };

    GLFWwindow* glfw_window { nullptr };
    VkSurfaceKHR vk_surface { nullptr };

    VulkanPhysicalDevices vk_physical_devices {};
    uint32_t vk_queue_family { 0 };
    VkDevice vk_device { nullptr };

    VkSurfaceFormatKHR vk_surface_format { }; 
    VkSwapchainKHR vk_swapchain { nullptr };
    std::vector<VkImage> vk_swapchain_imgs {};
    std::vector<VkImageView> vk_swapchain_img_views {};

    VkQueue vk_queue { nullptr };
    VkSemaphore vk_render_complete_semaphore { nullptr };
    VkSemaphore vk_present_complete_semaphore { nullptr };

    VkRenderPass vk_render_pass { nullptr };
    std::vector<VkFramebuffer> vk_frame_buffers { nullptr };

    VkCommandPool vk_cmd_pool { nullptr };
    std::vector<VkCommandBuffer> vk_cmd_bufs {};

    VkShaderModule vk_vert_shader_module { nullptr };
    VkShaderModule vk_frag_shader_module { nullptr };

    bool use_compute_shaders { false };

    uint32_t VkQueue_GetNextImg();
    void VkQueue_SubmitBuf(VkCommandBuffer const & _cmdBuf, bool const _async = true);
    void VkQueue_Present(uint32_t const _imgIdx);
    void VkQueue_WaitIdle();
    // TUTS
    void __Record__Clr__Cmd__Bufs();
};

#endif