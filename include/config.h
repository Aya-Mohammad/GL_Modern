#ifndef CONFIG_H
#define CONFIG_H

#include <string>

struct Config
{
    int width = 800;
    int height = 600;
    bool startFullscreen = false;
};

Config loadConfig(const std::string &filename);

#endif // CONFIG_H
