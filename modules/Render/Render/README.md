# Render

DeepSea Render contains the interface for the rendering engine. Implementations are available for various system graphics APIs to be used in the final application.

# Note on design

The design of the Render library borrows heavily from Vulkan. This is because it has the most limitations for how the API is used, but also provides some useful features to get greater performance. There are three main places where this is obvious:

1. Usage of render passes and subpasses. Subpasses allow for key optimizations for cases like tiled renderers found often in mobile hardware, when the pixel results from a previous subpass is used in the next subpass. An example where this can be very useful is deferred lighting.
2. Limitations that only draw operations may occur withing render passes, while other operations must be done outside. This is presumably done to provide the GPU with stricter guarantees for when and how resources are used. Working around this limitation is unfortunately impossible if subpasses are to be supported, and could also introduce performance penalties if render passes are constantly ended and re-started to interleave operations.
3. Most render states, as well as other properties such as primitive types and anti-alias samples, must be known ahead of time when creating a shader and cannot be changed dynamically. While this has traditionally been hidden by high-level APIs such as OpenGL, it has always been a performance concern, since changing these states may require re-compilation of the shader. Exposing this limitation not only makes the implementation of Render simpler, but also helps guide higher level code to make more optimal decisions.

Apart from providing an abstraction over multiple system-level graphics APIs, the biggest feature the Render library provides is the shader and material model. A proper shader and material model is both the most difficult portion to define and is also implemented very differently for the separate system-level graphics APIs to provide optimal performance. Most of the basic functionality of drawing geometry and dispatching compute is fairly low-level to allow the greatest flexibility to implement any rendering optimally, with the intent of other layers on top (such as the Scene library) to provide easier to use interfaces for specific use cases.

# Overview

The central object is provided by `dsRenderer`. From this, `dsResourceManager` can be used to create resources such as graphics buffers, textures, shaders, etc.

All rendering must be done withing a `dsRenderPass`. This provides information about the framebuffer to render to. A render pass contains one or more subpasses. This is the same as Vulkan's subpass concept, where you can read the result of the current pixel from previous subpasses. This can improve rendering performance on some platforms compared to using independent render passes.

In general, drawing operations, including binding of shaders for drawing, must be **within** a render pass. All other operations, such as updating of resources and dispatching compute jobs, must be done **outside** a render pass.

# Shaders

DeepSea uses [Modular Shader Language](https://github.com/akb825/ModularShaderLanguage) for shaders. Shaders should either be compiled with a configuration provided with DeepSea or one that is based on a DeepSea configuration. Some notes to keep in mind:

* gl_FragCoord is always in the upper-left.
* Use the `DS_ADJUST_CLIP(x)` macro with the clip position (such as that assigned to gl_Position) to ensure the clip position x is transformed to the correct space. If using a direct clip position without a projection matrix, use `DS_ADJUST_DIRECT_CLIP(x)` instead.
* When support for older hardware with OpenGL 2.x, use the `DS_RG_SWZL` macro for RG-format textures. (e.g. `color.DS_RG_SWZL`) This ensures compatibility with targets that use luminance-alpha textures instead.
* Subpass inputs may always be used. Defines are provided to fall back to a standard texture lookup on targets that don't support them directly.
* When declaring subpass inputs, instead of using `layout(input_attachment_index = i) uniform subpassInput input`, declare as `inputAttachment(i) input`.
* If you wish to provide optimal performance on Apple silicon, a separate set of render passes utilizing fragment inputs. In place of using multiple subpasses, only one subpass is used while using [fragment inputs](https://github.com/akb825/ModularShaderLanguage/blob/master/doc/Language.md#fragment-inputs) in the shaders. You can check for fragment input support at runtime with the `hasFragmentInputs` member of `dsRenderer`.

Once groups of shaders have been compiled into modules using `mslc`, they may be loaded using the `dsShaderModule` object. Once you have a `dsShaderModule`, the various pipelines declared within the module can be created as individual `dsShader` instances. When creating a `dsShader`, you will need to know the type of primitives that will be drawn with as well as the number of anti-alias samples. If a shader needs to be drawn with multiple types of primitives, or surfaces with different anti-alias samples, separate instances will need to be created.

> **Note:** `DS_DEFAULT_ANTIALIAS_SAMPLES` may be used to automatically keep in sync with the default samples in `dsRenderer` without having to re-create the shader. `DS_SURFACE_ANTIALIAS_SAMPLES` can also be used for the render surface (e.g. window surface) samples, which may be different in situations such as when post-processing is used.

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

There are three types of bindings that can be used for mateirial values, determined by the `dsMaaterialBinding` enum:

* Material: values are stored directly in the `dsMaterial` instance. All scalar, vector, and matrix uniform types *must* use material binding. These types may be wrapped in a uniform block, uniform buffer, or shader variable group to use Global or Instance bindings.
* Global: values are pulled from a `dsSharedMaterialValues` instance when a shader is bound with a material, and cannot be changed until the shader is unbound and bound again.
* Instance: values are pulled from a `dsSharedMaterialValues` instance when the shader is bound, and may be updated while the shader is still bound.

Values on a `dsMaterial` instance values may be updated at any time, but will only be applied when `dsShader_bind()` is called. Only values with Instance binding bay be updated when a shader is bound, and can be updated by calling `dsShader_updateInstanceValues()`.

> **Note:** since buffer copy oeprations need to be performed outside of render passes, updates to buffers for uniform blocks and uniform buffers, as well as updating members of `dsShaderVariableGroup`, must be done outside of a render pass as well. Individual uniform values stored within a `dsMaterial` *may* be updated within a render pass, and pointers to buffers, variable groups, or textures may be updated as well.
>
> When calling `dsShader_updateInstanceValues()`, you will get the best performance if you only update offsets within buffers.

## Render states

Most render states are set by declaring the render state in the shader pipeline. These will be automatically applied when binding the shader. A subset of render states may be dynamically provided when binding a shader, and may be provided with a `dsDynamicRenderStates` instance.

## Built-in defines

The following built-in defines are available when writing shaders:

* `DS_MIN_CLIP_Z`: The minimum Z value in clip space. In most cases this is 0.0, but for OpenGL it may be -1.0 or 0.0 depending on whether the `preferHalfDepthRange` renderer option is enabled and the device supports adjusting the clip range.
* `DS_ADJUST_CLIP(v)`: Call when assigning `gl_Position` at the end of the vertex shader to adjust for the current system's clip space. It's assumed that the position was computed with the projection matrix.
* `DS_ADJUST_DIRECT_CLIP(v)`: Same as `DS_ADJUST_CLIP`, except for creating the position directly in clip space rather than using the projection matrix.
* `DS_RG_SWZL`: Swizzle for RG channel textures. This takes into account older versions of OpenGL that use lumanance alpha for two-channel textures, which requires `ra` rather than `rg`.

# CMake Helpers

CMake helpers can be found under the `modules/Render/cmake` folder. This includes helpers for compiling shaders and converting textures as part of your build process. These CMake functions are available when importing a pre-compiled package.

# Shadows

The Renderer library provides the basic functionality used for shadow mapping.

The first structure used is `dsShadowProjection`. This is initialized with the basic information such as the camera matrix, light direction and transform, and light projection when used for point or spot lights. This is used in conjunction with `dsShadowCullVolume`, which is used to both determine what objects can influence shadows and the actual extents to compute the projection matrix for. Once all objects have been compared with `dsShadowCullVolume` and the full extents are computed, the final matrix can be created from `dsShadowProjection`.

When using cascaded shadows, the `dsComputeCascadeCount()` and `dsComputeCascadeDistance()` helper functions can be used to determine where the splits are. Once the splits have been computed, the `dsShadowProjection` and `dsShadowCullVolume` can be used as normal for each split.

The following shader headers are provided for sampling the shadow map under the `DeepSea/Render/Shaders/Shadows` include directory:

* `ShadowMap.mslh`: Contains the `dsShadowMap()` lookup for both standard directional and spot light shadows.
* `CascadedShadowMap.mslh`: Contains the uniforms and function to compute cascaded shadows. The uniforms are provided here rather than passed as parameters since the generated code is very suboptimal when passing arrays. \#defines can be used to control the names for the uniform and function if multiple lookups are needed.
* `PointShadowMap.mslh`: Contains the uniforms and function to compute spot light, or omnidirectional, shadows. The uniforms are provided here rather than passed as parameters since the generated code is very suboptimal when passing arrays. \#defines can be used to control the names for the uniform and function if multiple lookups are needed.
* `ShadowPCF*x*.mslh`: Contains filter functions for 2x2, 3x3, and 4x4 shadow filtering. One of these, or another file that defines `DS_SHADOW_FILTER` to an appropriate function, should be included above one of the above shadow lookup includes to provide the filtering function used in the lookup.
