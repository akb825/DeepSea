# Render

DeepSea Render contains the interface for the rendering engine. This will be implemented for the various system graphics APIs to be used in the final application.

The central object is provided by dsRenderer. From this, dsResourceManager can be used to create resources such as graphics buffers, textures, shaders, etc.

# CMake Helpers

CMake helpers can be found under the `Render/cmake` folder. This includes helpers for compiling shaders and converting textures as part of your build process.
