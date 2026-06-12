#ifndef vulkan_engine_hpp
#define vulkan_engine_hpp

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include "VulkanDevices.hpp"
#include <chrono>

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
    void CreateShaders();
    void CreatePipeline();
    void CreateCommandBuffer();
    void RecordCommandBuffers();
  
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
    std::vector<VkSemaphore> vk_render_complete_semaphores {};
    std::vector<VkSemaphore> vk_present_complete_semaphores {};
    uint32_t curr_semaphore_idx { 0 };
    //VkSemaphore vk_render_complete_semaphore { nullptr };
    //VkSemaphore vk_present_complete_semaphore { nullptr };

    VkRenderPass vk_render_pass { nullptr };
    std::vector<VkFramebuffer> vk_frame_buffers { nullptr };

    VkShaderModule vk_vert_shader_module { nullptr };
    VkShaderModule vk_frag_shader_module { nullptr };

    VkPipeline vk_pipeline { nullptr };
    VkPipelineLayout vk_pipeline_layout { nullptr };

    VkCommandPool vk_cmd_pool { nullptr };
    std::vector<VkCommandBuffer> vk_cmd_bufs {};

    bool use_compute_shaders { false };

    uint32_t VkQueue_GetNextImg();
    void VkQueue_SubmitBuf(VkCommandBuffer const & _cmdBuf, bool const _async = true);
    void VkQueue_Present(uint32_t const _imgIdx);
    void VkQueue_WaitIdle();

    // TUTS / DBG
    void __Record__Clr__Cmd__Bufs();
    void __Print_Frame_Info(
      std::chrono::time_point<std::chrono::high_resolution_clock> __startTime
    );
    std::string __prev_msg {};
    int __force_fps { 60 };
    void __Cap_Frame_Rate(
      std::chrono::time_point<std::chrono::high_resolution_clock> __fstartTime,
      std::chrono::time_point<std::chrono::high_resolution_clock> __fendTime
    );
};

#endif