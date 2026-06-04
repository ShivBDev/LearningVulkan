#include "GraphicsEngine.hpp"

namespace {
  void GLFW_KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
  }
}

GraphicsEngine::~GraphicsEngine() {
  glfwTerminate();
  glfw_window = nullptr;
}

GraphicsEngine::GraphicsEngine(int _width, int _height, bool _skipInit) {
  width = _width;
  height = _height;
  if (!_skipInit) {
    init();
  }
}

bool GraphicsEngine::isRunning() {
  return !glfwWindowShouldClose(glfw_window);
}

void GraphicsEngine::update() {
  glfwPollEvents();
}

void GraphicsEngine::init() {
  if (!glfwInit() || !glfwVulkanSupported()) {
      return;
  }
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

  glfw_window = glfwCreateWindow(width, height, "Tut01", NULL, NULL);

  if(!glfw_window) {
      glfwTerminate();
      glfw_window = nullptr;
      return;
  }

  glfwSetKeyCallback(glfw_window, GLFW_KeyCallback);
}
