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
	* `axis`: the axis of the capsule. Valid values are `X`, `Y`, and `Z`.
* `"PhysicsCone"`: physics shape for a cone.
	* `height`: the height of the cone.
	* `radius`: the radius of the cone.
	* `axis`: the axis of the cone. Valid values are `X`, `Y`, and `Z`.
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
		* `vertices`: array of floats for the raw vertex data. This must be divisible by 3, with each vertex having three values.
		* `indices`: array of ints for the indices for each triangle. Must be divisible by 3, with each triangle having three values
		* `triangleMaterials`: array of floats for the raw per-triangle material. This must be divisible by 3, with each triangle having three values for the friction, restitution, and hardness, respectively. Defaults to no per-triangle materials.
		* `materialIndices`: array of ints for which material each triangle uses.
	* `cacheName`: name used for caching pre-computed data. If not set, the pre-computed data will not be cached.
* `"PhysicsSphere"`: physics shape for a sphere.
	* `radius`: the radius of the sphere.
* `"PhysicsShapeRef"`: reference to a physics shape.
	* `shape`: the name of the referenced shape.
* `"RigidBody"`: unique rigid body instance.
	* `group`: the name of the rigid body group, or unset if not part of a group.
	* `flags`: list of flags control the behavior of the rigid body. See the `dsRigidBodyFlags` enum for the valid values, omitting the type prefix.
	* `motionType`: the type of motion for the rigid body. See the `dsPhysicsMotionType` enum for valid values, omitting the type prefix.
	* `dofMask`: list of DOF mask values to apply. See the `dsPhysicsDOFMask` enum for valid values, omitting the type prefix.
	* `layer`: the physics layer the rigid body is a member of. See the `dsPhysicsLayer` enum for valid values, omitting the type prefix.
	* `collisionGroup`: integer ID for the collision group. Defaults to 0 if not provided.
	* `customMassProperties`: either a shifted mass or mass properties to customize the mass and inertia. If unset, the mass properties will be computed based on the shapes in the rigid body. If set, it is expected to contain the following elements based on the type of mass properties:
		* Shafted mass:
			* `rotationPointShift`: array of 3 floats for the offset to shift the rotation point. A value of all zeros will be the center of mass, while non-zero values will adjust the point of rotation when the object is in free-fall. For example, to make the rigid body top or bottom heavy.
			* `mass`: the mass for the rigid body. If unset or a value < 0, the mass will be computed by the shapes.
		* Full mass properties:
			* `centeredInertia`: 2D array of floats for the moment of inertia as a 3x3 tensor matrix, centered around the center of mass. Each inner array corresponds to a column of the matrix. The matrix should be symmetrical, such that the transpose is the same.
			* `centerOfMass`: array of 3 floats for the center of mass relative to the local space of the rigid body.
			* `mass`: the mass of the rigid body.
			* `inertiaTranslate`: array of 3 floats for the translation for the frame of reference of the inertia tensor. This will be the point around which the object will rotate when in free-fall. If unset, the center of mass will be used.
			* `inertiaRotate`: array with x, y, z Euler angles in degrees for the rotation of the frame of reference of the inertia tensor. If unset, the identity rotation will be used.
	* `position`: array of 3 floats for the position of the body in world space. If unset, the origin will be used.
	* `orientation`: array with x, y, z Euler angles for the orinetation of the body in world space. If unset, the identity rotation will be used.
	* `scale`: array of 3 floats for the scale of the body. If unset, a scale of 1 will be used.
	* `linearVelocity`: array of 3 floats for the initial linear velocity of the body. If unset, the body will have no linear motion.
	* `angularVelocity`: array of 3 floats for the initial angular velocity of the body along each axis. If unset, the body will have no linear motion.
	* `friction`: the coefficient of friction, with 0 meaning no friction and increasing values having higher friction
	* `restitution`: the restitution value, where 0 is fully inelastic and 1 is fully elastic.
	* `hardness`: the hardness value, where 0 indicates to use this body's restitution on collision and 1 indicates to use the other body's restitution.
	* `linearDamping`: linear damping factor in the range [0, 1] to reduce the velocity over time. If unset or a value < 0 the default will be used.
	* `angularDamping`: angular damping factor in the range [0, 1] to reduce the velocity over time. If unset or a value < 0 the default will be used.
	* `maxLinearVelocity`: the maximum linear velocity. If unset or a value < 0 the default will be used.
	* `maxAngularVelocity`: the maximum angular velocity. If unset or a value < 0 the default will be used.
	* `shapes`: array of objects for the shapes to use with the rigid body. Each element of the array is expected to have the following members:
		* `type`: the type of the shape. See the different shape resource types, removing the "Physics" prefix. (e.g. instead of "PhysicsSphere" use just "Sphere") The members for the corresponding shape resource type should be used as well.
		* `density`: the density of the shape for computing its mass.
		* `translate`: array of 3 floats for the translation of the shape. If unset, the shape will never have a translation applied beyond the transform of the body itself.
		* `rotate`: array x, y, z Euler angles in degrees for the rotation of the shape. If unset, the shape will never have a rotation applied beyond the transform of the body itself.
		* `scale`: array of 3 floats for the scale of the shape. If unset, the shape will never have a scale applied beyond the transform of the body itself.
		* `material`: material to apply to the shape. If unset, the material values for the body will be used. If set, it is expected to be an object with the following elements:
			* `friction`: the coefficient of friction, with 0 meaning no friction and increasing values having higher friction
			* `restitution`: the restitution value, where 0 is fully inelastic and 1 is fully elastic.
			* `hardness`: the hardness value, where 0 indicates to use this body's restitution on collision and 1 indicates to use the other body's restitution.
* `"RigidBodyTemplate"`: template to create rigid body instances.
	* `flags`: list of flags control the behavior of the rigid body. See the `dsRigidBodyFlags` enum for the valid values, omitting the type prefix.
	* `motionType`: the type of motion for the rigid body. See the `dsPhysicsMotionType` enum for valid values, omitting the type prefix.
	* `dofMask`: list of DOF mask values to apply. See the `dsPhysicsDOFMask` enum for valid values, omitting the type prefix.
	* `layer`: the physics layer the rigid body is a member of. See the `dsPhysicsLayer` enum for valid values, omitting the type prefix.
	* `collisionGroup`: integer ID for the collision group. Defaults to 0 if not provided.
	* `customMassProperties`: either a shifted mass or mass properties to customize the mass and inertia. If unset, the mass properties will be computed based on the shapes in the rigid body. If set, it is expected to contain the following elements based on the type of mass properties:
		* Shafted mass:
			* `rotationPointShift`: array of 3 floats for the offset to shift the rotation point. A value of all zeros will be the center of mass, while non-zero values will adjust the point of rotation when the object is in free-fall. For example, to make the rigid body top or bottom heavy.
			* `mass`: the mass for the rigid body. If unset or a value < 0, the mass will be computed by the shapes.
		* Full mass properties:
			* `centeredInertia`: 2D array of floats for the moment of inertia as a 3x3 tensor matrix, centered around the center of mass. Each inner array corresponds to a column of the matrix. The matrix should be symmetrical, such that the transpose is the same.
			* `centerOfMass`: array of 3 floats for the center of mass relative to the local space of the rigid body.
			* `mass`: the mass of the rigid body.
			* `inertiaTranslate`: array of 3 floats for the translation for the frame of reference of the inertia tensor. This will be the point around which the object will rotate when in free-fall. If unset, the center of mass will be used.
			* `inertiaRotate`: array with x, y, z Euler angles in degrees for the rotation of the frame of reference of the inertia tensor. If unset, the identity rotation will be used.
	* `friction`: the coefficient of friction, with 0 meaning no friction and increasing values having higher friction
	* `restitution`: the restitution value, where 0 is fully inelastic and 1 is fully elastic.
	* `hardness`: the hardness value, where 0 indicates to use this body's restitution on collision and 1 indicates to use the other body's restitution.
	* `linearDamping`: linear damping factor in the range [0, 1] to reduce the velocity over time. If unset or a value < 0 the default will be used.
	* `angularDamping`: angular damping factor in the range [0, 1] to reduce the velocity over time. If unset or a value < 0 the default will be used.
	* `maxLinearVelocity`: the maximum linear velocity. If unset or a value < 0 the default will be used.
	* `maxAngularVelocity`: the maximum angular velocity. If unset or a value < 0 the default will be used.
	* `shapes`: array of objects for the shapes to use with the rigid body. Each element of the array is expected to have the following members:
		* `type`: the type of the shape. See the different shape resource types, removing the "Physics" prefix. (e.g. instead of "PhysicsSphere" use just "Sphere") The members for the corresponding shape resource type should be used as well.
		* `density`: the density of the shape for computing its mass.
		* `translate`: array of 3 floats for the translation of the shape. If unset, the shape will never have a translation applied beyond the transform of the body itself.
		* `rotate`: array x, y, z Euler angles in degrees for the rotation of the shape. If unset, the shape will never have a rotation applied beyond the transform of the body itself.
		* `scale`: array of 3 floats for the scale of the shape. If unset, the shape will never have a scale applied beyond the transform of the body itself.
		* `material`: material to apply to the shape. If unset, the material values for the body will be used. If set, it is expected to be an object with the following elements:
			* `friction`: the coefficient of friction, with 0 meaning no friction and increasing values having higher friction
			* `restitution`: the restitution value, where 0 is fully inelastic and 1 is fully elastic.
			* `hardness`: the hardness value, where 0 indicates to use this body's restitution on collision and 1 indicates to use the other body's restitution.

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
	* `maxStaticBodies`: the maximum number of bodies that are only used for collision and not affected by physics.
	* `maxDynamicBodies`: the maximum number of bodies that are affected by physics.
	* `maxConstrainedBodyGroups`: the maximum number of groups of bodies that are connected through constraints.
	* `maxStaticShapes`: the maximum number of shapes used by static bodies. If 0 maxStaticBodies will be used. Defaults to 0.
	* `maxDynamicShapes`: the maximum number of shapes used by dynamic bodies. If 0 maxDynamicBodies will be used. Defaults to 0.
	* `maxConstraints`: The maximum number of constraints.
	* `maxBodyCollisionPairs`: the maximum number of pairs of bodies that may collide. The implementation is only guaranteed to process this many pairs of potentially colliding bodies. If it is exceeded, further collisions may be ignored. This should be much larger than the maximum number of contact points as the collision pairs may not actually touch.
	* `maxContactPoints`: the maximum number of contact points between colliding bodies. The implementation is only guaranteed to process this many contacts between bodies. If it is exceeded, further contacts may be discarded.
	* `gravity`: array of 3 floats for the the initial gravity of the scene.
	* `multiThreadedModifications`: whether modifications may be made across threads. When false, the locking functions will become NOPs that only enforce that the proper locking functions are used. This can reduce overhead when locking isn't required. Defaults to false.
	* `targetStepTime`: the step time that is desired when updating the physics list. This will keep each step as close to this time as possible. Defaults to 1/60 s.
