
#include <assert.h>
#include <embedder.h>
#include <glfw3.h>
#include <chrono>
#include <iostream>

// If your flutter app is drawing at the wrong size you may need to adjust your
// pixel ratio.
static const double kPixelRatio = 2.0;
static const size_t kInitialWindowWidth = 800;
static const size_t kInitialWindowHeight = 600;

static_assert(FLUTTER_ENGINE_VERSION == 1, "");

void GLFWcursorPositionCallbackAtPhase(GLFWwindow* window,
                                       FlutterPointerPhase phase,
                                       double x,
                                       double y) {
  FlutterPointerEvent event = {};
  event.struct_size = sizeof(event);
  event.phase = phase;
  event.x = x * kPixelRatio;
  event.y = y * kPixelRatio;
  event.timestamp =
      std::chrono::duration_cast<std::chrono::microseconds>(
          std::chrono::high_resolution_clock::now().time_since_epoch())
          .count();
  FlutterEngineSendPointerEvent(
      reinterpret_cast<FlutterEngine>(glfwGetWindowUserPointer(window)), &event,
      1);
}

void GLFWcursorPositionCallback(GLFWwindow* window, double x, double y) {
  GLFWcursorPositionCallbackAtPhase(window, FlutterPointerPhase::kMove, x, y);
}

void GLFWmouseButtonCallback(GLFWwindow* window,
                             int key,
                             int action,
                             int mods) {
  if (key == GLFW_MOUSE_BUTTON_1 && action == GLFW_PRESS) {
    double x, y;
    glfwGetCursorPos(window, &x, &y);
    GLFWcursorPositionCallbackAtPhase(window, FlutterPointerPhase::kDown, x, y);
    glfwSetCursorPosCallback(window, GLFWcursorPositionCallback);
  }

  if (key == GLFW_MOUSE_BUTTON_1 && action == GLFW_RELEASE) {
    double x, y;
    glfwGetCursorPos(window, &x, &y);
    GLFWcursorPositionCallbackAtPhase(window, FlutterPointerPhase::kUp, x, y);
    glfwSetCursorPosCallback(window, nullptr);
  }
}

static void GLFWKeyCallback(GLFWwindow* window,
                            int key,
                            int scancode,
                            int action,
                            int mods) {
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
    glfwSetWindowShouldClose(window, GLFW_TRUE);
  }
}

void GLFWwindowSizeCallback(GLFWwindow* window, int width, int height) {
  FlutterWindowMetricsEvent event = {};
  event.struct_size = sizeof(event);
  event.width = width * kPixelRatio;
  event.height = height * kPixelRatio;
  event.pixel_ratio = kPixelRatio;
  FlutterEngineSendWindowMetricsEvent(
      reinterpret_cast<FlutterEngine>(glfwGetWindowUserPointer(window)),
      &event);
}

bool RunFlutter(GLFWwindow* window,
                const std::string& project_path,
                const std::string& icudtl_path) {
  FlutterRendererConfig config = {};
  config.type = kOpenGL;
  config.open_gl.struct_size = sizeof(config.open_gl);
  config.open_gl.make_current = [](void* userdata) -> bool {
    glfwMakeContextCurrent((GLFWwindow*)userdata);
    return true;
  };
  config.open_gl.clear_current = [](void*) -> bool {
    glfwMakeContextCurrent(nullptr);  // is this even a thing?
    return true;
  };
  config.open_gl.present = [](void* userdata) -> bool {
    glfwSwapBuffers((GLFWwindow*)userdata);
    return true;
  };
  config.open_gl.fbo_callback = [](void*) -> uint32_t {
    return 0;  // FBO0
  };

  // This directory is generated by `flutter build bundle`.
  std::string assets_path = project_path + "/build/flutter_assets";
  FlutterProjectArgs args = {
      .struct_size = sizeof(FlutterProjectArgs),
      .assets_path = assets_path.c_str(),
      .icu_data_path =
          icudtl_path.c_str(),  // Find this in your bin/cache directory.
  };
  FlutterEngine engine = nullptr;
  auto result = FlutterEngineRun(FLUTTER_ENGINE_VERSION, &config,  // renderer
                                 &args, window, &engine);
  assert(result == kSuccess && engine != nullptr);

  glfwSetWindowUserPointer(window, engine);
  GLFWwindowSizeCallback(window, kInitialWindowWidth, kInitialWindowHeight);

  return true;
}

void printUsage() {
  std::cout << "usage: flutter_glfw <path to project> <path to icudtl.dat>"
            << std::endl;
}

int main(int argc, const char* argv[]) {
  if (argc != 3) {
    printUsage();
    return 1;
  }

  std::string project_path = argv[1];
  std::string icudtl_path = argv[2];

  auto result = glfwInit();
  assert(result == GLFW_TRUE);

  auto window = glfwCreateWindow(kInitialWindowWidth, kInitialWindowHeight,
                                 "Flutter", NULL, NULL);
  assert(window != nullptr);

  bool runResult = RunFlutter(window, project_path, icudtl_path);
  assert(runResult);

  glfwSetKeyCallback(window, GLFWKeyCallback);
  glfwSetWindowSizeCallback(window, GLFWwindowSizeCallback);
  glfwSetMouseButtonCallback(window, GLFWmouseButtonCallback);

  while (!glfwWindowShouldClose(window)) {
    std::cout << "Looping..." << std::endl;
    glfwWaitEvents();
  }

  glfwDestroyWindow(window);
  glfwTerminate();

  return EXIT_SUCCESS;
}
