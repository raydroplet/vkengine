# VKEngine

<img src="https://github.com/raydroplet/vkengine/blob/main/etc/demo.gif?raw=true" style="width:500px;" />

3D rendering engine built from the ground up using the Vulkan 1.0 API. \
It serves as a learning tool and a base for building more complex graphics applications.


## Running the Project
Before running the project, make sure you have the following installed on your system:
- **[Nix](https://nixos.org/download/#nix-install-linux):** Provides an environment with required system dependencies
- **[Xmake](https://xmake.io/):** Simple cross-platform build system

Clone and build the project:

    $ git clone https://github.com/raydroplet/vkengine
    $ cd vkengine
    $ nix develop
    $ xmake build

Run the Application:

    $ xmake run

### ⌨️ Controls
- **Move Forward/Left/Back/Right**: `W`, `A`, `S`, `D`
- **Move Up/Down**: `Spacebar`, `Left Shift`
- **Look Up/Down/Left/Right**: `Up`, `Down`, `Left`, `Right` arrow keys

## Acknowledgments
- [Vulkan Tutorial](https://vulkan-tutorial.com/): Vulkan API explanation and usage
- [Brenda Galea](https://www.youtube.com/@BrendanGalea): Engine architecture and advanced features
- [Modern 3D Graphics Programming](https://paroj.github.io/gltut/): 3D graphics development fundamentals
- [Vulkan Perspective Matrix](https://www.vincentparizet.com/blog/posts/vulkan_perspective_matrix/): Projection matrix mathematics and coordinate mapping
