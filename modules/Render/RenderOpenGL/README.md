# Render OpenGL

DeepSea Render OpenGL provides the OpenGL implementation of the renderer. This supports desktop OpenGL as well as OpenGL ES.

A `dsGLRenderer` instance should be created in order use OpenGL. The options to customize the renderer may be set in the `dsOpenGLOptions` structure. OpenGL-specific information may be queried from the `dsGLRenderer` instance, but otherwise may be accessed as a `dsRenderer` object.
