# Scene Physics

The DeepSea Scene Physics library integrates the Physics and Scene libraries to update transforms based on the physics simulations and update kinematic objects based on manual transform changes within a scene.

# Additional Conversion Types

The following JSON formats are added to extend scene conversion.

## Scene Resources

The following custom scene resource types are provided with the members that are expected:

### Shape Resources

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

### Constraint Resources

* `"FixedConstraint"`: constraint that has zero degrees of freedom.
	* `firstActor`: the name of the first actor used in the constraint. This may be unset if the actor will be provided later.
	* `firstPosition`: array of 3 floats for the position of the constraint relative to the first actor.
	* `firstOrientation`: array of x, y, z Euler angles in degrees for the orientation of the constraint relative to the first actor.
	* `secondActor`: the name of the second actor used in the constraint. This may be unset if the actor will be provided later.
	* `secondPosition`: array of 3 floats for the position of the constraint relative to the second actor.
	* `secondOrientation`: array of x, y, z Euler angles in degrees for the orientation of the constraint relative to the second actor.
* `"PointConstraint"`: constraint that has free rotation around a point.
	* `firstActor`: the name of the first actor used in the constraint. This may be unset if the actor will be provided later.
	* `firstPosition`: array of 3 floats for the position of the constraint relative to the first actor.
	* `secondActor`: the name of the second actor used in the constraint. This may be unset if the actor will be provided later.
	* `secondPosition`: array of 3 floats for the position of the constraint relative to the second actor.
* `"ConeConstraint"`: constraint that has limited rotation around a point.
	* `firstActor`: the name of the first actor used in the constraint. This may be unset if the actor will be provided later.
	* `firstPosition`: array of 3 floats for the position of the constraint relative to the first actor.
	* `firstOrientation`: array of x, y, z Euler angles in degrees for the orientation of the constraint relative to the first actor.
	* `secondActor`: the name of the second actor used in the constraint. This may be unset if the actor will be provided later.
	* `secondPosition`: array of 3 floats for the position of the constraint relative to the second actor.
	* `secondOrientation`: array of x, y, z Euler angles in degrees for the orientation of the constraint relative to the second actor.
	* `maxAngle`: the maximum angle in degrees of the constraint relative to the attachment orientation axes.
* `"SwingTwistConstraint"`: constraint that has limited rotation around a point.
	* `firstActor`: the name of the first actor used in the constraint. This may be unset if the actor will be provided later.
	* `firstPosition`: array of 3 floats for the position of the constraint relative to the first actor.
	* `firstOrientation`: array of x, y, z Euler angles in degrees for the orientation of the constraint relative to the first actor.
	* `secondActor`: the name of the second actor used in the constraint. This may be unset if the actor will be provided later.
	* `secondPosition`: array of 3 floats for the position of the constraint relative to the second actor.
	* `secondOrientation`: array of x, y, z Euler angles in degrees for the orientation of the constraint relative to the second actor.
	* `maxSwingXAngle`: the maximum angle in degrees of the constraint along the X axis.
	* `maxSwingYAngle`: the maximum angle in degrees of the constraint along the Y axis.
	* `maxTwistZAngle`: the maximum angle in degrees of the constraint along the Z axis.
	* `motorType`: the type of the motor to apply to the constraint. See the `dsPhysicsConstraintMotorType` enum for valid values, omitting the type prefix. `Velocity` is not supported. Defaults to `Disabled`.
	* `motorTargetOrientation`: array of x, y, z Euler angles in degrees for the target orientation of the motor relative to the second actor. Defaults to the identity rotation.
	* `maxMotorTorque`: the maximum torque of the motor to reach the target orientation. If the motor is disabled, this will be the toque used to apply to stop motion. Defaults to 0.
* `"RevolutePhysicsConstraint"`: constraint that can rotate around an arbitrary axis.
	* `firstActor`: the name of the first actor used in the constraint. This may be unset if the actor will be provided later.
	* `firstPosition`: array of 3 floats for the position of the constraint relative to the first actor.
	* `firstOrientation`: array of x, y, z Euler angles in degrees for the orientation of the constraint relative to the first actor.
	* `secondActor`: the name of the second actor used in the constraint. This may be unset if the actor will be provided later.
	* `secondPosition`: array of 3 floats for the position of the constraint relative to the second actor.
	* `secondOrientation`: array of x, y, z Euler angles in degrees for the orientation of the constraint relative to the second actor.
	* `limitEnabled`: whether the limit is enabled. Defaults to false.
	* `minAngle`: the minimum angle in degrees in the range [-180, 0] when the limit is enabled. Defaults to -180 degrees.
	* `maxAngle`: the maximum angle in degrees in the range [0, 180] when the limit is enabled. Defaults to 180 degrees.
	* `limitStiffness`: the spring stiffness applied when limiting the angle. Defaults to 100.
	* `limitDamping`: the spring damping in the range [0, 1] applied when limiting the angle. Defaults to 1.
	* `motorType`: the type of the motor to apply to the constraint. See the `dsPhysicsConstraintMotorType` enum for valid values, omitting the type prefix. Defaults to `Disabled`.
	* `motorTarget`: the target for the motor. This will be an angle in degrees if `motorType` is `Position` or an angular velocity (typically `degrees/second`) if `motorType` is `Velocity`. Defaults to 0.
	* `maxMotorTorque`: the maximum torque of the motor to reach the target. If the motor is disabled, this will be the toque used to apply to stop motion. Defaults to 0.
* `"DistancePhysicsConstraint"`: constraint that limits the distance between two points.
	* `firstActor`: the name of the first actor used in the constraint. This may be unset if the actor will be provided later.
	* `firstPosition`: array of 3 floats for the position of the constraint relative to the first actor.
	* `secondActor`: the name of the second actor used in the constraint. This may be unset if the actor will be provided later.
	* `secondPosition`: array of 3 floats for the position of the constraint relative to the second actor.
	* `minDistance`: the minimum distance between reference points.
	* `maxDistance`: the maximum distance between reference points.
	* `limitStiffness`: the stiffness for the spring to keep within the distance range.
	* `limitDamping`: the damping in the range [0, 1] to keep within the distance range.
* `"SliderPhysicsConstraint"`: constraint that limits movement along a single axis.
	* `firstActor`: the name of the first actor used in the constraint. This may be unset if the actor will be provided later.
	* `firstPosition`: array of 3 floats for the position of the constraint relative to the first actor.
	* `firstOrientation`: array of x, y, z Euler angles in degrees for the orientation of the constraint relative to the first actor.
	* `secondActor`: the name of the second actor used in the constraint. This may be unset if the actor will be provided later.
	* `secondPosition`: array of 3 floats for the position of the constraint relative to the second actor.
	* `secondOrientation`: array of x, y, z Euler angles in degrees for the orientation of the constraint relative to the second actor.
	* `limitEnabled`: whether the limit is enabled. Defaults to false.
	* `minDistance`: the minimum distance when the limit is enabled.  Defaults to 0.
	* `maxDistance`: the maximum distance when the limit is enabled. Defaults to 100.
	* `limitStiffness`: the spring stiffness applied when limiting the angle. Defaults to 100.
	* `limitDamping`: the spring damping in the range [0, 1] applied when limiting the angle. Defaults to 1.
	* `motorType`: the type of the motor to apply to the constraint. See the `dsPhysicsConstraintMotorType` enum for valid values, omitting the type prefix. Defaults to `Disabled`.
	* `motorTarget`: the target for the motor. This will be a distance if `motorType` is `Position` or an velocity if `motorType` is `Velocity`. Defaults to 0.
	* `maxMotorForce`: the maximum force of the motor to reach the target. If the motor is disabled, this will be the force used to apply to stop motion. Defaults to 0.
* `"GenericPhysicsConstraint"`: constraint that allows for control along all 6 degrees of freedom.
	* `firstActor`: the name of the first actor used in the constraint. This may be unset if the actor will be provided later.
	* `firstPosition`: array of 3 floats for the position of the constraint relative to the first actor.
	* `firstOrientation`: array of x, y, z Euler angles in degrees for the orientation of the constraint relative to the first actor.
	* `secondActor`: the name of the second actor used in the constraint. This may be unset if the actor will be provided later.
	* `secondPosition`: array of 3 floats for the position of the constraint relative to the second actor.
	* `secondOrientation`: array of x, y, z Euler angles in degrees for the orientation of the constraint relative to the second actor.
	* `limits`: the limits for the degrees of freedom. Missing DOfs will have their limits implicitly set to Free. Each array element is expected to have the following members:
		* `dof`: the degree of freedom. See the `dsPhysicsConstraintDOF` enum for valid values, omitting the type prefix.
		* `limitType`: the type of limit. See the `dsPhysicsConstraintLimitType` enum for valid values, omitting the type prefix.
		* `minValue`: the minimum value. For the rotation DOFs the value will be in degrees.
		* `maxValue`: the maximum value. For the rotation DOFs the value will be in degrees.
		* `stiffness`: the stiffness of the spring to limit the value.
		* `damping`: the damping in the range of [0, 1] for the spring to limit the value.
	* `motors`: the motors for the degrees of freedom. Missing DOFs will have their motors implicitly disabled. Each array element is expected to have the following members:
		* `dof`: the degree of freedom. See the `dsPhysicsConstraintDOF` enum for valid values, omitting the type prefix.
		* `motorType`: the type of the motor to apply. See the `dsPhysicsConstraintMotorType` enum for valid values, omitting the type prefix. Defaults to `Disabled`.
		* `target`: the target of the motor, as either a position or velocity. Rotation DOFs have the target in degrees or `degrees/second`.
		* `maxForce`: the maximum force or torque of the motor. If the motor is disabled this is the maximum amount of force to apply to stop motion.
	* `combineSwingTwistMotors`: whether the swing and twist motors are combined. Defaults to false.
* `"GearPhysicsConstraint"`: constraint that locks the rotation of two actors together based on a gear ratio. It is expected that each actor has a revolute constraint used with it.
	*  `firstActor`: the name of the first actor used in the constraint. This may be unset if the actor will be provided later.
	* `firstAxis`: array of 3 floats for the axis of rotation of the first actor.
	* `firstConstraint`: the name of the revolute constraint for the first actor. This may be unset if the constraint won't be set or will be provided later.
	* `firstToothCount`: the number of teeth for the first actor's gear to compute the gear ratio. This may be negative if it is flipped.
	* `secondActor`: the name of the second actor used in the constraint. This may be unset if the actor will be provided later.
	* `secondAxis`: array of 3 floats for the axis of rotation of the second actor.
	* `secondConstraint`: the name of the revolute constraint for the second actor. This may be unset if the constraint won't be set or will be provided later.
	* `secondToothCount`: the number of teeth for the second actor's gear to compute the gear ratio. This may be negative if it is flipped.
* `"RackAndPinionPhysicsConstraint"`: constraint that locks the translation of one actor and rotation of another actor together based on a gear ratio. It is expected that each actor has a revolute constraint used with it.
	*  `rackActor`: the name of the rack actor used in the constraint. This may be unset if the actor will be provided later.
	* `rackAxis`: array of 3 floats for the axis of translation of the rack actor.
	* `rackConstraint`: the name of the slider constraint for the rack actor. This may be unset if the constraint won't be set or will be provided later.
	* `rackToothCount`: the number of teeth for the rack to compute the gear ratio. This may be negative if it is flipped.
	* `rackLength`: the length of the rack to compute the gear ratio.
	* `pinionActor`: the name of the pinion actor used in the constraint. This may be unset if the actor will be provided later.
	* `pinionAxis`: array of 3 floats for the axis of rotation of the pinion actor.
	* `pinionConstraint`: the name of the revolute constraint for the pinion actor. This may be unset if the constraint won't be set or will be provided later.
	* `pinionToothCount`: the number of teeth for the pinion actor to compute the gear ratio. This may be negative if it is flipped.

### Other Resources

* `"RigidBody"`: unique rigid body instance.
	* `group`: the name of the rigid body group, or unset if not part of a group.
	* `flags`: list of flags control the behavior of the rigid body. See the `dsRigidBodyFlags` enum for the valid values, omitting the type prefix.
	* `motionType`: the type of motion for the rigid body. See the `dsPhysicsMotionType` enum for valid values, omitting the type prefix.
	* `dofMask`: list of DOF mask values to apply. See the `dsPhysicsDOFMask` enum for valid values, omitting the type prefix. Defaults to `["All"]`.
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
	* `dofMask`: list of DOF mask values to apply. See the `dsPhysicsDOFMask` enum for valid values, omitting the type prefix. Defaults to `["All"]`.
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
	* `motionType`: the motion type for rigid bodies within the group. See `dsPhysicsMotionType` for valid enum values, omitting the type prefix.
	* `rigidBodyTemplates`: array of string names for the rigid body templates that will be instantiated for each instance of the group node within the scene graph.
	* `constraints`: array of string names for the constraints that will be instantiated for each instance of the group node within the scene graph.
	* `children`: an array of child nodes. Each element is an object with the following elements:
		* `nodeType`: the name of the node type.
		* `data`: the data for the node.
	* `itemLists`: array of item list names to add the node to.
* `"RigidBodyNode"`: node using a rigid body within a parent `RigidBodyGroupNode`.
	* `rigidBody`: the name of the rigid body within the parent group node.
	* `itemLists`: array of item list names to add the node to.
* `"RigidBodyTemplateNode"`: node using a rigid body template from scene resources to instantiate the template without being a part of a `dsRigidBodyGroupNode`.
	* `rigidBody`: the name of the rigid body template  within the scene resources.
	* `itemLists`: array of item list names to add the node to.
* `"UniqueRigidBodyNode"`: node using a rigid body from scene resources intended to be used only once within a scene graph.
	* `rigidBody`: the name of the rigid body within the scene resources.
	* `itemLists`: array of item list names to add the node to.
* `"PhysicsConstraintNode"`: node to instantiate a constraint, potentially across multiple sub-trees of the scene graph.
	* `constraint`: the name of the base constraint.
	* `firstActor`: the first actor for the constraint. This may be unset if using the existing first actor of the constraint, or an object with the following elements depending on how the actor is referenced:
		* For referencing an instance as part of a rigid body group node:
			* `rootNode`: the name of a distinct root node. This may be unset to use the rigid body group node.
			* `rigidBodyGroupNode`: the name of the rigid body group node.
			* `instance`: the name of the instance within the rigid body group node.
		* For referencing an actor (such as a rigid body) resource:
			* `actor`: the name of the actor.
	* `firstConnectedConstraint`: the first connected constraint. This may be unset if using the existing first connected constraint or if the first connected constraint is unused. Otherwise it is an object with the following elements depending on how the constraint is referenced:
		* For referencing an instance as part of a rigid body group node:
			* `rootNode`: the name of a distinct root node. This may be unset to use the rigid body group node.
			* `rigidBodyGroupNode`: the name of the rigid body group node.
			* `instance`: the name of the instance within the rigid body group node.
		* For referencing a constraint resource:
			* `constraint`: the name of the constraint.
		* For referencing another constraint node:
			* `constraintNode`: the name of the constraint node.
	* `secondActor`: the second actor for the constraint. This may be unset if using the existing second actor of the constraint, or an object with the following elements depending on how the actor is referenced:
		* For referencing an instance as part of a rigid body group node:
			* `rootNode`: the name of a distinct root node. This may be unset to use the rigid body group node.
			* `rigidBodyGroupNode`: the name of the rigid body group node.
			* `instance`: the name of the instance within the rigid body group node.
		* For referencing an actor (such as a rigid body) resource:
			* `actor`: the name of the actor.
	* `secondConnectedConstraint`: the second connected constraint. This may be unset if using the existing second connected constraint or if the second connected constraint is unused. Otherwise it is an object with the following elements depending on how the constraint is referenced:
		* For referencing an instance as part of a rigid body group node:
			* `rootNode`: the name of a distinct root node. This may be unset to use the rigid body group node.
			* `rigidBodyGroupNode`: the name of the rigid body group node.
			* `instance`: the name of the instance within the rigid body group node.
		* For referencing a constraint resource:
			* `constraint`: the name of the constraint.
		* For referencing another constraint node:
			* `constraintNode`: the name of the constraint node.
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
