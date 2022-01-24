# Scene

DeepSea Scene provides structures and functions to manage the layout of a 3D scene. The main structure is the scene graph, which is a graph of nodes to define a transform hierarchy. The scene itself holds a scene graph and a description for how to draw it. A view ties a scene to the view and projection matrices and framebuffer to draw to.

# Scene graph

A scene graph is a hierarchy of `dsSceneNode` nodes. Typically you will have a tree of `dsSceneTransformNode` instances to create a transform hierarchy, with `dsSceneModelNode` instances as leaf nodes.

Nodes may be "subclassed" by placing making the first member of your own struct be the base class' struct. The type is determined by a pointer to a static `dsSceneNodeType` instance, which also contains a pointer to the parent type to allow operations to be done on the base class. Use the `dsSceneNode_isOfType()` function to determine whether the node is either the type specified or a subclass. See the documentation comment for `dsSceneNode_setupParentType()` for how to properly hook up the parent type and ensure that the full hierarchy is available.

Nodes are reference counted. When created, they have a reference count of 1. Call `dsSceneNode_addRef()` to increment the reference count and `dsSceneNode_freeRef()` to decrement the reference count. Once the reference count reaches 0, the node is destroyed. When set as a child of another node, added to a scene, or added to a `dsSceneResources` instance the reference count will be added automatically. Incrementing and decrementing reference counts is thread safe.

> **Note:** If you create a node and hand it off to another object (e.g. as a child of another node), make sure to call `dsSceneNode_freeRef()` once you no longer need the direct pointer to decrement the initial reference count from creation.

## Performance considerations

Traditionally scene graphs are traversed each time you draw. However, this can be very inefficient when much of the graph remains unchanged between frames, and the constant jumping around memory is very cache inefficient. This traversal also cannot be parallelized.

In order to avoid these inefficiencies, the scene graph is flattened for the internal representation. Traversal only happens for portions of the graph that have changed transforms, while other processing and drawing is performed on flat lists. See the [Advanced Scenegraph Rendering Pipeline](http://on-demand.gputechconf.com/gtc/2013/presentations/S3032-Advanced-Scenegraph-Rendering-Pipeline.pdf) presentation by NVIDIA for details on how the scene graph is flattened.

# Scenes

Scenes combine the scene graph hierarchy with a description for how to process and draw it. The following is provided to a `dsScene` instance to determine how to process and draw the scene data:

* Global data, which is updated before anything else on the main thread.
* Shared item lists to be processed before the rest of the scene. This is a 2D array, where `dsSceneItemList` instances in the same index level can be processed in parallel during multithreaded rendering. For example, all the instances under `sharedItemLists[0]` can be processed in parallel, then are synchronized before processing `sharedItemLists[1]`. All shared item lists will be processed before processing the main scene pipelines.
* Scene pipeline items. This is a combination of `dsSceneItemList` instances for processing (e.g. compute shaders) and `dsSceneRenderPass` instances for drawing. `dsSceneRenderPass` instances can have multiple subpasses, each of which can have any number of `dsSceneItemList` instances for drawing.

# Item lists

Each node may have any number of item lists referenced by name that they belong to. When a node is added to the hierarchy, it is also added to any item list it's declared to belong to, and is removed from the lists when removed from the hierarchy. The `dsSceneItemList` struct itself is abstract, with function pointers to add or remove a node, update a node when the transform changes, and commit (such as process or draw) the item list.

One of the most commonly item list types is `dsSceneModelList` for drawing models. Different models within a `dsSceneModelNode` can be assigned to a different `dsSceneModelList`to adjust draw order. Within the a `dsSceneModelList`, the individual items may optionally be sorted:

* By material to reduce state changes.
* Back to front based on the transform matrix to draw transparent objects.
* Front to back based on the transform matrix to reduce overdraw.

`dsViewCullList` is another commonly used item list type, which performs cull checks on the models based on the transformed bounding box against the view frustum. You can get the cull result by calling `dsSceneNodeItemData_findID()`, passing the node and name ID for the `dsViewCullList` instance, which will return a non-zero result if it's out of view.

## Instance data

Some item list types, such as `dsSceneModelList`, contain a list of `dsSceneInstanceData` instances. This allows data to be bound before drawing each instance.

The most common type to use is `dsSceneInstanceVariables`, which places shader variable data into a buffer and binds the offset to the shader. `dsInstanceTransformData` is a helper for creating a `dsSceneInstanceVariables` instance that contains standard transform matrices for each instance.

# Global data

`dsSceneGlobalData` is similar to `dsSceneItemList`, except it works with data global to the scene rather than individual instances. For example, `dsViewTransformData` sets standard view and projection matrices that can be shared across all instances.

# Scene draw sequence

When drawing a scene, it first processes the global data. Next, the shared items `dsSceneItemLists` instances are processed. This is an array of arrays, where each inner array of `dsSceneItemLists` may be processed in parallel, while it will synchronize for each outer array element.

After the global data and shared item lists have been processed, the render pipeline is processed. Each item can be either a render pass, which contains an array of `dsSceneItemLists` for each subpass, or a raw `dsSceneItemList` intended to be used for compute tasks.

At the very end, the global item data instances are cleaned up.

## Multithreaded rendering

Scenes may be drawn with a `dsSceneThreadManager` to draw on multiple threads:

1. Each inner array of the shared item lists may be processed in parallel.
2. Each item in the render pipeline may be processed in parallel. The results are placed on separate command buffers and replayed in the order of the pipeline.

Since command buffers are used for multithreaded rendering, you will get best results on rendering backends that natively support command buffers, such as Vulkan and Metal.

# Views

Views contain surfaces, framebuffers, and the view and projection matrices.

Surfaces may be provided directly or declared with a description to be dynamically created. A ratio may be used to resize the surface when the view is resized.

Framebuffers are declared with the name of the surfaces to use. This allows the framebuffers to be dynamically recreated with the dynamically created surfaces, even as the view resizes.

# Scene resources

The `dsSceneResources` holds onto resources that can be referenced by name. It is reference counted similar to nodes, where `dsSceneResources_addRef()` increments the reference count and `dsSceneResources_freeRef()` decrements the reference count. The items may optionally be destroyed when the `dsSceneResources` is destroyed, with the exception of scene nodes, where the reference count is incremented and decremented as necessary.

# Loading from file

All of the main types can be loaded from file using an optimized file format implemented with [flatbuffers](https://google.github.io/flatbuffers/). These can be created with the `CreateSceneResources.py`, `CreateScene.py`, and `CreateView.py` python scripts. These can be hooked into the CMake build with the `ds_create_scene_resources()`, `ds_create_scene()`, and `ds_create_view()` functions. See the TestScene GUI tester for an example.

Input for conversion, whether through the Python or CMake functions, is typically provided by JSON files. When calling the Python code directly the data can also be provided with dicts in the same format as deserializing the JSON data. See the [conversion documentation](README-conversion.md) for more information for the expected input data.

## Extending the file format

Custom types for nodes, item lists, instance data, and global data may be registered with the system when saving and loading files.

To embed the data for files, convert functions may be registered with the `DeepSeaScene.Convert.ConvertContext` class in Python. These functions take the convert context and deserialized JSON data and output the flatbuffer data. When a `deepSeaSceneExtension(convertContext)` function is present in a Python module, this can be loaded and called automatically to register the custom types. See the TestScene GUI tester for an example, where a LightData instance data list is registered.

To load these custom types, they must be registered with the `dsSceneLoadContext` with the same type name used when saving the data.
