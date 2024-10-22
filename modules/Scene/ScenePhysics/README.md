# Scene Physics

The DeepSea Scene Physics library integrates the Physics and Scene libraries to update transforms based on the physics simulations and update kinematic objects based on manual transform changes within a scene.

# Additional Conversion Types

The following JSON formats are added to extend scene conversion.

## Scene Resources

The following custom scene resource types are provided with the members that are expected:

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
