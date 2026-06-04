#ifndef GRAPHICS_ENGINE_HPP
#define GRAPHICS_ENGINE_HPP

#define GLFW_INCLUDE_VULKAN 
#include <GLFW/glfw3.h>

class GraphicsEngine {
  public:
    GraphicsEngine(int _width, int _height, bool _skipInit = false);
    ~GraphicsEngine();
    void update();
    bool isRunning();
  private:
    GraphicsEngine() = delete;
    void init();

    int width {};
    int height {};
    bool initialized {false};
    GLFWwindow* glfw_window = nullptr;
};

#endif