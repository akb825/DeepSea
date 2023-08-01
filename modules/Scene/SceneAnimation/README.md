# Scene Animation

The DeepSea Scene Animation library provides various structures, routines, and shaders to apply animations to a scene. This includes animating transforms within the scene graph and skinning of meshes.

# Additional Conversion Types

The following JSON formats are added to extend scene conversion.

## Scene Resources

The following custom scene resource types are provided with the members that are expected:

* `"AnimationTree"`: animation tree without joint nodes.
	* `file`: file with the animation tree. If omitted, the data will be provided inline.
	* `fileType`: the name of the type, such as "gltf". If omitted, the type is inferred from the file extension.
	* `nodes`: list of nodes to define the animation tree. If `file` is set, this will be the list of root node names. If `file` is not set, each element of the array has the following members:
		* `name`: the name of the node.
		* `translation`: array with x, y, z offset. Defaults to [0, 0, 0].
		* `scale`: array with x, y, z scale factors. Defaults to [1, 1, 1].
		* `rotation`: array with x, y, z Euler angles in degrees. Defaults to [0, 0, 0].
		* `children`: array with the child nodes. Each element of the array has the same members as the `nodes` members. Defaults to no children if omitted.
* `"AnimationJointTree"`: animation tree with joint nodes.
	* `file`: file with the animation joint tree. If omitted, the data will be provided inline.
	* `fileType`: the name of the type, such as "gltf". If omitted, the type is inerred from the file extension.
	* `nodes`: list of nodes to define the animation tree. If `file` is set, this will be the list of root node names. If `file` is not set, each element of the array has the following members:
		* `name`: the name of the node.
		* `translation`: array with x, y, z offset. Defaults to [0, 0, 0].
		* `scale`: array with x, y, z scale factors. Defaults to [1, 1, 1].
		* `rotation`: array with x, y, z Euler angles in degrees. Defaults to [0, 0, 0].
		* `toLocalSpace`: 4x4 2D array for a column-major matrix converting to local joint space.
		* `childIndices`: array with the child node indices.
* `"DirectAnimation"`: values to set directly on an animation tree.
	* `channels`: array of channels for the animation. Each member of the array has the following members:
		* `node`: the name of the node to apply the value to.
		* `component`: the component to apply the value to. See the `dsAnimationComponent` enum for values, removing the type prefix.
		* `value`: the value for the channel as an array of three floats based on the component:
			* `"Translation"`: x, y, z offset.
			* `"Scale"`: x, y, z scale factors.
			* `"Rotation"`: x, y, z Euler angles in degrees.
* `"KeyframeAnimation"`: values to set on an animation tree interpolated by time value.
	* `file`: file with the keyframe animation. If omitted, the data will be provided inline.
	* `fileType`: the name of the type, such as "gltf". If omitted, the type is inferred from the file extension.
	* `keyframes`: array of keyframes and channels that comprise the animation. If "file" is set, this will be the list of keyframe animation names. If "file" is not set, each element of the array has the following members:
		* `keyframeTimes`: array of times for each keyframe in seconds.
		* `channels`: array of channels associated with the keyframe times. Each element of the array has the following members:
			* `node`: the name of the node the channel applies to.
			* `component`: the component to apply the value to. See the `dsAnimationComponent` enum for values, removing the type prefix.
			* `interpolation`: the interpolation method of the. See the `dsAnimationInterpolation` enum for values, removing the type prefix.
			* `values`: The values for the animation component as an array of float arrays. The inner arrays have 3 elements for translation and scale, or 4 elements for a quaternion for rotation. The outer array has either one element for each keyframe time for step and linear interpolation, or three elements for each keyframe time for cubic interpolation. The three cubic values are the in tangent, value, and out tangent, respectively.
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
