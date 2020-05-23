# Render

DeepSea Render contains the interface for the rendering engine. This will be implemented for the various system graphics APIs to be used in the final application.

# Note on design

The design of the Render library borrows heavily from Vulkan. This is because it has the most limitations for how the API is used, but also provides some useful features to get greater performance. There are three main places where this is obvious:

1. Usage of render passes and subpasses. Subpasses allow for key optimizations for cases like tiled renderers found often in mobile hardware, when the pixel results from a previous subpass is used in the next subpass. An example where this can be very useful is deferred lighting.
2. Limitations that only draw operations may occur withing render passes, while other operations must be done outside. This is presumably done to provide the GPU with stricter guarantees for when and how resources are used. Working around this limitation is unfortunately impossible if subpasses are to be supported, and could also introduce performance penalties if render passes are constantly ended and re-started to interleave operations.
3. Most render states, as well as other properties such as primitive types and anti-alias samples, must be known ahead of time when creating a shader and cannot be changed dynamically. While this has traditionally been hidden by high-level APIs such as OpenGL, it has always been a performance concern, since changing these states may require re-compilation of the shader. Exposing this limitation not only makes the implementation of Render simpler, but also helps guide higher level code to make more optimal decisions.

# Overview

The central object is provided by `dsRenderer`. From this, `dsResourceManager` can be used to create resources such as graphics buffers, textures, shaders, etc.

All rendering must be done withing a `dsRenderPass`. This provides information about the framebuffer to render to. A render pass contains one or more subpasses. This is the same as Vulkan's subpass concept, where you can read the result of the current pixel from previous subpasses. This can improve rendering performance on some platforms compared to using independent render passes.

In general, drawing operations, including binding of shaders for drawing, must be **within** a render pass. All other operations, such as updating of resources and dispatching compute jobs, must be done **outside** a render pass.

# Shaders

DeepSea uses [Modular Shader Language](https://github.com/akb825/ModularShaderLanguage) for shaders. Shaders should either be compiled with a configuration provided with DeepSea or one that is based on a DeepSea configuration. Some notes to keep in mind:

* gl_FragCoord is always in the upper-left.
* Use the `DS_ADJUST_CLIP(x)` macro with the clip position (such as that assigned to gl_Position) to ensure the clip position x is transformed to the correct space.
* Use the `DS_RG_SWZL` macro for RG-format textures. (e.g. `color.DS_RG_SWZL`) This ensures compatibility with targets that use luminance-alpha textures instead.
* Subpass inputs may always be used. Defines are provided to fall back to a standard texture lookup on targets that don't support them directly.
* When declaring subpass inputs, instead of using `layout(input_attachment_index = i) uniform subpassInput input`, declare as `inputAttachment(i) input`.

Once groups of shaders have been compiled into modules using `mslc`, they may be loaded using the `dsShaderModule` object. Once you have a `dsShaderModule`, the various pipelines declared within the module can be created as individual `dsShader` instances. When creating a `dsShader`, you will need to know the type of primitives that will be drawn with as well as the number of anti-alias samples. If a shader needs to be drawn with multiple types of primitives, or surfaces with different anti-alias samples, separate instances will need to be created.

> **Note:**`DS_DEFAULT_ANTIALIAS_SAMPLES` may be used to automatically keep in sync with the default samples in `dsRenderer` without having to re-create the shader.

## Shader materials

Materials provide the uniform values to pass to the shader. When a `dsShader` is created, it must also be provided the `dsMaterialDesc` that describes the material that will be used. `dsMaterialDesc` can be thought of as a struct declaration, describing the names and types of members stored in the material. The shader instantiated for a `dsShader` must have all of its uniforms described by `dsMaterialDesc`, though `dsMaterialDesc` may contain more material values than will be used by a shader. The same `dsMaterialDesc` instance may be shared across many `dsShader` instances, even ones that use a different pipeline declaration or even a different `dsShaderModule`.

A `dsMaterialDesc` object may hold materials of any uniform type:

* Free uniiforms of any built-in type. (e.g. `float`, `vec3`, `ivec2`, including arrays)
* Textures and images.
* Uniform blocks and buffers.
* Variable groups, which allows using a uniform block when supported, falling back to individual uniforms when not.

When a variable group is used, a separate structure (`dsShaderVariableGroupDesc`) is used to describe the members of a uniform block. On systems that support uniform blocks it will use a buffer to contain the values, but can fall back to storing the values in individual uniforms for systems that don't support uniform blocks. A `dsGfxBuffer` may be used to directly set the values of a uniform block or uniform buffer, but this will fail on systems that don't support these features.

> **Note:** when binding a shader with `dsShader_bind()`, it must be provided a `dsMaterial` instance that was created with the same `dsMaterialDesc` instance to provide the material values.

## Updating material values

Material values may be updated at any time, but will only be applied when `dsShader_bind()` is called, providing both the shader and material instance. Uniform values are maintained per `dsMaterial` instance. In cases such as uniform blocks, uniform buffers, and variable groups, the same buffer or variable group instance may be set on multiple `dsMaterial` instances to share values between materials.

> **Note:** since buffer copy oeprations need to be performed outside of render passes, updates to buffers for uniform blocks and uniform buffers, as well as updating members of `dsShaderVariableGroup`, must be done outside of a render pass as well. Individual uniform values stored within a `dsMaterial` *may* be updated within a render pass, and pointers to buffers, variable groups, or textures may be updated as well.

Textures, images, uniform blocks, uniform buffers, and variable groups may be marked as volatile when creating a `dsMaterialDesc`. Volatile values are provided with the `dsVolatileMaterialValues` structure as opposed to being stored within a `dsMaterial` structure. The pointers for these types may be re-assigned and updated while a shader remains bound, calling `dsShader_updateVolatileValues()` to update the values. Volatile material values will typically be used to communicate values about the current render state as opposed to values that are tied to the material of an object. For example, the current lighting state, such as the direction and color of lights and a shadow map, would be typically passed as volatile values.

## Render states

Most render states are set by declaring the render state in the shader pipeline. These will be automatically applied when binding the shader. A subset of render states may be dynamically provided when binding a shader, and may be provided with a `dsDynamicRenderStates` instance

# CMake Helpers

CMake helpers can be found under the `modules/Render/cmake` folder. This includes helpers for compiling shaders and converting textures as part of your build process.
