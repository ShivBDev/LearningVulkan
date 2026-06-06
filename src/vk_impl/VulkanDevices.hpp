#ifndef vulkan_devices_hpp
#define vulkan_devices_hpp
#include <vulkan/vulkan.h>
#include <vector>

struct PhysicalDevice {
  VkPhysicalDevice vk_device;
  VkPhysicalDeviceProperties device_props;
  std::vector<VkQueueFamilyProperties> queue_family_props;
  std::vector<VkBool32> supports_present;
  std::vector<VkSurfaceFormatKHR> surface_formats;
  VkSurfaceCapabilitiesKHR surface_capabilities;
  VkPhysicalDeviceMemoryProperties memory_properties;
  std::vector<VkPresentModeKHR> present_modes;
  VkPhysicalDeviceFeatures features;
};

class VulkanPhysicalDevices {
  public:
    VulkanPhysicalDevices();
    ~VulkanPhysicalDevices();
    void Init(const VkInstance& _instance, const VkSurfaceKHR& _surface);
    uint32_t SelectDevice(VkQueueFlags _reqQueueType, bool _supportsPresent);
    PhysicalDevice const& Selected() const;
  private:
    std::vector<PhysicalDevice> devices {};
    int device_index = -1;
};
#endif