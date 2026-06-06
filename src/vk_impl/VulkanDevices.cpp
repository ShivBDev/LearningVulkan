#include "VulkanDevices.hpp"
#include "../Logging.h"

namespace {
  void __print_img_usage_flags(const VkImageUsageFlags& _flags) {
    if (_flags & VK_IMAGE_USAGE_TRANSFER_SRC_BIT) { Logging::Debug("    Image usage transfer src is supported"); }
    if (_flags & VK_IMAGE_USAGE_TRANSFER_DST_BIT) { Logging::Debug("    Image usage transfer dest is supported"); }
    if (_flags & VK_IMAGE_USAGE_SAMPLED_BIT) { Logging::Debug("    Image usage sampled is supported"); }
    if (_flags & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT) { Logging::Debug("    Image usage color attachment is supported"); }
    if (_flags & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) { Logging::Debug("    Image usage depth stencil attachment is supported"); }
    if (_flags & VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT) { Logging::Debug("    Image usage transient attachment is supported"); }
    if (_flags & VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT) { Logging::Debug("    Image usage input attachment is supported"); }
  }
  void __print_memory_properties(VkMemoryPropertyFlags _flags)
  {
    if (_flags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) { Logging::Debug("      DEVICE LOCAL"); }
    if (_flags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) { Logging::Debug("      HOST VISIBLE"); }
    if (_flags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) { Logging::Debug("      HOST COHERENT"); }
    if (_flags & VK_MEMORY_PROPERTY_HOST_CACHED_BIT) { Logging::Debug("      HOST CACHED"); }
    if (_flags & VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT) { Logging::Debug("      LAZILY ALLOCATED"); }
    if (_flags & VK_MEMORY_PROPERTY_PROTECTED_BIT) { Logging::Debug("      PROTECTED"); }
  }
  void __print_device_features(VkPhysicalDeviceFeatures const& _features) {
    if (VK_TRUE == _features.robustBufferAccess) {Logging::Debug("    ROBUST_BUFFER_ACCESS");}
    if (VK_TRUE == _features.fullDrawIndexUint32) {Logging::Debug("    FULL_DRAW_INDEX_U32");}
    if (VK_TRUE == _features.imageCubeArray) {Logging::Debug("    IMAGE_CUBE_ARRAY");}
    if (VK_TRUE == _features.independentBlend) {Logging::Debug("    INDEPENDENT_BLEND");}
    if (VK_TRUE == _features.geometryShader) {Logging::Debug("    GEOMETRY_SHADER");}
    if (VK_TRUE == _features.tessellationShader) {Logging::Debug("    TESSELLATION_SHADER");}
    if (VK_TRUE == _features.sampleRateShading) {Logging::Debug("    SAMPLE_RATE_SHADING");}
    if (VK_TRUE == _features.dualSrcBlend) {Logging::Debug("    DUAL_SOURCE_BLEND");}
    if (VK_TRUE == _features.logicOp) {Logging::Debug("    LOGICAL_OPS");}
    if (VK_TRUE == _features.multiDrawIndirect) {Logging::Debug("    MULTI_DRAW_INDIRECT");}
    if (VK_TRUE == _features.drawIndirectFirstInstance) {Logging::Debug("    DRAW_INDIRECT_FIRST_INST");}
    if (VK_TRUE == _features.depthClamp) {Logging::Debug("    DEPTH_CLAMP");}
    if (VK_TRUE == _features.depthBiasClamp) {Logging::Debug("    DEPTH_BIAS_CLAMP");}
    if (VK_TRUE == _features.fillModeNonSolid) {Logging::Debug("    FILL_MODE_NON_SOLID");}
    if (VK_TRUE == _features.depthBounds) {Logging::Debug("    DEPTH_BOUNDS");}
    if (VK_TRUE == _features.wideLines) {Logging::Debug("    WIDE_LINES");}
    if (VK_TRUE == _features.largePoints) {Logging::Debug("    LARGE_POINTS");}
    if (VK_TRUE == _features.alphaToOne) {Logging::Debug("    ALPHA_TO_ONE");}
    if (VK_TRUE == _features.multiViewport) {Logging::Debug("    MULTI_VIEWPORT");}
    if (VK_TRUE == _features.samplerAnisotropy) {Logging::Debug("    SAMPLER_ANISOTROPY");}
    if (VK_TRUE == _features.textureCompressionETC2) {Logging::Debug("    TEXTURE_COMPRESS_ETC2");}
    if (VK_TRUE == _features.textureCompressionASTC_LDR) {Logging::Debug("    TEXTURE_COMPRESS_ASTC_LDR");}
    if (VK_TRUE == _features.textureCompressionBC) {Logging::Debug("    TEXTURE_COMPRESS_BC");}
    if (VK_TRUE == _features.occlusionQueryPrecise) {Logging::Debug("    OCCLUSION_QUERY_PRECISE");}
    if (VK_TRUE == _features.pipelineStatisticsQuery) {Logging::Debug("    PIPELINE_STATISTICS_QUERY");}
    if (VK_TRUE == _features.vertexPipelineStoresAndAtomics) {Logging::Debug("    VERTEX_PIPELINE_STORES_AND_ATOMICS");}
    if (VK_TRUE == _features.fragmentStoresAndAtomics) {Logging::Debug("    FRAGMENT_STORES_AND_ATOMICS");}
    if (VK_TRUE == _features.shaderTessellationAndGeometryPointSize) {Logging::Debug("    SHADER_TESSELLATION_AND_GEOMETRY_POINT_SZ");}
    if (VK_TRUE == _features.shaderImageGatherExtended) {Logging::Debug("    SHADER_IMG_GATHER_EXT");}
    if (VK_TRUE == _features.shaderStorageImageExtendedFormats) {Logging::Debug("    SHADER_STORE_IMG_EXT_FMT");}
    if (VK_TRUE == _features.shaderStorageImageMultisample) {Logging::Debug("    SHADER_STORE_IMG_MULTI_SAMPLE");}
    if (VK_TRUE == _features.shaderStorageImageReadWithoutFormat) {Logging::Debug("    SHADER_STORE_IMG_READ_NO_FMT");}
    if (VK_TRUE == _features.shaderStorageImageWriteWithoutFormat) {Logging::Debug("    SHADER_STORE_IMG_WRITE_NO_FMT");}
    if (VK_TRUE == _features.shaderUniformBufferArrayDynamicIndexing) {Logging::Debug("    SHADER_UNIFORM_BUF_ARR_DYNAMIC_IDX");}
    if (VK_TRUE == _features.shaderSampledImageArrayDynamicIndexing) {Logging::Debug("    SHADER_SAMPLE_IMG_ARR_DYNAMIC_IDX");}
    if (VK_TRUE == _features.shaderStorageBufferArrayDynamicIndexing) {Logging::Debug("    SHADER_STORE_BUF_ARR_DYNAMIC_IDX");}
    if (VK_TRUE == _features.shaderStorageImageArrayDynamicIndexing) {Logging::Debug("    SHADER_STORE_IMG_ARR_DYNAMIC_IDX");}
    if (VK_TRUE == _features.shaderClipDistance) {Logging::Debug("    SHADER_CLIP_DIST");}
    if (VK_TRUE == _features.shaderCullDistance) {Logging::Debug("    SHADER_CULL_DIST");}
    if (VK_TRUE == _features.shaderFloat64) {Logging::Debug("    SHADER_FLOAT64");}
    if (VK_TRUE == _features.shaderInt64) {Logging::Debug("    SHADER_INT64");}
    if (VK_TRUE == _features.shaderInt16) {Logging::Debug("    SHADER_INT16");}
    if (VK_TRUE == _features.shaderResourceResidency) {Logging::Debug("    SHADER_RES_RESIDENCY");}
    if (VK_TRUE == _features.shaderResourceMinLod) {Logging::Debug("    SHADER_RES_MIN_LOD");}
    if (VK_TRUE == _features.sparseBinding) {Logging::Debug("    SPARSE_BINDING");}
    if (VK_TRUE == _features.sparseResidencyBuffer) {Logging::Debug("    SPARSE_RESIDENCY_BUF");}
    if (VK_TRUE == _features.sparseResidencyImage2D) {Logging::Debug("    SPARSE_RESIDENCY_IMG2D");}
    if (VK_TRUE == _features.sparseResidencyImage3D) {Logging::Debug("    SPARSE_RESIDENCY_IMG3D");}
    if (VK_TRUE == _features.sparseResidency2Samples) {Logging::Debug("    SPARSE_RESIDENCY_2SAMPLE");}
    if (VK_TRUE == _features.sparseResidency4Samples) {Logging::Debug("    SPARSE_RESIDENCY_4SAMPLE");}
    if (VK_TRUE == _features.sparseResidency8Samples) {Logging::Debug("    SPARSE_RESIDENCY_8SAMPLE");}
    if (VK_TRUE == _features.sparseResidency16Samples) {Logging::Debug("    SPARSE_RESIDENCY_16SAMPLE");}
    if (VK_TRUE == _features.sparseResidencyAliased) {Logging::Debug("    SPARSE_RESIDENCY_ALIASED");}
    if (VK_TRUE == _features.variableMultisampleRate) {Logging::Debug("    VARIABLE_MULTI_SAMPLE_RATE");}
    if (VK_TRUE == _features.inheritedQueries) {Logging::Debug("    INHERITED_QUERIES");}
  }

  void __print_device_info(PhysicalDevice const & _device) {
    Logging::Debug(std::format("Device name: {}", _device.device_props.deviceName));
    uint32_t apiVer = _device.device_props.apiVersion;
    Logging::Debug(std::format("Api Version: {}.{}.{}.{}",
      VK_API_VERSION_VARIANT(apiVer),
      VK_API_VERSION_MAJOR(apiVer),
      VK_API_VERSION_MINOR(apiVer),
      VK_API_VERSION_PATCH(apiVer)));
    Logging::Debug(std::format("  Num Family Queues: %{}", _device.queue_family_props.size()));
    for(uint32_t idx = 0; idx < _device.queue_family_props.size(); idx++) {
      VkQueueFamilyProperties const & queueFamilyProp = _device.queue_family_props[idx];
      Logging::Debug(std::format("    Family: {} | Num Queues: {} | ", idx, queueFamilyProp.queueCount));
      VkQueueFlags flags = queueFamilyProp.queueFlags;
      Logging::Debug(std::format("GFX: {} | Compute: {} | Transfer: {} | Sparse Binding: {}",
        (flags & VK_QUEUE_GRAPHICS_BIT) ? "Yes" : "No",
        (flags & VK_QUEUE_COMPUTE_BIT) ? "Yes" : "No",
        (flags & VK_QUEUE_TRANSFER_BIT) ? "Yes" : "No",
        (flags & VK_QUEUE_SPARSE_BINDING_BIT) ? "Yes" : "No"));
    }
    Logging::Debug("  Surface Formats:");
    for(uint32_t idx = 0; idx < _device.surface_formats.size(); idx++) {
      VkSurfaceFormatKHR const & surfaceFormat = _device.surface_formats[idx];
      Logging::Debug(std::format("    Format: {:x} | Color Space: {:x}",
        uint32_t(surfaceFormat.format), uint32_t(surfaceFormat.colorSpace)));
    }
    Logging::Debug("  Image Usage Flags:");
    __print_img_usage_flags(_device.surface_capabilities.supportedUsageFlags);
    Logging::Debug(std::format("  Num Presentation Modes: {}", _device.present_modes.size()));
    Logging::Debug(std::format("  Num Memory Types: {}", _device.memory_properties.memoryTypeCount));
    for(uint32_t idx = 0; idx < _device.memory_properties.memoryTypeCount; idx++) {
      Logging::Debug(std::format("    Mem Type {} | flags: {} | heap: {}",
        idx,
        _device.memory_properties.memoryTypes[idx].propertyFlags,
        _device.memory_properties.memoryTypes[idx].heapIndex));
      __print_memory_properties(_device.memory_properties.memoryTypes[idx].propertyFlags);
    }
    Logging::Debug(std::format("  Num Heap Types: {}", _device.memory_properties.memoryHeapCount));
    Logging::Debug("  Device Features:");
    __print_device_features(_device.features);
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
        Logging::Debug(std::format("Using GFX Device {} and Queue Family {}", device_index, queueFam));
        return queueFam;
      }
    }
  }
  throw Logging::Error(std::format("Required Queue Type {} and Supports Present {} not found.",
    _reqQueueType, _supportsPresent));
}

PhysicalDevice const& VulkanPhysicalDevices::Selected() const {
  if (device_index < 0) { throw Logging::Error("A Physical Device has not been selected!"); }
  return devices[device_index];
}

void VulkanPhysicalDevices::Init(const VkInstance& _instance, const VkSurfaceKHR& _surface) {
  Logging::Debug("Initializing Vulkan Physical Devices...");
  uint32_t numDevices {};
  VkResult result = vkEnumeratePhysicalDevices(_instance, &numDevices, nullptr);
  if(result != VK_SUCCESS) {
    throw Logging::Error("vk: Failed to Enumerate Physical Devices.");
  }
  devices.resize(numDevices);

  std::vector<VkPhysicalDevice> vkDevices {};
  vkDevices.resize(numDevices);
  result = vkEnumeratePhysicalDevices(_instance, &numDevices, vkDevices.data());
  if(result != VK_SUCCESS) {
    throw Logging::Error("vk: Failed to Fetch Physical Devices.");
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
        Logging::Error("Error getting device surface support for KHR");
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
    // features
    vkGetPhysicalDeviceFeatures(vkDevice, &newDevice.features);

    Logging::Debug(std::format("Physical Device {}", idx));
    __print_device_info(newDevice);
    Logging::Debug("Vulkan Physical Devices Initialized.");
  }
}