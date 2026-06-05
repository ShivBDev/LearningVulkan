#ifndef graphics_engine_hpp
#define graphics_engine_hpp

#define GLFW_INCLUDE_VULKAN 
#include <GLFW/glfw3.h>
#include "VulkanCore/VulkanCore.hpp"

class GraphicsEngine {
  public:
    GraphicsEngine(int _width, int _height, bool _skipInit = false);
    ~GraphicsEngine();
    bool isRunning();
    void update();
    void init();
  private:
    GraphicsEngine() = delete;
    
    int width {};
    int height {};
    GLFWwindow* glfw_window = nullptr;
    std::unique_ptr<VulkanCore> vk_core = nullptr;
};

#endif