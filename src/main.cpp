#include "GraphicsEngine.hpp"

int main(void) {
    GraphicsEngine window {640, 360};
    while(window.IsRunning()) {
        window.Update();
    }
    window.~GraphicsEngine();
}
