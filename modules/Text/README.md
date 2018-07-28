# Text

DeepSea Text manages drawing of arbitrary Unicode text with standard fonts. Rendering is performed using the signed distance field method, allowing a wide range of scaling with the same glyph textures.

One or more fonts are loaded using `dsFaceGroup`. A `dsFont` is created with fonts loaded from the `dsFaceGroup`, allowing for support of international text. If a glyph isn't available in one font, it will try the next until it is found.

Once a font has been created, it can be used along with a Unicode string to create a `dsText` instance. This will do the initial calculations required to draw the text. A `dsTextLayout` object can then be created to apply styling information to compute the geometry that can be rendered. Rendering is typically performed with the `dsTextRenderBuffer` helper object. The vertex format is provided for the text geometry, and a callback is used to provide the vertex information for each glyph. A `dsTextRenderBuffer` object can be used to combine multiple `dsTextLayout` objects into a single draw call.

A shader must be provided and bound to perform drawing of text. When writing shaders, the `dsComputeTextDistance()` and `dsComputeTextColor()` functions in `Text.mslh` can be used to compute the final text color, while the texture containing glyph information can be queried from the `dsFont` instance. The texture containing the fonts Examples can be found in `testers/TestText/Font.msl` and `testers/TestText/FontTess.msl` for drawing standard quads and generating quads with a tesselation shader, respectively.
