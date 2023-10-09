#include <iostream>
#include "app.h"

int main()
{
    glm::ivec2 resolution = glm::ivec2(640, 360) * 2;
    App app(resolution.x, resolution.y, "Flightsim");
    return app.run();
}
