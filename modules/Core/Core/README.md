# Core

DeepSea Core contains the core functionality for DeepSea. This includes:

* Asserts
* Logging
* Threads
* Streams
* Memory management
* Hooks for profiling

# Memory, object sizes, and alignment

Structures that are typically allocated and have a known size have a `ds<Struct>_sizeof()` and `ds<Struct>_fullAllocSize()` function. Structures that are generally stored by value (such as dsTimer and dsThread) do not have these functions. Additionally, structures that can change based on runtime configuration (e.g. Render implementations) also don't implement these functions.

The `ds<Struct>_sizeof()` function is the equivalent of calling `sizeof(<Struct>)`. The main difference is this will also work on opaque types, which may have a different implementation on different platforms or build environments. Some implementations provide parameters for dynamic elements in order to calculate the final size.

The `ds<Struct>_fullAllocSize()` function is used to determine the full size to allocate for a type, including any sub-allocations. This can be used to pre-allocate a buffer for the object, then use a `dsBufferAllocator` to allocate each sub-object by incrementing the buffer pointer. If the size is variable, `ds<Struct>_fullAllocSize()` will provide parameters to provide extra information to calculate the size.

Implementations of `ds<Struct>_fullAllocSize()` should use the `DS_ALIGNED_SIZE()` macro in `DeepSea/Core/Memory/Memory.h` for each required allocation to ensure proper packing based on the alignment rules. In the most simple implmenetation, `ds<Struct>_fullAllocSize()` would return `DS_ALIGNED_SIZE(sizeof(ds<Struct>))`.

When allocating an array of objects based on `ds<Struct>_fullAllocSize()`, each element should be a pointer, even if the struct definition is publically provided. This is because any sub-allocations or extra padding might cause the array access to be incorrect.

# Object creation and destruction

Structs that represent objects that are dynamically allocated have `ds<Struct>_create()` and `ds<Struct>_destroy()` functions. When possible, `ds<Struct>_fullAllocSize()` will be used to perform a single allocation for the outer object and all sub-objects. It is always safe to call `ds<Struct>_destroy()` on a `NULL` object, and in cases where destruction can fail (e.g. not valid to destory a resource), destroying a `NULL` object isn't an error.

Structs that don't need to be dynamically allocated have a `ds<Struct>_initialize()` function, and may or may not have a corresponding `ds<Struct>_shutdown()` function to destroy any internal objects.

# Error management

DeepSea will set `errno` (either directly or indirectly) when a function fails. Typical values to look out for, some of which are custom, include:

* `EINVAL`: invalid arguments.
* `ENOMEM`: failed to allocate memory.
* `EAGAIN`: run out of thread or processess resources.
* `ENOENT`: file not found.
* `EACCESS`: permission denied reading a file.
* `EIO`: IO error reading the a stream.
* `ERANGE`: math value is out of the expected range.

Custom `errno` values are:

* `EINDEX`: index out of range.
* `ESIZE`: invalid data size.
* `EFORMAT`: invalid file format.
* `ENOTFOUND`: element not found.

In order to get a string for the error enum, use the `dsErrorString()` function found in `DeepSea/Core/Error.h`.

## Why `ERRNO`?

`errno` "feels" dirty due to relying on a (thread-local) global value that may be inadvertently changed before you have a chance to inspect it. However, the alternative (explicit result codes) is very invasive and makes the interface awkward to use. (e.g. `Object* create()` vs. `ResultCode create(Object** outPtr)`)

Nearly all code only cares about whether or not an operation succeeds, and the reason (communicated by `errno`) is only for informational purposes. While it's not *desirable* to have the possibility of reporting the incorrect reason if `errno` is changed before it's reported, it isn't critical to the execution of the program. It was decided that this is an acceptable cost compared to the added API complexity and extra work required to keep track of result codes in nested function calls.

Specialized result enums are used for the rare functions where the exact reason must be communicated to ensure proper program execution.
