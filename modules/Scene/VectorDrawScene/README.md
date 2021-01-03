# Vector Draw Scene

The DeepSea Vector Draw Scene library declares types for drawing vector images and text within a scene.

This library holds a `dsCustomResource` wrapper to hold a `dsVectorImage`, and also defines the `dsSceneText` type used with `dsCustomResource` to hold text drawn within a scene. The `dsSceneVectorNode` base type is used as a base class type for vector nodes, with `dsSceneVectorImageNode` and `dsSceneTextNode` to draw vector images and text, respectively.

To support vector images, `dsCustomResource` wrappers are also provided for `dsVectorResources`, `dsVectorShaders`, and `dsVectorMaterialSet`. When loading through file, the `dsVectorShaders` wrapper will also add a material definition that can be referenced by other types. Text nodes use normal shaders rather than vector shaders, since vector image shaders are very specialized.

The vector image and text nodes are drawn within `dsSceneVectorItemList`. A `dsSceneVectorPrepareLIst` is also used to prepare the draw items for rendering.

# Additional Conversion Types

The following JSON formats are added to extend scene conversion.

## Scene Resources

The following custom scene resource types are provided with the members that are expected:

* `"Text"`
	* `resources`: the name of the vector resources to get the font from.
	* `font`: the name of the font to use if not provided by the text XML element. Defaults to `"serif"`.
	* `text`: the text. This can either be a path to a .xml file or embedded XML string. The XML contents should be a single \<text\> SVG element with any number of \<tspan\> embedded elements. (see https://www.w3.org/TR/SVG2/text.html#TextElement for details) Only solid colors are allowed for stroke and fill. When a position is provided, only a relative offset for the vertical position is supported.
* `"VectorResources"`
	* `resources`: path to the vector resources.
	* `output`: the path to the output the vector resources. This can be omitted if vector resources are embedded.
	* `outputRelativeDir`: the directory relative to output path. This will be removed from the path before adding the reference.
	* `resourceType`: the resource type. See the `dsFileResourceType` for values, removing the type prefix. Defaults to `"Embedded"`.
* `"VectorShaders"`
	* `modules`: array of versioned shader modules. The appropriate model based on the graphics API version being used will be chosen at runtime. Each element of the array has the following members:
		* `version`: the version of the shader as a standard config. (e.g. `"glsl-4.1"`, `"spirv-1.0"`)
		* `module`: path to the shader module or base64 encoded data prefixed with `base64:`. The module is expected to have been compiled with Modular Shader Language (MSL).
		* `output`: the path to the location to copy the shader module to. This can be omitted to embed the shader module directly.
		* `outputRelativeDir`: the directory relative to output path. This will be removed from the path before adding the reference.
		* `resourceType`: the resource type. See the `dsFileResourceType` for values, removing the type prefix. Defaults to `"Embedded"`.
	* `extraElements`: list of extra meterial elements to add for the material description. Each element of the array has the following members:
		* `name`: the name of the element.
		* `type`: the type of the element. See dsMaterialType enum for values, removing the type prefix.
		* `count`: the number of array elements. If 0 or omitted, this is not an array.
		* `binding`: the binding type for the element. See the `dsMaterialBinding` enum for values, removing the type prefix. This is only used for texture, image, buffer, and shader variable group types.
		* `shaderVariableGroupDesc`: the name of the shader variable group description when the type is a shader variable group.
	* `materialDesc`: the name of the material description to register. This can be referenced by other objects, such as creating materials used for drawing vector images.
	* `fillColor`: the name of the shader for filling with a solid color. Defaults to `"dsVectorFillColor"`.
	* `fillLinearGradient`: the name of the shader for filling with a linear gradient. Defaults to `"dsVectorFillLinearGradient"`.
	* `fillRadialGradient`: the name of the shader for filling with a radial gradient. Defaults to `"dsVectorFillRadialGradient"`.
	* `line`: the name of the shader for a line with a color or gradient. Defaults to `"dsVectorLine"`.
	* `image`: the name of the shader for a texture applied as an image. Defaults to `"dsVectorImage"`.
	* `textColor`: name of the shader for standard single-color text. Defaults to `"dsVectorTextColor"`.
	* `textColorOutline`: name of the shader for standard single-color text with a single-colored outline. Defaults to `"dsVectorTextColorOutline"`.
	* `textGradient`: name of the shader for text using a gradient. Defaults to `"dsVectorTextGradient"`.
	* `textGradientOutline`: name of the shader for text with an outline using a gradient. Defaults to `"dsVectorTextGradientOutline"`.
* `"VectorImage"`
	* `image`: path to the vector image or base64 encoded data prefixed with `base64:`.
	* `output`: the path to the output the vector image. This can be omitted if the vector image is embedded.
	* `outputRelativeDir`: the directory relative to output path. This will be removed from the path before adding the reference.
	* `resourceType`: the resource type. See the `dsFileResourceType` for values, removing the type prefix. Defaults to `"Embedded"`.
	* `targetSize`: the target size of the vector image for the tessellation quality as an array of two floats. Defaults to the original image size.
	* `sharedMaterials`: the name of the vector material set for shared material data.
	* `vectorShaders`: the name of the vector material set for shared material data.
	* `vectorResources`: list of strings for the names of the vector resources to get textures and fonts from.
	* `srgb`: bool for whether or not the embedded materials should be treated as sRGB and converted to linear when drawing. Defaults to `false`.

## Scene Nodes

The following scene node types are provided with the members that are expected:

* `"TextNode"`
	* `embeddedResources`: optional set of resources to embed with the node. This is a map containing the elements as expected by `SceneResourcesConvert.convertSceneResources()`.
	* `text`: the name of the text element to draw. 
	* `alignment`: the alignment of the text. May be `"Start"`, `"End"`, `"Left"`, `"Right"`, or `"Center"`. Defaults to `"Start"`.
	* `maxWidth`: the maximum width of the text. Defaults to no maximum.
	* `lineScale`: the scale to apply to the distance between each line. Defaults to 1.2.
	* `z`: the Z value used for sorting text and vector elements as a signed int.
	* `firstChar`: the first character to display. Defaults to 0.
	* `charCount`: the number of characters to display. Defaults to all characters.
	* `shader`: the name of the shader to draw with.
	* `material`: the name of the material to draw with.
	* `fontTexture`: the name of the texture for the font.
	* `itemLists`: array of item list names to add the node to.
* `"VectorImageNode"`
	* `embeddedResources`: optional set of resources to embed with the node. This is an array of maps as expected by `SceneResourcesConvert.convertSceneResources()`.
	* `vectorImage`: the name of the vector image to draw.
	* `size`: the size to draw the vector image as an array of two floats. Defaults to the original image size.
	* `z`: the Z value used for sorting text and vector elements as a signed int.
	* `vectorShaders`: the name of the vector shaders to draw with.
	* `material`: the name of the material to draw with.
	* `itemLists`: array of item list names to add the node to.
	
## Scene Item Lists

The following scene item lists are provided with the expected members:

* `"VectorPrepareList"` (no additional members)
* `"VectorItemList"`
	* `instanceData`: optional list of instance data to include with the item list. Each element of the array has the following members:
		* `type`: the name of the instance data type.
		* Remaining members depend on the value of `type`.
	* `dynamicRenderStates`: dynamic render states to apply when rendering. This may be ommitted if no dynamic render states are used. This is expected to contain any of the following members:
		* `lineWidth`: float width for the line. Defaults to 1.
		* `depthBiasConstantFactor`: float value for the depth bias constant factor. Defaults to 0.
		* `depthBiasClamp`: float value for the depth bias clamp. Defaults to 0.
		* `depthBiasSlopeFactor`: float value for the depth bias slope factor. Defaults to 0.
		* `blendConstants`: array of 4 floats for the blend color. Defaults to \[0, 0, 0, 0\].
		* `depthBounds`: array of 2 floats for the min and max depth value. Defaults to \[0, 1\].
		* `stencilCompareMask`: int compare mask for both the front and back stencil. Defaults to 0xFFFFFFFF.
		* `frontStencilCompareMask`: int compare mask for just the front stencil.
		* `backStencilCompareMask`: int compare mask for just the back stencil.
		* `stencilWriteMask`: int write mask for both the front and back stencil. Defaults to 0.
		* `frontStencilWriteMask`: int write mask for just the front stencil.
		* `backStencilWriteMask`: int write mask for just the back stencil.
		* `stencilReference`: int reference for both the front and back stencil. Defaults to 0.
		* `frontStencilReference`: int reference for just the front stencil.
		* `backStencilReference`: int reference for just the back stencil.
