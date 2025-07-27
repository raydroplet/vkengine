
<img src="https://github.com/raydroplet/vkengine/blob/main/etc/demo.gif?raw=true" />

> **⚠ Warning:** This repository contains a minimal Vulkan engine intended for learning, prototyping, and demonstration purposes. It is not optimized for production or full-scale game development.

# VKEngine
A simple 3D rendering engine built from the ground up using the Vulkan 1.0 API. It serves as a learning tool and a base for building more complex graphics applications.

## Features

* **Modern Graphics:** Renders 3D scenes using the explicit, low-level **Vulkan API**.
* **Advanced Architecture:**
    * A complete **Entity-Component-System (ECS)** implementation (`vke::Coordinator`) for flexible and decoupled scene management.
    * A template-based **event system** (`vke::EventRelayer`) for decoupled communication, handling events like window resizing and pipeline invalidation.
    * A custom `multilist` data structure, block-allocated component storage.
* **Resource Management:**
    * Custom memory allocator (`vke::MemAllocator`) for efficient Vulkan buffer and image management.
    * Model manager (`vke::ModelManager`) for loading and instantiating shared `.obj` 3D models using **tinyobjloader**.
    * Descriptor set abstraction (`vke::DescriptorPool`, `vke::DescriptorWriter`) to simplify shader resource binding.
* **Rendering Pipeline:**
    * Clean, high-level abstractions for core Vulkan objects like `Device`, `Swapchain`, `Pipeline`, and `Buffer`.
    * A global Uniform Buffer Object (UBO) for passing camera matrices and lighting data (ambient + single point light) to shaders.
    * System-based rendering, with distinct systems for rendering models (`RenderSystem`) and light representations (`PointLightSystem`).
* **User Interaction:** A first-person camera controller (`vke::KeyboardInput`) for navigating the scene.

## Dependencies
* **Vulkan SDK:** [https://vulkan.lunarg.com/](https://vulkan.lunarg.com/)
* **GLFW:** For windowing and input handling.
* **GLM:** For C++ mathematics (vectors, matrices).
* **tinyobjloader:** For parsing `.obj` model files.

## Building the Project
Before building the project, be sure you have the following installed on your system:
- **[Nix](https://nixos.org/download/#nix-install-linux):** `nix-shell` provides an environment with required system dependencies
- **[Xmake](https://xmake.io/):** Simple cross-platform build system

Clone the repository:

    $ git clone https://github.com/raydroplet/vkengine/
    $ cd vkengine

Enter the nix shell:

    $ nix-shell

Build the project:

    $ xmake

Run the Application:

    $ xmake run

### ⌨️ Controls
- **Move Forward/Left/Back/Right**: `W`, `A`, `S`, `D`
- **Move Up/Down**: `Spacebar`, `Left Shift`
- **Look Up/Down/Left/Right**: `Up`, `Down`, `Left`, `Right` arrow keys

## Special Acknowledgments
- [Vulkan Tutorial](https://vulkan-tutorial.com/): Vulkan API explanation and usage
- [Brenda Galea](https://www.youtube.com/@BrendanGalea): Engine architecture and avanced features
<!--
- [Modern 3D Graphics Programming](https://paroj.github.io/gltut/)
- [Vulkan Perspective Matrix](https://www.vincentparizet.com/blog/posts/vulkan_perspective_matrix/):
-->
