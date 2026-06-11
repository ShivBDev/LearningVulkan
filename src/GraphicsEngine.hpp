#ifndef graphics_engine_hpp
#define graphics_engine_hpp

#define GLFW_INCLUDE_VULKAN 
#include <GLFW/glfw3.h>
#include "vk_impl/VulkanEngine.hpp"
#include <memory>
#include "shared.hpp"

class GraphicsEngine {
  public:
    GraphicsEngine(
      int _width = __app_default_resolution_width,
      int _height = __app_default_resolution_height,
      bool _skipInit = false);
    ~GraphicsEngine();
    bool IsRunning();
    void Update();
    void Init();
  private:
    int width { 0 };
    int height { 0 };
    GLFWwindow* glfw_window { nullptr };
    std::unique_ptr<VulkanEngine> vk_engine { nullptr };
};

#endif