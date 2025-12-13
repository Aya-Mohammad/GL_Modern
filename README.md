<img width="749" height="351" alt="image" src="https://github.com/user-attachments/assets/7d5f25fd-97b0-4918-b441-26e973f4caf6" />

# 🌌 Solar System (OpenGL C++)

A simple solar system simulation built with C++ and modern OpenGL.
This project demonstrates core computer graphics concepts including **texturing, lighting, animation, hierarchical transformations, skyboxes, and eclipses**.

---

## ✨ Features
- **OpenGL Rendering**: Uses modern OpenGL for rendering.
- **Textured Planets**: Celestial bodies (Sun, Earth, Moon) textured using images sourced from NASA/SolarSystemScope.
- **Phong Lighting**: Basic Phong lighting model with the Sun as the primary light source. Emissive texture for the Sun.
- **Animation**: The Earth revolves around the Sun. The Moon revolves around the Earth.
- **Eclipses**: Press `G` or `H` to trigger eclipse conditions. Exit with `J`.
- **Skybox**: Star-filled skybox using cubemap textures (NASA SVS visualization #4851).
- **Camera Locking**: Lock the camera to orbit planets using number keys. Unlock with `N`.
- **Configuration File**: Uses `config.ini` to set resolution and fullscreen state.
- **Keyboard Controls**: Simulation speed and camera locking controlled via shortcuts.
- **Cross-Platform Build**: Uses **CMake** for build configuration.

---

## ⚙️ Building and Running
1. Ensure **VS Code** with the *C/C++* extension installed.
2. Clone the repository:
   ```bash
   git clone <your-repo-path>
   cd GL_Modern
3. Open the project folder in VS Code.
4. Compile:
g++ src/main.cpp src/glad.c src/ini.c src/scenario.cpp src/config.cpp src/shader.cpp src/planet.cpp src/camera.cpp src/stb_image.cpp \
-Iinclude -Iinclude/glad -Iinclude/GLFW -Iinclude/glm -Iinclude/stb \
-Llib -lglfw3 -lopengl32 -lgdi32 -o SolarSystem.exe
5. Run:
./SolarSystem.exe

---

## 🎮 Controls
- W, A, S, D: Move camera horizontally (Free mode only)
- Space: Move camera up (absolute Y) (Free mode only)
- Left Control: Sprint (increase movement speed) (Free mode only)
- Mouse: Look around (Free mode) / Orbit target (Locked mode)
- Scroll Wheel: Zoom FOV (Free mode) / Adjust distance (Locked mode)
- G / H: Trigger eclipses
- J: Exit eclipse mode
- N: Unlock camera

---

## 📚 Credits & Sources
- Libraries: GLAD, GLFW, GLM, stb_image, inih
- Planet Textures: Solar System Scope

- Skybox Textures: NASA SVS visualization #4851

