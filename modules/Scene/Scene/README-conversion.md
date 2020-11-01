# Scene Conversion

These are the different types supported for conversion in the scene library. The JSON layout is given for each type. This only contains types defined directly in the Scene library, additional types may be registered externally as extensions.

* [Scene Resources](#scene-resources)
	* [Model Node](#model-node)
	* [Model Node Clone](#model-node-clone)
	* [Transform Node](#transform-node)
	* [Reference Node](#reference-node)
* [Scene](#scene)
	* [Item Lists](#item-lists)
		* [Model List](#model-list)
		* [View Cull List](#view-cull-list)
	* [Instance Data](#instance-data)
		* [Instance Transform Data](#instance-transform-data)
	* [Global Data](#global-data)
		* [View Transform Data](#view-transform-data)
* [View](#view)

# Scene Resources

Scene resources provide the resources that will be drawn inside of scenes. When provided by JSON the document is an array, or a Python list when provided by code. Each element of the array is a JSON object (or Python dict), each with the following members:

* `type`: string for the resource type.
* `name`: string name to reference the resource.

The remaining members of each element depends on the value of `type`. The builtin type strings and their extra members are:

* `"Buffer"`
	* `usage`: array of usage flags. See the `dsGfxBufferUsage` enum for values, removing the type prefix. At least one must be provided.
	* `memoryHints`: array of memory hints. See the `dsGfxMemory` enum for values, removing the type prefix. At least one must be provided.
	* `size`: the size of the buffer. This is only used if no data is provided.
	* `data`: path to the buffer data or base64 encoded data prefixed with `base64:`. This may be omitted to leave the buffer data uninitialized.
	* `output`: the path to the output the buffer. This can be omitted if no input path is provided or if the buffer is embedded.
	* `outputRelativeDir`: the directory relative to output path. This will be removed from the path before adding the reference.
	* `resourceType`: the resource type. See the `dsFileResourceType` for values, removing the type prefix. Defaults to `"Embedded"`.
* `"Texture"`
	* `usage`: array of usage flags. See the `dsGfxBufferUsage` enum for values, removing the type prefix. Defaults to `["Texture"]`. If set, at least one must be provided.
	* `memoryHints`: array of memory hints. See the `dsGfxMemory` enum for values, removing the type prefix. Defaults to `["GPUOnly"]`. If set, at least one must be provided.
	* `path`: path to the texture image. This may be omitted if no initial texture data is used.
	* `pathArray`: array of paths to texture images. Use this in place of `path` for texture arrays or cubemaps.
	* `output`: the path to the output the texture. This can be omitted if no input path is provided or if the texture is embedded. When converting textures, the extension should match the desired output container format.
	* `outputRelativeDir`: the directory relative to output path. This will be removed from the path before adding the reference.
	* `resourceType`: the resource type. See the `dsFileResourceType` for values, removing the type prefix. Defaults to `"Embedded"`.
	* `textureInfo`: the info to describe the texture. This must be provided if no image path is provided or if the image from the input path is to be converted. This is expected to have the following elements:
		* `format`: the texture format. See the `dsGfxFormat` enum for values, removing the type pre The decorator values may not be used.
		* `decoration`: the decoration for the format. See the `dsGfxFormat` enum for values, removing the type prefix. Only the decorator values may be used. May also be `"Unset"` in cases where a decorator isn't valid.
		* `dimension`: the dimension of the texture. See the `dsTextureDim` enum for values, removing the type prefix and starting with "Dim". `"Dim2D"` is used by default.
		* `width`: the width of the texture in pixels. When converting, it may also be a string as documented with cuttlefish's `-r, --resize` option. This may also be omitted when converting.
		* `height`: the height of the texture in pixels. When converting, it may also be a string as documented with cuttlefish's `-r, --resize` option. This may also be omitted when converting.
		* `depth`: the depth or array layers of the texture. If 0 or omitted, this is not a texture array.
	    * (the following elements are only used for texture conversion)
		* `mipLevels`: the number of mipmap levels.
		* `quality`: the quality to use during conversion. May be one of `"lowest"`, `"low"`, `"normal"`, `"high"`, or `"highest"`. Defaults to `"normal"`.
		* `normalmap`: float value for a height to use to convert to a normalmap.
		* `swizzle`: string of `R`, `G`, `B`, `A`, or `X` values to swizzle the color channels.
		* `rotate`: angle to rotate. Must be a multile of 90 degrees in the range \[-270, 270\].
		* `alpha`: the alpha mode to use. Must be: `"none"`, `"standard"`, `"pre-multiplied"`, or `"encoded"`. Default value is `"standard"`.
		* `transforms`: array of transforms to apply. Valid values are: `"flipx"`, `"flipy"`, `"srgb"`, `"grayscale"`, and `"pre-multiply"`. Note that `"srgb "`is implied if the decorator is srgb.
* `"ShaderVariableGroupDesc"`
	* `elements`: array of elements for the shader variable group. Each element of the array has the following members:
		* `name`: the name of the element.
		* `type`: the type of the element. See `dsMaterialType `enum for values, removing the type prefix.
		* `count`: the number of array elements. If 0 or omitted, this is not an array.
* `"ShaderVariableGroup"`
	* `description`: the name of the description defined in `shaderVariableGroupDescs`. The
	    description may be in a different scene resources package.
	* `data`: array of data elements to set. Each element of the array has the following members:
		* `name`: the name of the data element.
		* `type`: the type of the element. See the `dsMaterialType` enum for values, removing the type prefix.
		* `first`: the index of the first element to set when it's an array. Defaults to 0 if not set.
		* `data`: the data to set, with the contents depending on the `type` that was set.
			* Vector types are arrays, while matrix types are arrays of column vector arrays.
			* Textures, images, and variable groups have the string name of the resource.
			* Buffers are objects with the following members:
				* `name`: the name of the buffer.
				* `offset`: integer byte offset into the buffer. Defaults to 0.
				* `size`: the integer bytes to bind within the buffer.
			* Texture buffers and image buffers are objects with the following members:
				* `name`: the name of the buffer.
				* `format`: the texture format. See the dsGfxFormat enum for values, removing the type prefix. The decorator and compressed values may not be used.
				* `decoration`: the decoration for the format. See the `dsGfxFormat` enum for values, removing the type prefix. Only the decorator values may be used. May also be `"Unset"` in cases where a decorator isn't valid.
			* `offset`: integer byte offset into the buffer. Defaults to 0.
				* `count`: integer number of texels in the buffer.
		* `dataArray`: this may be set in place of the data member to provide an array of data elements rather than a single one.
		* `srgb`: set to true to consider the data values as sRGB colors that will be converted to linear space. Defaults to false.
* `"MaterialDesc"`
	* `elements`: array of elements for the material. Each element of the array has the following members:
		* `name`: the name of the element.
		* `type`: the type of the element. See `dsMaterialType` enum for values, removing the type prefix count: the number of array elements. If 0 or omitted, this is not an array.
	* `binding`: the binding type for the element. See the `dsMaterialBinding` enum for values, removing the type prefix. This is only used for texture, image, buffer, and shader variable group types.
		* `shaderVariableGroupDesc`: the name of the shader variable group description when the type is a shader variable group. The description may be in a different scene resources package.
* `"Material"`: See `"ShaderVariableGroup"` for a description of the object members, except the
	  "description" element is for a material description rather than a shader variable group
	  description.
* `"ShaderModule"`
	* `modules`: array of versioned shader modules. The appropriate model based on the graphics API version being used will be chosen at runtime. Each element of the array has the following members:
	* `version`: the version of the shader as a standard config. (e.g. `"glsl-4.1"`, `"spirv-1.0"`)
		* `module`: path to the shader module or base64 encoded data prefixed with `base64:`. The module is expected to have been compiled with Modular Shader Language (MSL).
		* `output`: the path to the location to copy the shader module to. This can be omitted to embed the shader module directly.
		* `outputRelativeDir`: the directory relative to output path. This will be removed from the path before adding the reference.
		* `resourceType`: the resource type. See the dsFileResourceType for values, removing the type prefix. Defaults to "Embedded".
* `"Shader"`
	* `module`: the name of the shader module the shader resides in. The shader module may be in a different scene resources package.
	* `pipelineName`: the name of the shader pipeline within the shader module.
	* `materialDesc`: The name of the material description for materials that will be used with the shader. The material may be in a different scene resources package.
* `"DrawGeometry"`
	* `vertexBuffers`: array of vertex buffers. This can have up to 4 elements with the following members:
		* `name`: the name of the buffer to use. The buffer may be in a different scene resources package.
		* `offset`: the offset in bytes into the buffer to the first vertex. Defaults to 0.
		* `count`: the number of vertices in the buffer.
		* `format`: the vertex format. This is a dict with the following members:
			* `attributes`: array of attributes for the format. Each element has the following members:
				* `attrib`: the attribute. This can either be an enum value from `dsVertexAttrib`, removing the type prefix, or the integer for the attribute.
				* `format`: the attribute format. See the dsGfxFormat enum for values, removing the type prefix. Only the "standard" formats may be used.
				* `decoration`: the decoration for the format. See the `dsGfxFormat` enum for values, removing the type prefix. Only the decorator values may be used.
			* `instanced`: true if the vertex data is instanced. Defaults to false.
	* `indexBuffer`: the index buffer. If not set, the draw geometry isn't indexed. This is a dict with the following members:
		* `name`: the name of the buffer to use. The buffer may be in a different scene resources package.
		* `offset`: the offset in bytes into the buffer to the first index. Defaults to 0.
		* `count`: the number of indices in the buffer.
		* `indexSize`: the size of the index in bytes. This must be either 2 or 4.
* `"SceneNode"`
	* `nodeType`: the name of the node type.
	* Remaining members depend on the value of `nodeType`.

The different types for scene nodes are documented below.

## Model Node

Model nodes have the type string "ModelNode" and contains the following data members:

* `embeddedResources`: optional set of resources to embed with the node. This is a map containing the elements as expected by [Scene Resources](#scene-resources).
* `modelGeometry`: array of model geometry. Each element of the array has the following members:
	* `type`: the name of the geometry type, such as "obj" or "gltf". If omitted, the type is inferred from the path extension.
	* `path`: the path to the geometry.
	* `vertexFormat`: array of vertex attributes defining the vertex format. Each element of the array has the following members:
		* `attrib`: the attribute. This can either be an enum value from `dsVertexAttrib`, removing the type prefix, or the integer for the attribute.
		* `format`: the attribute format. See the `dsGfxFormat` enum for values, removing the type prefix. Only the "standard" formats may be used.
		* `decoration`: the decoration for the format. See the `dsGfxFormat` enum for values,
	      removing the type prefix. Only the decorator values may be used.
	* `indexSize`: the size of the index in bytes. This must be either 2 or 4. If not set, no indices will be produced.
	* `transforms`: optional array of transforms to perform on the vertex values. Each element of the array has the following members:
		* `attrib`: the attribute, matching one of the attributes in vertexFormat.
		* `transform`: transform to apply on the attribute. Valid values are:
			* `Identity`: leaves the values un-transformed.
			* `Bounds`: normalizes the values based on the original value's bounds
			* `UNormToSNorm`: converts `UNorm` values to `SNorm` values.
			* `SNormToUNorm`: converts `SNorm` values to `UNorm` values.
		* `drawInfos`: array of definitions for drawing components of the geometry. Each element of the array has the following members:
			* `name`: the name of the model component. Note that only model components referenced in the drawInfo array will be included in the final model.
			* `shader`: te name of the shader to draw with.
			* `material`: the name of the material to draw with.
			* `distanceRange`: array of two floats for the minimum and maximum distance to draw at. Defaults to `[0, 3.402823466e38]`.
			* `listName`: the name of the item list to draw the model with.
* `models`: array of models to draw with manually provided geometry. (i.e. not converted from the `modelGeometry` array) Each element of the array has the following members:
	* `name`: optional name for the model for use with material remapping.
	* `shader`: the name of the shader to draw with.
	* `material`: the name of the material to draw with.
	* `geometry`: the name of the geometry to draw.
	* `distanceRange`: array of two floats for the minimum and maximum distance to draw at. Defaults to `[0, 3.402823466e38]`.
	* `drawRange`: the range of the geometry to draw. This is an object with the following members, depending on if the geometry is indexed or not:
	    * Indexed geometry:
			* `indexCount`: the number of indices to draw.
			* `instanceCount`: the number of instances to draw. Defaults to 1.
			* `firstIndex`: the first index to draw. Defaults to 0.
			* `vertexOffset`: the offset to apply to each index value. Defaults to 0.
			* `firstInstance`: the first instance to draw. Defaults to 0.
		* Non-indexed geometry:
			* `vertexCount`: the number of vertices to draw.
			* `instanceCount`: the number of instances to draw. Defaults to 1.
			* `firstVertex`: the first vertex to draw. Defaults to 0.
			* `firstIstance`: the first instance to draw. Defaults to 0.
		* `primitiveType`: the primitive type to draw with. See the dsPrimitiveType enum for values, removing the type prefix. Defaults to "TriangleList".
		* `listName`: The name of the item list to draw the model with.
* `extraItemLists`: array of extra item list names to add the node to.
* `bounds`: 2x3 array of float values for the minimum and maximum values for the positions. This will be automatically calculated from geometry in `modelGeometry` if unset. Otherwise if unset the model will have no explicit bounds for culling.

## Model Node Clone

Model node clones have the type name "ModelNodeClone" and clone an existing model node, optionally remapping the materials, containing the following members:

* `name`: the name of the model node to clone.
* `materialRemaps`: optional array of material remaps to apply. Each element of the array has the following members:
	* `name`: the name of the model inside the node to replace the material with.
	* `shader`: the name of the shader to use. If unset, the shader will remain unchanged.
	* `material`: the name of the material to use. If unset, the material will remain unchanged.

## Transform Node

Transform nodes have the type string "TransformNode" and contains the following members:

* `transformList`: array of transforms to apply. The matrices for the transforms provided are multiplied in reverse order given (i.e. in logical transform order), starting from the identity matrix. Each member of the array has the following elements:
	* `type`: the type of transform. May be `Rotate`, `Scale`, `Translate`, or `Matrix`.
	* `value`: the value of transform based on the type:
		* `Rotate`: array of 3 floats for the X, Y, and Z rotation in degrees.
		* `Scale`: array of 3 floats for the scale value along the X, Y, and Z axes.
		* `Translate`: array of 3 floats for the translation along X, Y, and Z.
		* `Matrix`: a 4x4 array of floats for a matrix. Each inner array is a column of the matrix.
* `children`: an array of child nodes. Each element is an object with the following elements:
	* `type`: the name of the node type.
	* Remaining members depend on the value of `nodeType`.
	
## Reference Node

Reference nodes have the type name "ReferenceNode" and contains the following members:

* `ref`: string name of the node that's referenced.

# Scene

The scene describes the how to process and draw a scene, as well as the list of nodes to initially populate the scene with. The JSON object (or Python dict supplied directly) has the following elements:

* `sharedItems`: an optional array of shared item lists. Each element of the array is itself an array with the following members:
	* `type`: the name of the item list type.
	* `name`: the name of the item list.
	* Remaining members depend on the value of `type`.
* `pipeline`: array of stages to define the pipeline to process when drawing the scene. Each element of the array has one of the the following set members.
	* For an item list:
		* `type`: the name of the item list type.
		* `name`: the name of the item list.
		* Remaining members depend on the value of `type`.
	* For a render pass:
		* `framebuffer`: the name of the framebuffer to use when rendering.
		* `attachments`: array of attachments to use during the render pass. Each element of the array has the following members:
		* `usage`: array of usage flags. See the dsAttachmentUsage enum for values, removing the type prefix. Defaults to `["Standard"]`.
		* `format`: the attachment format. See the `dsGfxFormat` enum for values, removing the type prefix. The decorator values may not be used. May also be `SurfaceColor` or `SurfaceDepthStencil` to use the color or depth/stencil format for render surfaces.
		* `decoration`: the decoration for the format. See the `dsGfxFormat` enum for values, removing the type prefix. Only the decorator values may be used. May also be `Unset` in cases where a decorator isn't valid.
		* `samples`: the number of anti-alias samples. When omitted, this uses the number of samples set on the renderer for window surfaces.
		* `clearValue`: a dict with one of the following members:
			* `floatValues`: array of 4 float values.
			* `intValues`: array of 4 signed int values.
			* `uintValues`: array of 4 unsigned int values.
			* `depth`: float depth value. This may be paired with "stencil".
			* `stencil`: unsigned int stencil value. This may be paired with "depth".
		* `subpasses`: array of subpasses in the render pass. Each element of the array has the following members:
			* `name`: the name of the subpass.
			* `inputAttachments`: array of indices into the attachment arrays for the attachments to use as subpass inputs.
			* `colorAttachments`: array of attachments to write to for color. Each element of the array has the following members:
				* `index`: the index into the attachment array.
				* `resolve`: whether or not to resolve multisampled results.
			* `depthStencilAttachment`: if set, the attachment to write to for depth/stencil. Each element has the following members:
				* `index`: the index into the attachment array.
				* `resolve`: whether or not to resolve multisampled results.
		* `drawLists`: array of item lists to draw within the subpass. Each element of the array has the following members:
			* `type`: the name of the item list type.
			* `name`: the name of the item list.
			* Remaining members depend on the value of `type`.
		* `dependencies`: optionall array of dependencies between subpasses. If omitted, default dependencies will be used, which should be sufficient for all but very specialized use cases. Each element of the array has the following members:
			* `srcSubpass`: the index to the source subpass. If omitted, the dependency will be before the render pass.
			* `srcStages`: array of stages for the source dependency. See the `dsGfxPipelineStage` enum for values, removing the type prefix.
			* `srcAccess`: array of access types for the source dependency. See the `dsGfxAccess` enum for values, removing the type prefix.
			* `dstSubpass`: the index to the destination subpass. If omitted, the dependency will be after the render pass.
			* `dstStages`: array of stages for the destination dependency. See the `dsGfxPipelineStage` enum for values, removing the type prefix.
			* `dstAccess`: array of access types for the destination dependency. See the `dsGfxAccess` enum for values, removing the type prefix.
			* `regionDependency`: `true` to have the dependency can be applied separately for different regions of the attachment, `false` if it must be applied for the entire subpass.
* `globalData`: optional array of items to store data globally in the scene. Each element of the
	  array has the following members:
	* `type`: the name of the global data type.
	* Remaining members depend on the value of `type`.
* `nodes`: array of string node names to set on the scene.
	
## Item Lists

Builtin item list specifications are documented below.

### Model List

Model lists have the type name "ModelList" and define how to draw models that reference it by name. It contains the following members:

* `instanceData`: optional list of instance data to include with the model list. Each element of the array has the following members:
	* `type`: the name of the instance data type.
	* Remaining members depend on the value of `type`.
* `sortType`: the method to sort the models. See the `dsModelSortType` enum for values, removing the type prefix. Defaults to `None`.
* `dynamicRenderStates`: dynamic render states to apply when rendering. This may be omitted if no dynamic render states are used. This is expected to contain any of the following members:
	* `lineWidth`: float width for the line. Defaults to 1.
	* `depthBiasConstantFactor`: float value for the depth bias constant factor. Defaults to 0.
	* `depthBiasClamp`: float value for the depth bias clamp. Defaults to 0.
	* `depthBiasSlopeFactor`: float value for the depth bias slope factor. Defaults to 0.
	* `blendConstants`: array of 4 floats for the blend color. Defaults to `[0, 0, 0, 0]`.
	* `depthBounds`: array of 2 floats for the min and max depth value. Defaults to `[0, 1]`.
	* `stencilCompareMask`: int compare mask for both the front and back stencil. Defaults to `0xFFFFFFFF`.
	* `frontStencilCompareMask`: int compare mask for just the front stencil.
	* `backStencilCompareMask`: int compare mask for just the back stencil.
	* `stencilWriteMask`: int write mask for both the front and back stencil. Defaults to 0.
	* `frontStencilWriteMask`: int write mask for just the front stencil.
	* `backStencilWriteMask`: int write mask for just the back stencil.
	* `stencilReference`: int reference for both the front and back stencil. Defaults to 0.
	* `frontStencilReference`: int reference for just the front stencil.
	* `backStencilReference`: int reference for just the back stencil.
* `cullName`: optional name for the item list to handle culling.

## Instance Data

Instance data is typically included in specific item lists that utilize them. Builtin instance data specifications are documented below.

### Instance Transform Data

Instance transform data has the type name "InstanceTransformData" and sets standard transform matrices for each item that's drawn. It contains the following members:

* `variableGroupDescName`: string name for the shader variable group to use.

### View Cull List

View cull list has the type name "ViewCullList" and performs cull checks for model bounding boxes against the view frustum. The data is ignored and may be omitted.

## Global Data

Builtin global data specifications are documented below.

### View Transform Data

View transform data has the type name "ViewTransformData" and sets standard view and projection transform matrices. It contains the following members:

* `variableGroupDescName`: string name for the shader variable group to use.

# View

The view describes the layout of surfaces and framebuffers to draw to. The JSON object (or Python dict supplied directly) has the following elements:

* `surfaces`: array of surfaces to use within the view. Each element of the array contains the following members:
	* `name`: the name of the surface.
	* `type`: the type of the surface. May either be `Renderbuffer` or `Offscreen`.
	* `usage`: array of usage flags. See the `dsRenderbufferUsage` enum (for renderbuffers) and 
	    `dsTextureUsage` enum (for offscreens) for values, removing the type prefix. Defaults to
	    `["Standard"]` for renderbuffers or `["Texture"]` for offscreens.
	* `memoryHints`: array of memory hints. See the `dsGfxMemory` enum for values, removing the type prefix. Defaults to `["GPUOnly"]`.
	* `dimension`: dimension for an offscreen. See the `dsTextureDim` enum for values, removing the type prefix. Defaults to `Dim2D`.
	* `format`: the texture format. See the `dsGfxFormat` enum for values, removing the type prefix. The decorator values may not be used. May also be `SurfaceColor` or `SurfaceDepthStencil` to use the color or depth/stencil format for render surfaces.
	* `decoration`: the decoration for the format. See the `dsGfxFormat` enum for values, removing the type prefix. Only the decorator values may be used. May also be "Unset" in cases where a decorator isn't valid.
	* `width`: the explicit width of the surface.
	* `widthRatio`: the ratio of the width relative to the view width. `width` should be omitted when this is used. Defaults to 1.0 if neither `width` nor `widthRatio` is set.
	* `height`: the explicit height of the surface.
	* `heightRatio`: the ratio of the height relative to the view height. `height` should be omitted when this is used. Defaults to 1.0 if neither `height` nor `heightRatio` is set.
	* `depth`: the depth or array layers of the surface if an offscreen. If 0 or omitted, this is not a texture array.
	* `mipLevels`: the number of mipmap levels for an offscreen. Defaults to 1.
	* `samples`: the number of anti-alias samples. When omitted, this uses the number of samples set on the renderer for window surfaces.
	* `resolve`: whether or not to resolve multisampled results.
	* `windowFramebuffer`: Whether or not the surface is used in the same framebuffer as the window surface. When `true`, the surface will follow the rotation of the view and window surface. Defaults to `true`.
* `framebuffers`: array of framebuffers to use in the view. Each element of the array has the following members:
	* `name`: the name of the framebuffer.
	* `surfaces`: array of surfaces within the framebuffer. Each element of the array has the following members:
		* `name`: the name of the surface to use. This should either be present in the surfaces array or provided to the View when loaded.
		* `face`: cube face for when the surface is a cubemap. See the `dsCubeFace` enum for values, removing the type prefix. Defaults to `PosX`.
		* `layer`: the texture array or 3D texture level to use for an offscreen. Defaults to 0.
		* `mipLevel`: the mip level to use for an offscreen. Defaults to 0.
	* `width`: the explicit width of the framebuffer.
	* `widthRatio`: the ratio of the width relative to the view width. `width` should be omitted when this is used. Defaults to 1.0 if neither `width` nor `widthRatio` is set.
	* `height`: the explicit height of the framebuffer.
	* `heightRatio`: the ratio of the height relative to the view height. `height` should be omitted when this is used. Defaults to 1.0 if neither `height` nor `heightRatio` is set.
	* `layers`: the number of layers in the framebuffer. Defaults to 1.
	* `viewport`: the viewport to use. This is a dict with the following elements.
		* `minX`: the minimum X value for the upper-left position as a fraction of the width. Defaults  to 0.
		* `minY`: the minimum Y value for the upper-left position as a fraction of the height. Defaults to 0.
		* `maxX`: the maximum X value for the lower-right position as a fraction of the width. Defaults to 1.
		* `maxY`: the maximum Y value for the lower-right position as a fraction of the hieght. Defaults to 1.
		* `minDepth`: the minimum depth value. Defaults to 0.
		* `maxDepth`: the maximum depth value. Defaults to 1.
