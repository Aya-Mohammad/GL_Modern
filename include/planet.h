#ifndef PLANET_H
#define PLANET_H

#include "glad\glad.h"
#include <vector>
#include "glm\glm\glm.hpp"
#include "glm\glm\gtc\constants.hpp"

class Planet
{
public:
    Planet(float radius, unsigned int rings, unsigned int sectors);
    ~Planet();
    void draw();

private:
    unsigned int VAO;
    unsigned int VBO;
    unsigned int EBO;
    unsigned int indexCount;
};

#endif
