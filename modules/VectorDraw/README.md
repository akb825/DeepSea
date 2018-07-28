# VectorDraw

DeepSea VectorDraw draws vector graphics, providing a subset of the functionality from SVG with optimized GPU-accelerated rendering.

A rough overview of features supported are:

* Basic shapes (ellipses, rectangles, rectangles with rounded corners)
* Paths containing lines and Bezier curves. (quadratic and cubic)
* Stroke and fill. Fill supports complex polygons with both even-odd and non-zero fill rules.
* Linear and radial gradients. (radial gradients must have the focal point inside the circle)
* Standard texture images
* Text lain out automatically or manually within ranges. Automatic layout supports wrapping to a specified width, and can be left, right, or center justified. Text along paths is not supported.

Materials for shapes are provided with the `dsVectorMaterialSet` structure. This supports solid colors, linear gradients, and radial gradients. These values may be updated at runtime in order to change the appearance of shapes that reference them. Other shared resources are provided with `dsVectorResources`. This provides textures for image elements, as well as face groups and fonts for rendering of text.

`dsVectorImage` is the structure that contains the image data itself. It is created with the shape data to be converted into GPU-renderable geometry. Any number of `dsVectorResources` instances may be provided to provide shard resource data. Additionally, up to two `dsVectorMaterialSet` instances may be provided: one for material values specific to that one image, and another for material values shared across multiple images. Sizing information is also provided to create geometry suitable for the target dimensions.

# Loading from file

SVG cannot be loaded directly, but they can be converted to a custom binary format using the `python/ConvertSVG.py` script. Vector resources may also be converted from a Json file using the `python/CreateVectorResources.py` script. This will convert images to a texture files and bundle font files together into a package that can be loaded.

Both of these above scripts can be invoked within CMake with the scripts under `modules/VectorDraw/cmake` folder.
