#ifndef vulkan_core_hpp
#define vulkan_core_hpp

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <vector>
#include <memory>
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
    void CreateDebugCallback();
    void CreateSurface();

    VkInstance vk_instance = nullptr;
    VkDebugUtilsMessengerEXT vk_dbgMessenger = nullptr;
    VkSurfaceKHR vk_surface = nullptr;
    GLFWwindow* glfw_window = nullptr;

    VulkanPhysicalDevices physical_devices {};
    uint32_t queue_family {};
};

#endif