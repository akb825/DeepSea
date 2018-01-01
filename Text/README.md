# Text

DeepSea Text manages drawing of arbitrary Unicode text with standard fonts.

One or more fonts are loaded using `dsFaceGroup`. A `dsFont` is created with fonts loaded from the `dsFaceGroup`, allowing for support of international text. If a glyph isn't available in one font, it will try the next until it is found.

Once a font has been created, it can be used along with a Unicode string to create a `dsText` instance. This will do the initial calculations required to draw the text. A `dsTextLayout` object can then be created to apply styling information to compute the geometry that can be rendered. This can be done manually or with the `dsTextRenderBuffer` helper object. A `dsTextRenderBuffer` object can be used to combine multiple `dsTextLayout` objects into a single draw call.

When writing shaders, the `computeTextColor()` function in `Text.mslh` can be used to compute the final text color. Examples can be found in `Testers/TestText/Font.msl` and `Testers/TestText/FontTess.msl` for drawing standard quads and generating quads with a tesselation shader, respectively.
