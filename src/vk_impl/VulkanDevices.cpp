#include "VulkanDevices.hpp"

namespace {
  void __print_img_usage_flags(const VkImageUsageFlags& _flags) {
    if (_flags & VK_IMAGE_USAGE_TRANSFER_SRC_BIT) { printf("\t\tImage usage transfer src is supported\n"); }
    if (_flags & VK_IMAGE_USAGE_TRANSFER_DST_BIT) { printf("\t\tImage usage transfer dest is supported\n"); }
    if (_flags & VK_IMAGE_USAGE_SAMPLED_BIT) { printf("\t\tImage usage sampled is supported\n"); }
    if (_flags & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT) { printf("\t\tImage usage color attachment is supported\n"); }
    if (_flags & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) { printf("\t\tImage usage depth stencil attachment is supported\n"); }
    if (_flags & VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT) { printf("\t\tImage usage transient attachment is supported\n"); }
    if (_flags & VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT) { printf("\t\tImage usage input attachment is supported\n"); }
  }
  void __print_memory_properties(VkMemoryPropertyFlags _flags)
  {
    if (_flags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) { printf("DEVICE LOCAL\n"); }
    if (_flags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) { printf("HOST VISIBLE\n"); }
    if (_flags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) { printf("HOST COHERENT\n"); }
    if (_flags & VK_MEMORY_PROPERTY_HOST_CACHED_BIT) { printf("HOST CACHED\n"); }
    if (_flags & VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT) { printf("LAZILY ALLOCATED\n"); }
    if (_flags & VK_MEMORY_PROPERTY_PROTECTED_BIT) { printf("PROTECTED\n"); }
}

  void __print_device_info(PhysicalDevice const & _device) {
    printf("Device name: %s\n", _device.device_props.deviceName);
    uint32_t apiVer = _device.device_props.apiVersion;
    printf("Api Version: %d.%d.%d.%d\n",
      VK_API_VERSION_VARIANT(apiVer),
      VK_API_VERSION_MAJOR(apiVer),
      VK_API_VERSION_MINOR(apiVer),
      VK_API_VERSION_PATCH(apiVer));
    printf("\tNum Family Queues: %zu\n", _device.queue_family_props.size());
    for(uint32_t idx = 0; idx < _device.queue_family_props.size(); idx++) {
      VkQueueFamilyProperties const & queueFamilyProp = _device.queue_family_props[idx];
      printf("\t\tFamily: %d | Num Queues: %d", idx, queueFamilyProp.queueCount);
      VkQueueFlags flags = queueFamilyProp.queueFlags;
      printf("\t\tGFX: %s | Compute: %s | Transfer: %s | Sparse Binding: %s\n",
        (flags & VK_QUEUE_GRAPHICS_BIT) ? "Yes" : "No",
        (flags & VK_QUEUE_COMPUTE_BIT) ? "Yes" : "No",
        (flags & VK_QUEUE_TRANSFER_BIT) ? "Yes" : "No",
        (flags & VK_QUEUE_SPARSE_BINDING_BIT) ? "Yes" : "No");
    }
    printf("\tSurface Formats:\n");
    for(uint32_t idx = 0; idx < _device.surface_formats.size(); idx++) {
      VkSurfaceFormatKHR const & surfaceFormat = _device.surface_formats[idx];
      printf("\t\tFormat: %x | Color Space: %x\n",
        surfaceFormat.format, surfaceFormat.colorSpace);
    }
    printf("\tImage Usage Flags:\n");
    __print_img_usage_flags(_device.surface_capabilities.supportedUsageFlags);
    printf("\tNum Presentation Modes: %zu\n", _device.present_modes.size());
    printf("\tNum Memory Types: %d\n", _device.memory_properties.memoryTypeCount);
    for(uint32_t idx = 0; idx < _device.memory_properties.memoryTypeCount; idx++) {
      printf("\t\tMem Type %d | flags: %x | heap: %d\n",
        idx,
        _device.memory_properties.memoryTypes[idx].propertyFlags,
        _device.memory_properties.memoryTypes[idx].heapIndex);
      __print_memory_properties(_device.memory_properties.memoryTypes[idx].propertyFlags);
    }
    printf("\tNum Heap Types: %d\n", _device.memory_properties.memoryHeapCount);
    printf("\n");
  }
}

VulkanPhysicalDevices::VulkanPhysicalDevices() {}
VulkanPhysicalDevices::~VulkanPhysicalDevices() {}

uint32_t VulkanPhysicalDevices::SelectDevice(VkQueueFlags _reqQueueType, bool _supportsPresent) {
  for(uint32_t idx = 0; idx < devices.size(); idx++) {
    PhysicalDevice const& device = devices[idx];
    for(uint32_t idx2 = 0; idx2 < device.queue_family_props.size(); idx2++) {
      VkQueueFamilyProperties const& props = device.queue_family_props[idx2];
      if ((props.queueFlags & _reqQueueType) &&
      bool(device.supports_present[idx2]) == _supportsPresent) {
        device_index = idx;
        int queueFam = idx2;
        printf("Using GFX Device %d and Queue Family %d\n", device_index, queueFam);
        return queueFam;
      }
    }
  }
  printf("Required Queue Type %x and Supports Present %d not found.\n", _reqQueueType, _supportsPresent);
  throw "Error finding device w/ required queue type and presentation support.";
}

PhysicalDevice const& VulkanPhysicalDevices::Selected() const {
  if (device_index < 0) { throw "A Physical Device has not been selected!\n"; }
  return devices[device_index];
}

void VulkanPhysicalDevices::Init(const VkInstance& _instance, const VkSurfaceKHR& _surface) {
  uint32_t numDevices {};
  VkResult result = vkEnumeratePhysicalDevices(_instance, &numDevices, nullptr);
  if(result != VK_SUCCESS) {
    throw "vk: Failed to Enumerate Physical Devices.\n";
  }
  devices.resize(numDevices);

  std::vector<VkPhysicalDevice> vkDevices {};
  vkDevices.resize(numDevices);
  result = vkEnumeratePhysicalDevices(_instance, &numDevices, vkDevices.data());
  if(result != VK_SUCCESS) {
    throw "vk: Failed to Fetch Physical Devices.";
  }

  for(uint32_t idx = 0; idx < numDevices; idx++) {
    // track vk device
    VkPhysicalDevice& vkDevice = vkDevices[idx];
    PhysicalDevice& newDevice = devices[idx];
    newDevice.vk_device = vkDevice;
    // get device props
    vkGetPhysicalDeviceProperties(vkDevice, &newDevice.device_props);
    // get queue family data
    uint32_t numQFamilies {};
    vkGetPhysicalDeviceQueueFamilyProperties(vkDevice, &numQFamilies, nullptr);
    newDevice.queue_family_props.resize(numQFamilies);
    newDevice.supports_present.resize(numQFamilies);
    vkGetPhysicalDeviceQueueFamilyProperties(vkDevice, &numQFamilies, newDevice.queue_family_props.data());
    for(uint32_t idx2 = 0; idx2 < numQFamilies; idx2++) {
      VkQueueFamilyProperties const & queueFamilyProp = newDevice.queue_family_props[idx2];
      result = vkGetPhysicalDeviceSurfaceSupportKHR(
        vkDevice, idx2, _surface, &newDevice.supports_present[idx2]);
      if (result != VK_SUCCESS) {
        printf("Error getting device surface support for KHR");
        newDevice.supports_present[idx2] = false;
      }
    }
    // surface formats
    uint32_t numFormats {};
    result = vkGetPhysicalDeviceSurfaceFormatsKHR(vkDevice, _surface, &numFormats, nullptr);
    if(result != VK_SUCCESS || numFormats <= 0) { throw "vk: Error Getting Physical Device Surface Formats KHR."; }
    newDevice.surface_formats.resize(numFormats);
    result = vkGetPhysicalDeviceSurfaceFormatsKHR(vkDevice, _surface, &numFormats, newDevice.surface_formats.data());
    if(result != VK_SUCCESS) { throw "vk: Error Getting Physical Device Surface Formats KHR."; }
    result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vkDevice, _surface, &newDevice.surface_capabilities);
    if(result != VK_SUCCESS) { throw "vk: Error Getting Physical Device Surface Capabilities KHR."; }
    // presentation modes
    uint32_t numPresentModes {};
    result = vkGetPhysicalDeviceSurfacePresentModesKHR(vkDevice, _surface, &numPresentModes, nullptr);
    if(result != VK_SUCCESS || numPresentModes <= 0) { throw "vk: error getting physical device surface presentation modes."; }
    newDevice.present_modes.resize(numPresentModes);
    result = vkGetPhysicalDeviceSurfacePresentModesKHR(vkDevice, _surface, &numPresentModes, newDevice.present_modes.data());
    if(result != VK_SUCCESS) { throw "vk: error getting physical device surface presentation modes."; }
    // memory properties
    vkGetPhysicalDeviceMemoryProperties(vkDevice, &newDevice.memory_properties);

    __print_device_info(newDevice);
  }
}