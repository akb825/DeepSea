# Copyright 2023 Aaron Barany
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import flatbuffers
import os

from DeepSeaAnimation.Convert.Quaternion import eulerToQuaternion
from DeepSeaAnimation.Quaternion4f import CreateQuaternion4f
from DeepSeaAnimation.Vector3f import CreateVector3f
from DeepSeaAnimation import AnimationTreeNode as AnimationNode
from DeepSeaAnimation import AnimationTree

class AnimationTreeNode:
	"""
	Node within an animation tree.
	"""
	def __init__(self, name, scale, rotation, translation):
		"""
		Initializes an animation tree node.
		Scale and translation are a tuple of 3 floats, rotation is a tuple of 4 floats.
		"""
		self.name = name
		self.scale = scale
		self.rotation = rotation
		self.translation = translation
		self.children = []

def addAnimationTreeType(convertContext, typeName, convertFunc):
	"""
	Adds an animation tree type with the name and the convert function.

	The function should take the ConvertContext, path to the animation tree to convert, and the list
	of root node names, and should return an array of AnimationTreeNode objects for the contents of
	the animation tree.

	An exception will be raised if the type is already registered.
	"""
	if not hasattr(convertContext, 'animationTreeTypeMap'):
		convertContext.animationTreeTypeMap = dict()

	if typeName in convertContext.animationTreeTypeMap:
		raise Exception('Animation tree type "' + typeName + '" is already registered.')
	convertContext.animationTreeTypeMap[typeName] = convertFunc

def convertAnimationTree(convertContext, data):
	"""
	Converts an AnimationTree without joints. The data map is expected to contain the following
	elements:
	- file: file with the animation tree.
	- fileType: the name of the type, such as "gltf". If ommitted, the type is inerred from the
	  file extension.
	- nodes: list of nodes to define the animation tree. If "file" is set, this will be the list
	  of root node names. If "file" is not set, each element of the array has the following members:
	  - name: the name of the node.
	  - translation: array with x, y, z offset. Defaults to [0, 0, 0].
	  - scale: array with x, y, z scale factors. Defaults to [1, 1, 1].
	  - rotation: array with x, y, z Euler angles in degrees. Defaults to [0, 0, 0].
	  - children: array with the child nodes. Each element of the array has the same members as the
	    "nodes" members. Defaults to no children if ommitted.
	"""
	def readFloat(value, name):
		try:
			floatVal = float(value)
			return floatVal
		except:
			raise Exception('Invalid ' + name + ' value "' + str(value) + '".')

	def readValues(value, name):
		if not isinstance(value, list) or len(value) != 3:
			raise Exception('AnimationTree "' + name + '" must be an array of three floats.')

		return (readFloat(value[0], name), readFloat(value[1], name), readFloat(value[2], name))

	def convertNodeRec(node):
		try:
			name = str(node['name'])

			scaleData = node.get('scale')
			if scaleData:
				scale = readValues(scaleData, 'scale')
			else:
				scale = (1.0, 1.0, 1.0)

			rotationData = node.get('rotation')
			if rotationData:
				rotation = readValues(rotationData, 'rotation')
			else:
				rotation = (0.0, 0.0, 0.0)

			translationData = node.get('translation')
			if translationData:
				translation = readValues(translationData, 'translation')
			else:
				translation = (0.0, 0.0, 0.0)

			treeNode = AnimationTreeNode(name, scale, eulerToQuaternion(*rotation), translation)

			childNodeData = node.get('children', [])
			try:
				for child in childNodeData:
					treeNode.children.append(convertNodeRec(child))
			except (TypeError, ValueError):
				raise Exception('AnimationTree node "children" must be an array of objects.')

			return treeNode
		except KeyError as e:
			raise Exception('AnimationTree node doesn\'t contain element ' + str(e) + '.')

	try:
		path = str(data.get('file', ''))
		nodeData = data['nodes']

		if not nodeData:
			raise Exception('AnimationTree contains no nodes.')

		if path:
			fileType = str(data.get('fileType', ''))
			if not fileType:
				fileType = os.splittype(path)[1]
				if fileType:
					fileType = fileType[1:]
				if not fileType:
					raise Exception('AnimationTree file has no known file type.')

			convertFunc = convertContext.animationTreeTypeMap.get(fileType) \
				if hasattr(convertContext, 'animationTreeTypeMap') else None
			if not convertFunc:
				raise Exception('Animation tree type "' + fileType + '" hasn\'t been registered.')

			if not isinstance(nodeData, list):
				raise Exception('AnimationTree "nodes" must be a list of strings.')
			for node in nodeData:
				if not isinstance(node, str):
					raise Exception('AnimationTree "nodes" must be a list of strings.')

			rootNodes = convertFunc(convertContext, path, nodeData)
		else:
			rootNodes = []
			try:
				for curNode in nodeData:
					rootNodes.append(convertNodeRec(curNode))
			except (TypeError, ValueError):
				raise Exception('AnimationTree "nodes" must be an array of objects.')
	except (TypeError, ValueError):
		raise Exception('AnimationTree data must be an object.')
	except KeyError as e:
		raise Exception('AnimationTree data doesn\'t contain element ' + str(e) + '.')

	if not rootNodes:
		raise Exception('AnimationTree contains no nodes.')

	builder = flatbuffers.Builder(0)

	def writeNodeRec(node):
		childOffsets = []
		for child in node.children:
			childOffsets.append(writeNodeRec(child))

		if childOffsets:
			AnimationNode.StartChildrenVector(builder, len(childOffsets))
			for offset in reversed(childOffsets):
				builder.PrependUOffsetTRelative(offset)
			childrenOffset = builder.EndVector()
		else:
			childrenOffset = 0

		nameOffset = builder.CreateString(node.name)

		AnimationNode.Start(builder)
		AnimationNode.AddName(builder, nameOffset)
		AnimationNode.AddScale(builder, CreateVector3f(builder, *node.scale))
		AnimationNode.AddRotation(builder, CreateQuaternion4f(builder, *node.rotation))
		AnimationNode.AddTranslation(builder, CreateVector3f(builder, *node.translation))
		AnimationNode.AddChildren(builder, childrenOffset)
		return AnimationNode.End(builder)

	rootNodeOffsets = []
	for node in rootNodes:
		rootNodeOffsets.append(writeNodeRec(node))

	AnimationTree.StartRootNodesVector(builder, len(rootNodeOffsets))
	for offset in reversed(rootNodeOffsets):
		builder.PrependUOffsetTRelative(offset)
	rootNodesOffset = builder.EndVector()

	AnimationTree.Start(builder)
	AnimationTree.AddRootNodes(builder, rootNodesOffset)
	AnimationTree.AddJointNodes(builder, 0)
	builder.Finish(AnimationTree.End(builder))
	return builder.Output()
