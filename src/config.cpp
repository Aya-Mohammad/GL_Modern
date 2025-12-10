#include "../include/config.h"
#include "../include/ini.h"
#include <iostream>
#include <string>
#include <cstring>

static int handler(void *user, const char *section, const char *name,
                   const char *value, int lineno)
{
    Config *pconfig = (Config *)user;

    #define MATCH(s, n) strcmp(section, s) == 0 && strcmp(name, n) == 0

    if (MATCH("window", "width"))
    {
        pconfig->width = std::stoi(value);
    }
    else if (MATCH("window", "height"))
    {
        pconfig->height = std::stoi(value);
    }
    else if (MATCH("window", "fullscreen"))
    {
        pconfig->startFullscreen = (strcmp(value, "true") == 0);
    }
    else
    {
        return 0;
    }
    return 1;
}

Config loadConfig(const std::string &filename)
{
    Config config;

    if (ini_parse(filename.c_str(), handler, &config) < 0)
    {
        std::cerr << "Warning: Could not load config file '" << filename << "'. Using default settings." << std::endl;
    }

    return config;
}
