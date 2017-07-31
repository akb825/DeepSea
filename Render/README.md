# Render

DeepSea Render contains the interface for the rendering engine. This will be implemented for the various system graphics APIs to be used in the final application.

The central object is provided by dsRenderer. From this, dsResourceManager can be used to create resources such as graphics buffers, textures, shaders, etc.

# Shader Notes

DeepSea uses Modular Shader Language for shaders. Shaders should either be compiled with a configuration provided with DeepSea or one that is based on a DeepSea configuration. Some notes to keep in mind:

* gl_FragCoord is always in the upper-left.
* Use the `DS_ADJUST_CLIP(x)` macro with the clip position (such as that assigned to gl_Position) to ensure the clip position x is transformed to the correct space.
* Use the `DS_RG_SWZL` macro for RG-format textures. (e.g. `color.DS_RG_SWZL`) This ensures compatibility with targets that use luminance-alpha textures instead.
* Subpass inputs may always be used. Defines are provided to fall back to a standard texture lookup on targets that don't support them directly.

# CMake Helpers

CMake helpers can be found under the `Render/cmake` folder. This includes helpers for compiling shaders and converting textures as part of your build process.
