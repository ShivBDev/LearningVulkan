#include "GraphicsEngine.hpp"

int main(void) {
    GraphicsEngine window {1280, 720};
    while(window.isRunning()) {
        window.update();
    }
}

/*
int makeVkInst() {
    printf("Hello, Vulkan!\n");

    VkInstance instance;

    VkApplicationInfo appInfo {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Learning Vulkan";
    appInfo.applicationVersion = VK_MAKE_VERSION(0, 0, 1);
    appInfo.pEngineName = "Braindead VK Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(0, 0, 1);
    appInfo.apiVersion = VK_API_VERSION_1_4;

    VkInstanceCreateInfo createInfo {};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    uint32_t extCount = 3;
    const char** extensions = new char const * [3] {nullptr};
    extensions[0] = "VK_KHR_surface";
    extensions[1] = "VK_EXT_metal_surface";
    extensions[2] = "VK_KHR_portability_enumeration";

    createInfo.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
    createInfo.enabledExtensionCount = extCount;
    createInfo.ppEnabledExtensionNames = extensions;

    VkResult vkResult = vkCreateInstance(&createInfo, nullptr, &instance);
    if(vkResult != VK_SUCCESS) {
        printf("FAILED TO CREATE VK INSTANCE: %d\n", vkResult);
        return -1;
    }

    return 0;
}
 */