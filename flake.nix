{
  description = "Vulkan Flake";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs =
    {
      self,
      nixpkgs,
      flake-utils,
    }:
    flake-utils.lib.eachDefaultSystem (
      system:
      let
        pkgs = import nixpkgs { inherit system; };
      in
      {
        devShells.default = pkgs.mkShell {
          name = "vkengine-env";

          nativeBuildInputs = with pkgs; [
            pkg-config
          ];

          buildInputs = with pkgs; [
            xwayland
            libXext
            libXrender
            libXfixes
            libXrandr
            libXinerama
            libXcursor
            libXi
            xmake
            glfw
            freetype
            shaderc
            vulkan-headers
            vulkan-loader
            vulkan-tools
            vulkan-validation-layers
          ];

          shellHook = ''
            export LD_LIBRARY_PATH="${
              pkgs.lib.makeLibraryPath (
                with pkgs;
                [
                  glfw
                  freetype
                  vulkan-loader
                  vulkan-validation-layers
                ]
              )
            }:$LD_LIBRARY_PATH"
            export VULKAN_SDK="${pkgs.vulkan-headers}"
            export VK_LAYER_PATH="${pkgs.vulkan-validation-layers}/share/vulkan/explicit_layer.d"
          '';
        };
      }
    );
}
