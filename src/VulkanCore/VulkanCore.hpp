#ifndef vulkan_core_hpp
#define vulkan_core_hpp

#include <vulkan/vulkan.h>
#include <vector>
#include <memory>

class VulkanCore {
  public:
    VulkanCore();
    ~VulkanCore();
    bool Initialized();
    void Init();
    void RenderScene();
  private:
    void CreateVkInst();
    void CreateDebugCallback();

    VkInstance vk_instance = nullptr;
    VkDebugUtilsMessengerEXT vk_dbgMessenger = nullptr;
    std::unique_ptr<std::vector<const char*>> layers {};
    std::unique_ptr<std::vector<const char*>> extensions {};
};

#endif