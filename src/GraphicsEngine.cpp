#include "GraphicsEngine.hpp"
#include "shared.hpp"

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
  vk_core = nullptr;
}

GraphicsEngine::GraphicsEngine(int _width, int _height, bool _skipInit) {
  width = _width;
  height = _height;
  if (!_skipInit) {
    Init();
  }
}

bool GraphicsEngine::IsRunning() {
  return glfw_window != nullptr &&
    vk_core->Initialized() &&
    !glfwWindowShouldClose(glfw_window);
}

void GraphicsEngine::Update() {
  vk_core->RenderScene();
  glfwPollEvents();
}

void GraphicsEngine::Init() {
  if (!glfwInit() || !glfwVulkanSupported()) {
      return;
  }
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

  glfw_window = glfwCreateWindow(width, height, __engine_name, NULL, NULL);

  if(!glfw_window) {
      glfwTerminate();
      glfw_window = nullptr;
      return;
  }

  glfwSetKeyCallback(glfw_window, GLFW_KeyCallback);

  vk_core = std::make_unique<VulkanCore>();
  vk_core->Init(glfw_window);
}
