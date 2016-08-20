# Core

DeepSea Core contains the core functionality for DeepSea. This includes:

* Asserts
* Logging
* Threads
* Streams
* Memory management
* Hooks for profiling

# Memory, object sizes, and alignment

Every structure that would typically be allocated has a corresponding `ds<Struct>_sizeof()` and `ds<Struct>_fullAllocSize()` function. Structures that are generally stored by value (such as dsTimer and dsThread) do not have these functions.

The `ds<Struct>_sizeof()` function is the equivalent of calling `sizeof(<Struct>)`. The main difference is this will also work on opaque types, which may have a different implementation on different platforms, build environments, or even runtime environments. (e.g. different implmenetations chosen at runtime) Some implementations provide parameters for dynamic elements in order to calculate the final size.

The `ds<Struct>_fullAllocSize()` function is used to determine the full size to allocate for a type, including any sub-allocations. This can be used to pre-allocate a buffer for the object, then use a `dsBufferAllocator` to allocate each sub-object by incrementing the buffer pointer. If the size is variable, `ds<Struct>_fullAllocSize()` will sometimes provide parameters to provide extra information to calculate the size, otherwise 0 will be returned.

Implementations of `ds<Struct>_fullAllocSize()` should use the `DS_ALIGNED_SIZE()` macro in `DeepSea/Core/Memory/Memory.h` for each required allocation to ensure proper packing based on the alignment rules. In the most simple implmenetation, `ds<Struct>_fullAllocSize()` would return `DS_ALIGNED_SIZE(sizeof(ds<Struct>))`.

When allocating an array of objects based on `ds<Struct>_fullAllocSize()`, each element should be a pointer, even if the struct definition is publically provided. This is because any sub-allocations or extra padding might cause the array access to be incorrect.
