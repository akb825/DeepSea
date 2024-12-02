# Scene Physics

The DeepSea Scene Physics library integrates the Physics and Scene libraries to update transforms based on the physics simulations and update kinematic objects based on manual transform changes within a scene.

# Additional Conversion Types

The following JSON formats are added to extend scene conversion.

## Scene Resources

The following custom scene resource types are provided with the members that are expected:

* `"PhysicsBox"`: physics shape for a box.
	* `halfExtents`: array of 3 floats for the half extents of the box. The full box geometry ranges from `-halfExtents` to `+halfExtents`.
	* `convexRadius`: the convex radius for collision checks. If unset or a value < 0 the physics system's default will be used.
* `"PhysicsCapsule"`: physics shape for a capsule.
	* `halfHeight`: half the height of the capsule.
	* `radius`: the radius of the capsule.
	* `axis`: the axis of the capsule. Valid values or `X`, `Y`, and `Z`.
* `"PhysicsCone"`: physics shape for a cone.
	* `height`: the height of the cone.
	* `radius`: the radius of the cone.
	* `axis`: the axis of the cone. Valid values or `X`, `Y`, and `Z`.
	* `convexRadius`: the convex radius for collision checks. If unset or a value < 0 the physics system's default will be used.
* `"PhysicsConvexHull"`: physics shape for a convex hull.
	* The following elements are used when providing data from a model:
		* `type`: the name of the geometry type, such as "obj" or "gltf". If omitted, the type is inferred from the path extension.
		* `path`: the path to the geometry.
		* `component`: the name of the component of the model.
	* The following elements are used when providing data directly:
		* `vertices`: array of floats for the raw vertex data. This must be divisible by 3, with each vertex having three values.
	* `convexRadius`: the convex radius for collision checks. If unset or a value < 0 the physics system's default will be used.
	* `cacheName`: name used for caching pre-computed data. If not set, the pre-computed data will not be cached.
* `"PhysicsMesh"`: physics shape for a triangle mesh.
	* The following elements are used when providing data from a model:
		* `modelType`: the name of the model type, such as "obj" or "gltf". If omitted, the type is inferred from the path extension.
		* `path`: the path to the geometry.
		* `component`: the name of the component of the model. The component must use a triangle list.
		* `triangleMaterialAttrib`: the attribute to gather per-triangle material values from. The attribute must have three values per vertex, corresponding to friction, restitution, and hardness, respectively. The values for for each vertex that comprises a triangle will be averaged. Most commonly the color attribute will be used, as it is the easiest to set in modeling programs. If not set, no per-trinagle materials will be used.
		* `frictionScale`: scale value to apply to the friction for the per-triangle material attributes. This can be used to allow for friction values > 1 when used with normalized attributes, such as colors. Defaults to 1.
	* The following elements are used when providing data directly:
		* vertices: array of floats for the raw vertex data. This must be divisible by 3, with each vertex having three values.
		* indices: array of ints for the indices for each triangle. Must be divisible by 3, with each triangle having three values
		* triangleMaterials: array of floats for the raw per-triangle material. This must be divisible by 3, with each triangle having three values for the friction, restitution, and hardness, respectively. Defaults to no per-triangle materials.
		* materialIndices: array of ints for which material each triangle uses.
	* cacheName: name used for caching pre-computed data. If not set, the pre-computed data will not be cached.
* `"PhysicsSphere"`: physics shape for a sphere.
	* `radius`: the radius of the sphere.
* `"PhysicsShapeRef"`: reference to a physics shape.
	* `shape`: the name of the referenced shape.

## Scene Nodes

The following scene node types are provided with the members that are expected:

* `"RigidBodyGroupNode"`: node containing multiple rigid bodies and constraints.
	* `motionType`: the motion type for rigid bodies within the group. See `dsPhysicsMotionType` for valid enum values, removing the prefix.
	* `rigidBodyTemplates`: array of string names for the rigid body templates that will be instantiated for each instance of the group node within the scene graph.
	* `constraints`: array of string names for the constraints that will be instantiated for each instance of the group node within the scene graph.
	* `itemLists`: array of item list names to add the node to.
* `"RigidBodyNode"`: node using a rigid body within a parent `RigidBodyGroupNode`.
	* `rigidBody`: the name of the rigid body within the parent group node.
	* `itemLists`: array of item list names to add the node to.
* `"UniqueRigidBodyNode"`: node using a rigid body from scene resources intended to be used only once within a scene graph.
	* `rigidBody`: the name of the rigid body within the scene resources.
	* `itemLists`: array of item list names to add the node to.

## Scene Item Lists

The following scene item lists are provided with the expected members:

* `"PhysicsList"`: item list to manage the physics scene and objects within it.
	* `maxStaticBodoes`: the maximum number of bodies that are only used for collision and not affected by physics.
	* `maxDynamicBodies`: the maximum number of bodies that are affected by physics.
	* `maxDynamicBodies`: the maximum number of groups of bodies that are connected through constraints.
	* `maxConstrainedBodyGroups`: the maximum number of shapes used by static bodies. If 0 maxStaticBodies will be used. Defaults to 0.
	* `maxDynamicShapes`: the maximum number of shapes used by dynamic bodies. If 0 maxDynamicBodies will be used. Defaults to 0.
	* `maxConstraints`: The maximum number of constraints.
	* `maxBodyCollisionPairs`: the maximum number of pairs of bodies that may collide. The implementation is only guaranteed to process this many pairs of potentially colliding bodies. If it is exceeded, further collisions may be ignored. This should be much larger than the maximum number of contact points as the collision pairs may not actually touch.
	* `maxContactPoints`: the maximum number of contact points between colliding bodies. The implementation is only guaranteed to process this many contacts between bodies. If it is exceeded, further contacts may be discarded.
	* `gravity`: The initial gravity of the scene as a 3D vector.
	* `multiThreadedModifications`: whether modifications may be made across threads. When false, the locking functions will become NOPs that only enforce that the proper locking functions are used. This can reduce overhead when locking isn't required. Defaults to false.
	* `targetStepTime`: the step time that is desired when updating the physics list. This will keep each step as close to this time as possible. Defaults to 1/60 s.
