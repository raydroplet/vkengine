--- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- ---

set_toolchains "gcc"
set_languages "c++20"

set_defaultmode "debug"
set_policy("build.warning", true)

add_requires("vulkansdk", "glfw", "glm", "tinyobjloader")
add_requires("glslang", { configs = { binaryonly = true } })

add_rules("mode.release", "mode.debug")
if is_mode "debug" then
  -- local sanitize = { "-fsanitize=undefined", "-fsanitize=leak" }
  -- local warnings = { "-Wall", "-Wextra", "-Weffc++", "-Wshadow" }
  -- "-Wfatal-errors"

  -- add_cxxflags(warnings, sanitize, "-pedantic-errors")
  -- add_ldflags(sanitize, "-lubsan")
end

-- Get the project root
local project_root = os.projectdir()

target "program"
  set_default(true)
  set_kind "binary"
  add_defines "GLM_ENABLE_EXPERIMENTAL"
  add_packages("vulkansdk", "glfw", "glm", "tinyobjloader")
  add_includedirs "include"
  add_files "src/**.cpp"
  on_load(function (target)
      -- Export environment variable
      os.setenv("ROOT_PATH", project_root)
  end)

-- Custom shader compilation rule
rule("shader_compile")
    set_extensions(".vert", ".frag", ".comp", ".geom", ".tesc", ".tese")
    on_buildcmd_file(function (target, batchcmds, sourcefile, opt)
        local outputdir = "build/shaders/"
        local outputfile = path.join(outputdir, path.filename(sourcefile) .. ".spv")
        
        batchcmds:mkdir(outputdir)
        batchcmds:vrunv("glslc", {"--target-env=vulkan1.0", "-o", outputfile, sourcefile})
        batchcmds:add_depfiles(sourcefile)
        batchcmds:set_depmtime(os.mtime(outputfile))
        batchcmds:set_depcache(target:targetfile() .. ".shaders")
    end)

target "shaders"
  set_kind "phony"
  add_rules("shader_compile")
  add_files("shaders/*.vert", "shaders/*.frag")
  add_packages "glslang"


--- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- ---
