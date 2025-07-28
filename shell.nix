with import <nixpkgs> {};
stdenv.mkDerivation {
  name = "vkengine-env";
  nativeBuildInputs = [ pkg-config ];
  buildInputs = [
    xwayland
    xorg.libXext
    xorg.libXrender
    xorg.libXfixes
    xorg.libXrandr
    xorg.libXinerama
    xorg.libXcursor
    xorg.libXi
    # libxkbcommon
    xmake
    glfw
    shaderc
    vulkan-headers
    vulkan-loader
    vulkan-tools
    vulkan-validation-layers
  ];

  LD_LIBRARY_PATH="${glfw}/lib:${freetype}/lib:${vulkan-loader}/lib:${vulkan-validation-layers}/lib";
  VULKAN_SDK = "${vulkan-headers}";
  VK_LAYER_PATH = "${vulkan-validation-layers}/share/vulkan/explicit_layer.d";
}
