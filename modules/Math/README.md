# Math

DeepSea Math contains the basic types and functions for the math used within DeepSea. Some general math utilities are provided, but the main purpose is for vector and matrix math.

Many types have float and double versions, and sometimes ints. The types have a suffix of `f` for float, `d` for double, or `i` for int. (e.g. `dsVector3f`, `dsVector3d`, `dsVector3i`) In these cases, most opearations are implemented as macros that can be used with each variant similar to templated code in C++. The macros are named similarly to functions and take the parameters by value. For example, `dsVector3_add(result, a, b)` can be called with `dsVector3f`, `dsVector3d`, and `dsVector3i` parameters passed by value. To handle use cases that need functions, `dsVector3f_add()`, `dsVector3d_add()`, and `dsVector3i_add()` functions are also included. Macros cannot be used in all situations, such as for operations that require math functions that differ by type. (e.g. `sqrt()` vs. `sqrtf()`)

For performance purposes, most operations that take parameters by pointer will assert that they aren't `NULL` rather than setting `errno` and returning `false`.
