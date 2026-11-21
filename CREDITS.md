# Third-Party Libraries, Course Material & Asset Credits

All 3D models (DeLorean DMC-12, Akira Bike, 8 biome cityscapes, LRT viaducts, trains, mountains, palm trees, collectibles, and 2D bitmap font) and all GLSL 330 core shaders in this repository are **procedurally generated in C++ and GLSL** by the team. No external copyrighted 3D models, image textures, or audio files are bundled.

## Third-Party C/C++ Libraries (Fetched Automatically via CMake `FetchContent`)

| Library | Version | Author / Maintainer | Official Link | License | Purpose |
| :--- | :--- | :--- | :--- | :--- | :--- |
| **GLFW** | 3.4 | Marcus Geelnard & Camilla Löwy | https://github.com/glfw/glfw | zlib/libpng | Cross-platform OpenGL 3.3 Core window creation and keyboard/mouse input |
| **GLEW** (`glew-cmake`) | 2.2.0 | Milan Ikits, Marcelo Magallon & Perlmint (`glew-cmake`) | https://github.com/Perlmint/glew-cmake | Modified BSD / MIT | OpenGL 3.3 Core Profile extension and function pointer loading |
| **GLM** | 1.0.1 | Christophe Riccio (G-Truc Creation) | https://github.com/g-truc/glm | Happy Bunny / MIT | 3D vector, matrix, perspective projection, and view-transform math |

## Course Material & Educational References

- **Computer Graphics Fundamentals Lectures & Labs** — Instructor **Nazira Shaimerdenova**, Astana IT University (OpenGL 3.3 Core pipeline, transformation hierarchies, Blinn-Phong illumination, procedural shaders, framebuffers).
- **LearnOpenGL** — Joey de Vries, https://learnopengl.com/ (CC BY-NC 4.0). Referenced for modern OpenGL VAO/VBO/FBO setup, instancing attribute divisors, and shading pipeline conventions; all shaders and scene modules in this repository were written and substantially extended for this project.
- **Interactive Computer Graphics: A Top-Down Approach with Shader-Based OpenGL (6th ed.)** — Edward Angel & Dave Shreiner, Pearson, 2012.
