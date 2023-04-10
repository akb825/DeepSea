# Scene Animation

The DeepSea Scene Animation library provides various structures, routines, and shaders to apply animations to a scene. This includes animating transforms within the scene graph and skinning of meshes.

# Additional Conversion Types

The following JSON formats are added to extend scene conversion.

## Scene Resources

The following custom scene resource types are provided with the members that are expected:

* `"DirectAnimation"`: values to set directly on an animation tree.
	* `channels`: array of channels for the animation. Each member of the array has the following members:
	* `node`: the name of the node to apply the value to.
		* `component`: the component to apply the value to. See the `dsAnimationComponent` enum for values, removing the type prefix.
		* `value`: the value for the channel as an array of three floats based on the component:
			* `"Translation"`: x, y, z offset.
			* `"Scale"`: x, y, z scale factors.
			* `"Rotation"`: x, y, z Euler angles in degrees.
* `"NodeMapCache"`: cache for node mappings between animations and animation trees. This contains no extra data members.

## Scene Nodes

The following scene node types are provided with the members that are expected:

* `"AnimationNode"`: node that manages an animation within its animation sub-graph. Child nodes can access the animation to apply to an animation tree.
	* `nodeMapCache`: the name of the animation node map cache to use with the animation tree.
	* `children`: an array of child nodes. Each element is an object with the following elements:
		* `nodeType`: the name of the node type.
		* `data`: the data for the node.
	* `itemLists`: array of item list names to add the node to.
* `"AnimationTransformNode"`: node that inherits a transform from an animation tree.
	* `animationNode`: the name of the node within the animation to retrieve the transform from.
	* `nodeMapCache`: the name of the animation node map cache to use with the animation tree.
	* `children`: an array of child nodes. Each element is an object with the following elements:
		* `nodeType`: the name of the node type.
		* `data`: the data for the node.
	* `itemLists`: array of item list names to add the node to.
* `"AnimationTreeNode"`: node that manages an animation tree within its animation sub-graph. Child nodes can access the animation tree to apply as a skin or an animation transform.
	* `animationTree`: the name of the animation tree to use for the node.
	* `nodeMapCache`: the name of the animation node map cache to use with the animation tree.
	* `children`: an array of child nodes. Each element is an object with the following elements:
		* `nodeType`: the name of the node type.
		* `data`: the data for the node.
	* `itemLists`: array of item list names to add the node to.

> **Note:** When creating an `AnimationNode`, `AnimationTransformNode`, or `AnimationTreeNode` it should be made a part of an `AnimationList` to ensure the animation and transforms are properly managed.

## Scene Item Lists

The following scene item lists are provided with the expected members:

* `"AnimationList"`: item list to manage `AnimationNode`, `AnimationTranformNode`, and `AnimationTreeNode` instances. This contains no extra data members.

## Instance Data

The following instance data types are provided with the expected members:

* `"SkinningData"`: management of transforms used for skinning to apply in the vertex shader. This contains no extra data members.