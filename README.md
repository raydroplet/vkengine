
<img src="https://github.com/raydroplet/vkengine/blob/main/etc/demo.gif?raw=true" />

> **⚠ Warning:** This repository contains a minimal Vulkan engine intended for learning, prototyping, and demonstration purposes. It is not optimized for production or full-scale game development.

# VKEngine
A simple 3D rendering engine built from the ground up using the Vulkan 1.0 API. It serves as a learning tool and a base for building more complex graphics applications.

## Features

### Graphics & Rendering
- **Vulkan API**: Low-level rendering with predictable performance and fine-grained GPU control
- **3D Model Loading:** `.obj` file support with automatic buffer generation via **tinyobjloader**
- **Lighting System:** Ambient + point light rendering with proper normal calculations

### Architecture
- **Entity-Component-System:** Custom ECS implementation (`vke::Coordinator`) for flexible scene management
- **Event System:** Template-based event handling for window resizing, pipeline recreation, and input processing

### Resource Management
- **Memory Allocator:** Custom Vulkan memory management with automatic allocation and alignment
- **Asset Sharing:** Centralized model manager for efficient resource reuse across entities
- **Descriptor Abstractions:** Simplified shader resource binding with type-safe updates

### Vulkan Abstractions
- **High-Level Wrappers:** Clean C++ abstractions for `Device`, `Swapchain`, `Pipeline`, and `Buffer`
- **Automatic Synchronization:** Proper frame-in-flight handling with semaphores and fences

### User Interaction
- **First-Person Camera:** WASD movement with arrow key look controls and configurable speeds

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

## Acknowledgments
- [Vulkan Tutorial](https://vulkan-tutorial.com/): Vulkan API explanation and usage
- [Brenda Galea](https://www.youtube.com/@BrendanGalea): Engine architecture and avanced features
<!--
- [Modern 3D Graphics Programming](https://paroj.github.io/gltut/)
- [Vulkan Perspective Matrix](https://www.vincentparizet.com/blog/posts/vulkan_perspective_matrix/):
-->
