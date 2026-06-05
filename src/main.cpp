#include "GraphicsEngine.hpp"

int main(void) {
    GraphicsEngine window {1280, 720};
    while(window.isRunning()) {
        window.update();
    }
    window.~GraphicsEngine();
}
