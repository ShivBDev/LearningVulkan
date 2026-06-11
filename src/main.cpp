#include "GraphicsEngine.hpp"

int main(void) {
    GraphicsEngine window { };
    while(window.IsRunning()) {
        window.Update();
    }
}
