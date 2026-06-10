#include "GraphicsEngine.hpp"
#include "shared.hpp"
#include "Logging.hpp"

namespace {
  void GLFW_KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
  } 
}

GraphicsEngine::~GraphicsEngine() {
  Logging::Debug("Entering Graphics Engine Destructor...");
  vk_engine = nullptr;
  Logging::Log("Tearing Down GLFW instance");
  glfwTerminate();
  glfw_window = nullptr;
}

GraphicsEngine::GraphicsEngine(int _width, int _height, bool _skipInit) {
  Logging::Debug("Entering Graphics Engine Constructor...");
  width = _width;
  height = _height;
  if (!_skipInit) {
    Init();
  }
}

bool GraphicsEngine::IsRunning() {
  return glfw_window != nullptr &&
    !glfwWindowShouldClose(glfw_window);
}

void GraphicsEngine::Update() {
  vk_engine->RenderScene();
  glfwPollEvents();
}

void GraphicsEngine::Init() {
  Logging::Log("Initializing Graphics Engine...");

  if (!glfwInit() || !glfwVulkanSupported()) {
    throw Logging::Error("Failed to init glfw!");
  }
  Logging::Debug("Glfw initialized.");

  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
  glfw_window = glfwCreateWindow(width, height, __engine_name, NULL, NULL);
  if(!glfw_window) {
      glfwTerminate();
      glfw_window = nullptr;
      throw Logging::Error("Failed to create glfw window!");
  }
  glfwSetKeyCallback(glfw_window, GLFW_KeyCallback);
  Logging::Debug("Glfw window created.");

  vk_engine = std::make_unique<VulkanEngine>(glfw_window);
  Logging::Log("Graphics Engine Initialized.");
}
