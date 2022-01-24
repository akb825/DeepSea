# Geometry

DeepSea Geometry contains simple geometry classes and routines. The geometry types provided here are oriented for what is typically used for applications such as frustum culling or geometry production.

The geometry types provided are:

* Aligned boxes
* Oriented boxes
* Planes
* Frustums
* Bounding volume hierarchy
* Kd tree
* Simple polygon triangulation, with or without holes.
* Simplification of complex polygons.

Many of the same design decisions of the math library are applied to the geometry library as well. This includes using `f`, `d`, and `i` suffixes for float, double, and int variants, function-like macros that take parameters by values for operations with the same implementation across f/d/i types, and asserting for `NULL` parameters rather than setting `errno` and returning `false`. Error checking for parameters is used for some of the more complex opeations like spatial data structures and polygon operations as the overhead is much smaller compared to the functions themselves.
