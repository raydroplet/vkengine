--- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- ---

set_toolchains "gcc"
set_languages "c++20"

set_defaultmode "debug"
set_policy("build.warning", true)

add_rules("mode.release", "mode.debug")
add_requires("vulkansdk", "glfw", "glm", "tinyobjloader")

add_requires("glslang", { configs = { binaryonly = true } })

if is_mode "debug" then
  -- local sanitize = { "-fsanitize=undefined", "-fsanitize=leak" }
  -- local warnings = { "-Wall", "-Wextra", "-Weffc++", "-Wshadow" }
  -- "-Wfatal-errors"

  -- add_cxxflags(warnings, sanitize, "-pedantic-errors")
  -- add_ldflags(sanitize, "-lubsan")
end

target "program"
set_default(true)
set_kind "binary"
add_defines "GLM_ENABLE_EXPERIMENTAL"
add_packages("vulkansdk", "glfw", "glm", "tinyobjloader")
add_includedirs "include"
add_files "src/**.cpp"

target "shaders"
set_kind "shared"
add_rules("utils.glsl2spv", { outputdir = "build/shaders/" })
add_files("shaders/*.vert", "shaders/*.frag")
add_packages "glslang"

--- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- ---
