# Scene Vector Draw

The DeepSea Scene Vector Draw library declares types for drawing vector images and text within a scene.

This library holds a `dsCustomResource` wrapper to hold a `dsVectorImage`, and also defines the `dsSceneText` type used with `dsCustomResource` to hold text drawn within a scene. The `dsSceneVectorNode` base type is used as a base class type for vector nodes, with `dsSceneVectorImageNode` and `dsSceneTextNode` to draw vector images and text, respectively.

To support vector images, `dsCustomResource` wrappers are also provided for `dsVectorResources`, `dsVectorShaders`, and `dsVectorMaterialSet`. When loading through file, the `dsVectorShaders` wrapper will also add a material definition that can be referenced by other types. Text nodes use normal shaders rather than vector shaders, since vector image shaders are very specialized.

The vector image and text nodes are drawn within `dsSceneVectorItemList`. A `dsSceneVectorPrepareLIst` is also used to prepare the draw items for rendering.

# Additional Conversion Types

The following JSON formats are added to extend scene conversion.

## Scene Resources

The following custom scene resource types are provided with the members that are expected:

* `"Text"`: text to draw within a scene with a `TextNode`.
	* `resources`: the name of the vector resources to get the font from.
	* `font`: the name of the font to use if not provided by the text XML element. Defaults to `"serif"`.
	* `text`: the text. This can either be a path to a .xml file or embedded XML string. The XML contents should be a single `<text>` SVG element with any number of `<tspan>` embedded elements. (see https://www.w3.org/TR/SVG2/text.html#TextElement for details) Only solid colors are allowed for stroke and fill. When a position is provided, only a relative offset for the vertical position is supported.
* `"VectorResources"`: vector resources that will be used for vector images within a scene.
	* `resources`: array of objects for the vector resources. Each element is expected to have the following members:
		* `type`: the type of the resource. The following sections document what types are supported and the members it contains:
			* `"Texture"`: texture referenced within a vector image or text icon.
				* `name`: name used to reference the texture. Defaults to path filename without extension
				* `path`: path to the input image.
				* `format`: texture format. See cuttlefish help for supported formats.
				* `channelType`: texture channel type. See cuttlefish help for supported types.
				* `srgb`: whether to perform sRGB to linear conversion when interpreting the texture. Defaults to `false`.
				* `size`: array with the target width and height. Defaults to the dimensions of the image.
				* `quality`: quality of encoding, one of `Lowest`, `Low`, `Normal`, `High`, or `Highest`. Defaults to `Normal`.
				* `container`: the texture container format, one of `pvr`, `dds`, or `ktx`. Defaults to `pvr`.
				* `embed`: whether to embed embed directly in the resources file. Defaults to `false`.
			* `"VectorImage"`: vector image that can be looked up by name.
				* `name`: name used to reference the vector image. Defaults to the path filename without extension.
				* `path`: path to the input SVG.
				* `defaultFont`: the default font to use when none is specified for a text element. Defaults to `"serif"`.
				* `targetSize`: array with the target width and height for tessellation quality. Defaults to the dimensions of the image.
				* `embed`: whether to embed embed directly in the resources file. Defaults to `false`.
				* `"TextureTextIcons"`, `"VectorTextIcons"`: icons used to replace specific codepoints with either a texture or vector image rather than the glyph from a font face.
				* `name`: name used to reference the text icons.
				* `icons`: array of array of objects for the individual icons. The outer array groups icons together for faster lookup, where all values between the minimum and maximum codepoints are considered one block of icon glyphs. Each element of the inner array is expected to have the following members:
					* `codepoint`: the integer codepoint for the Unicode value to assign the icon to. This may also be provided as a hexidecimal string.
					* `icon`: the name of the texture or vector image (depending on the parent object's type) to use for the icon.
					* `advance`: the amount to advance text after the icon, normalized to be typically in the range `[0, 1]`.
					* `bounds`: 2D array for the minimum and maximum bounds for the icon in normalized values typically in the range `[0, 1]`.
			* `"FaceGroup"`: font faces that can be used by a font.
				* `name`: name used to reference the face group.
				* `faces`: array of objects for the faces. Each element is expected to have the following members:
					* `name`: the name of the face to reference by a font. Defaults to the filename of the font file without the extension.
					* `path`: the path to the font file.
					* `embed`: whether to embed embed directly in the resources file. Defaults to `false`.
			* `"Font"`: font for displaying of text.
				* `name`: the name used to reference the font.
				* `faceGroup`: the face group that provides the faces used by the font.
				* `faces`: array of strings for the names of the faces to use within the face group.
				* `icons`: the name of text icons to use. Defaults to unset, meaning no icons are used.
				* `quality`: the quality of the font as one of `Low`, `Medium`, `High`, or `VeryHigh`.
				* `cacheSize`: the size of the cache as one of `Small` or `Large`. Defaults to `Large`.
	* `output`: the path to the output the vector resources. When omitted, the vector resources will be embedded and all `embed` members will be forced to `true`. If `resourceType` is `"Relative"`, this will be treated as relative to the scene resource file.
	* `outputRelativeDir`: the directory relative to output path. This will be removed from the path before adding the reference.
	* `resourceType`: the resource type. See the `dsFileResourceType` for values, removing the type prefix, in addition to `"Relative"` for a path relative to the scene resources file. Defaults to `"Relative"`.
	* `sharedMaterials`: the name of the vector material set for shared material data. This may be unset if vector images aren't within the resources or shared materials aren't used.
	* `vectorShaders`: the name of the vector shaders to draw vector images with. This may be unset if neither vector images nor vector icons are within the resources.
	* `textureIconShader`: the name of the shader to draw texture icons with. This may be unset if texture icons aren't within the resources.
	* `textureIconMaterial`: the name of the material to draw texture icons with. This may be unset if texture icons aren't within the resources or an empty material is sufficient.
	* `srgb`: whether the embedded materials for any vector images should be treated as sRGB and converted to linear when drawing. Defaults to `false`.
* `"VectorShaders"`: shaders to be used with vector images within a scene.
	* `modules`: array of versioned shader modules. The appropriate model based on the graphics API version being used will be chosen at runtime. Each element of the array has the following members:
		* `version`: the version of the shader as a standard config. (e.g. `"glsl-4.1"`, `"spirv-1.0"`)
		* `module`: path to the shader module or base64 encoded data prefixed with `base64:`. The module is expected to have been compiled with Modular Shader Language (MSL).
		* `output`: the path to the location to copy the shader module to. This can be omitted to embed the shader module directly. If `resourceType` is `"Relative"`, this will be treated as relative to the scene resource file.
		* `outputRelativeDir`: the directory relative to output path. This will be removed from the path before adding the reference.
		* `resourceType`: the resource type. See the `dsFileResourceType` for values, removing the type prefix, in addition to `"Relative"` for a path relative to the scene resources file. Defaults to `"Relative"`.
	* `extraElements`: list of extra material elements to add for the material description. Each element of the array has the following members:
		* `name`: the name of the element.
		* `type`: the type of the element. See `dsMaterialType` enum for values, removing the type prefix.
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
* `"VectorImage"`: vector image to draw within a scene with `VectorImageNode`.
	* `image`: path to the input SVG.
	* `defaultFont`: the default font to use when none is specified for a text element. Defaults to `"serif"`.
	* `output`: the path to the output the vector image. This can be omitted if the vector image is embedded. If `resourceType` is `"Relative"`, this will be treated as relative to the scene resource file.
	* `outputRelativeDir`: the directory relative to output path. This will be removed from the path before adding the reference.
	* `resourceType`: the resource type. See the `dsFileResourceType` for values, removing the type prefix, in addition to `"Relative"` for a path relative to the scene resources file. Defaults to `"Relative"`.
	* `targetSize`: the target size of the vector image for the tessellation quality as an array of two floats. Defaults to the original image size.
	* `sharedMaterials`: the name of the vector material set for shared material data.
	* `vectorShaders`: the name of the vector material set for shared material data.
	* `vectorResources`: list of strings for the names of the vector resources to get textures and fonts from.
	* `srgb`: bool for whether or not the embedded materials should be treated as sRGB and converted to linear when drawing. Defaults to `false`.

## Scene Nodes

The following scene node types are provided with the members that are expected:

* `"DiscardBoundsNode"`: scene node with bounds outside of which shader fragments are discarded.
	* `discardBounds`: the discard bounds as a 2x2 array of floats for the minimum and maximum points. If omitted, an empty bounds will be used to initially disable bounds discarding.
	* `children`: an array of child nodes. Each element is an object with the following elements:
		* `nodeType`: the name of the node type.
		* `data`: the data for the node.
* `"TextNode"`: scene node to draw a `Text` object.
	* `embeddedResources`: optional set of resources to embed with the node. This is a map containing the elements as expected by `SceneResourcesConvert.convertSceneResources()`.
	* `text`: the name of the text element to draw. 
	* `alignment`: the alignment of the text. May be `"Start"`, `"End"`, `"Left"`, `"Right"`, or `"Center"`. Defaults to `"Start"`.
	* `maxWidth`: the maximum width of the text. Defaults to no maximum.
	* `lineScale`: the scale to apply to the distance between each line. Defaults to 1.2.
	* `z`: the Z value used for sorting text and vector elements as a signed int.
	* `firstChar`: the first character to display. Defaults to 0.
	* `charCount`: the number of characters to display. Defaults to all characters.
	* `shader`: the name of the shader to draw with.
	* `itemLists`: array of item list names to add the node to.
* `"VectorImageNode"`: scene node to draw a `VectorImage` object.
	* `embeddedResources`: optional set of resources to embed with the node. This is an array of maps as expected by `SceneResourcesConvert.convertSceneResources()`.
	* `vectorImage`: the name of the vector image to draw.
	* `size`: the size to draw the vector image as an array of two floats. Defaults to the original image size.
	* `z`: the Z value used for sorting text and vector elements as a signed int.
	* `vectorShaders`: the name of the vector shaders to draw with.
	* `itemLists`: array of item list names to add the node to.

## Scene Item Lists

The following scene item lists are provided with the expected members:

* `"VectorDrawPrepare"`: item list to prepare `TextNode` and `VectorImageNode` nodes for drawing. (no additional members)
* `"VectorItemList"`: item list to draw `TextNode` and `VectorImageNode` nodes.
	* `instanceData`: optional list of instance data to include with the item list. Each element of the array has the following members:
		* `type`: the name of the instance data type.
		* Remaining members depend on the value of `type`.
	* `maxMaterialDescs`: maximum number of unique material descriptions for vector and text nodes.
	* `dynamicRenderStates`: dynamic render states to apply when rendering. This may be omitted if no dynamic render states are used. This is expected to contain any of the following members:
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
	* `views`: array of strings for the name of views to draw to. If omitted or empty, all views will be drawn to.

## Instance Data

The following instance data types are provided with the expected members:

* `"InstanceDiscardBounds"`: sets the discard bounds and transform from clip to discard space.
	* `variableGroupDesc`: string name for the shader variable group to use.
