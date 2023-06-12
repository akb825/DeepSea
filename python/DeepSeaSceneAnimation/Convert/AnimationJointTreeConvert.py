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
from DeepSeaAnimation.Matrix44f import CreateMatrix44f
from DeepSeaAnimation.Quaternion4f import CreateQuaternion4f
from DeepSeaAnimation.Vector3f import CreateVector3f
from DeepSeaAnimation import AnimationJointTreeNode as AnimationJointNode
from DeepSeaAnimation import AnimationTree

class AnimationJointTreeNode:
	"""
	Joint node within an animation tree.
	"""
	def __init__(self, name, scale, rotation, translation, toLocalSpace, childIndices = []):
		"""
		Initializes an animation tree joint node.
		Scale and translation are a tuple of 3 floats, rotation is a tuple of 4 floats.
		toLocalSpace is a 4x4 tuple of floats for a column-major index.
		"""
		self.name = name
		self.scale = scale
		self.rotation = rotation
		self.translation = translation
		self.toLocalSpace = toLocalSpace
		self.childIndices = childIndices

def addAnimationJointTreeType(convertContext, typeName, convertFunc):
	"""
	Adds an animation joint tree type with the name and the convert function.

	The function should take the ConvertContext, path to the animation tree to convert, and the list
	of root node names, and should return a list of AnimationJointTreeNode objects for the contents
	of the animation joint tree.

	An exception will be raised if the type is already registered.
	"""
	if not hasattr(convertContext, 'animationJointTreeTypeMap'):
		convertContext.animationJointTreeTypeMap = dict()

	if typeName in convertContext.animationJointTreeTypeMap:
		raise Exception('Animation joint tree type "' + typeName + '" is already registered.')
	convertContext.animationJointTreeTypeMap[typeName] = convertFunc

def convertAnimationJointTree(convertContext, data):
	"""
	Converts an AnimationTree with joints. The data map is expected to contain the following
	elements:
	- file: file with the animation joint tree.
	- fileType: the name of the type, such as "gltf". If omitted, the type is inerred from the
	  file extension.
	- nodes: list of nodes to define the animation tree. If "file" is set, this will be the list
	  of root node names. If "file" is not set, each element of the array has the following members:
	  - name: the name of the node.
	  - translation: array with x, y, z offset. Defaults to [0, 0, 0].
	  - scale: array with x, y, z scale factors. Defaults to [1, 1, 1].
	  - rotation: array with x, y, z Euler angles in degrees. Defaults to [0, 0, 0].
	  - toLocalSpace: 4x4 2D array for a column-major matrix converting to local joint space.
	  - childIndices: array with the child node indices.
	"""
	def readFloat(value, name):
		try:
			floatVal = float(value)
			return floatVal
		except:
			raise Exception('Invalid ' + name + ' value "' + str(value) + '".')

	def readValues(value, name):
		if not isinstance(value, list) or len(value) != 3:
			raise Exception('AnimationJointTree "' + name + '" must be an array of three floats.')

		return (readFloat(value[0], name), readFloat(value[1], name), readFloat(value[2], name))

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
					raise Exception('AnimationJointTree file has no known file type.')

			convertFunc = convertContext.animationJointTreeTypeMap.get(fileType) \
				if hasattr(convertContext, 'animationJointTreeTypeMap') else None
			if not convertFunc:
				raise Exception('Animation joint tree type "' + fileType + 
					'" hasn\'t been registered.')

			if not isinstance(nodeData, list):
				raise Exception('AnimationTree "nodes" must be a list of strings.')
			for node in nodeData:
				if not isinstance(node, str):
					raise Exception('AnimationTree "nodes" must be a list of strings.')

			nodes = convertFunc(convertContext, path, nodeData)
		else:
			nodes = []
			try:
				for curNode in nodeData:
					try:
						name = str(curNode['name'])

						scaleData = curNode.get('scale')
						if scaleData:
							scale = readValues(scaleData, 'scale')
						else:
							scale = (1.0, 1.0, 1.0)

						rotationData = curNode.get('rotation')
						if rotationData:
							rotation = readValues(rotationData, 'rotation')
						else:
							rotation = (0.0, 0.0, 0.0)

						translationData = curNode.get('translation')
						if translationData:
							translation = readValues(translationData, 'translation')
						else:
							translation = (0.0, 0.0, 0.0)

						toLocalSpaceData = curNode['toLocalSpace']
						if (not isinstance(toLocalSpaceData, list) or len(toLocalSpaceData) != 4 or
								not isinstance(toLocalSpaceData[0], list) or
									len(toLocalSpaceData[0]) != 4 or
								not isinstance(toLocalSpaceData[1], list) or
									len(toLocalSpaceData[1]) != 4 or
								not isinstance(toLocalSpaceData[2], list) or
									len(toLocalSpaceData[2]) != 4 or
								not isinstance(toLocalSpaceData[3], list) or
									len(toLocalSpaceData[3]) != 4):
							raise Exception(
								'AnimationJointTree "toLocalSpace" must be a 4x4 array of floats.')
						valueName = 'toLocalSpace'
						toLocalSpace = (
							(
								readFloat(toLocalSpace[0][0], valueName),
								readFloat(toLocalSpace[0][1], valueName),
								readFloat(toLocalSpace[0][2], valueName),
								readFloat(toLocalSpace[0][3], valueName),
							),
							(
								readFloat(toLocalSpace[1][0], valueName),
								readFloat(toLocalSpace[1][1], valueName),
								readFloat(toLocalSpace[1][2], valueName),
								readFloat(toLocalSpace[1][3], valueName),
							),
							(
								readFloat(toLocalSpace[2][0], valueName),
								readFloat(toLocalSpace[2][1], valueName),
								readFloat(toLocalSpace[2][2], valueName),
								readFloat(toLocalSpace[2][3], valueName),
							),
							(
								readFloat(toLocalSpace[3][0], valueName),
								readFloat(toLocalSpace[3][1], valueName),
								readFloat(toLocalSpace[3][2], valueName),
								readFloat(toLocalSpace[3][3], valueName),
							)
						)

						treeNode = AnimationJointTreeNode(name, scale, eulerToQuaternion(*rotation),
							translation, toLocalSpace)

						childNodeData = curNode.get('childIndices', [])
						try:
							for child in childNodeData:
								treeNode.childIndices.append(int(child))
						except (TypeError, ValueError):
							raise Exception('AnimationJointTree node "children" must be an array of ints.')
					except KeyError as e:
						raise Exception('AnimationJointTree node doesn\'t contain element ' +
							str(e) + '.')
					nodes.append(treeNode)
			except (TypeError, ValueError):
				raise Exception('AnimationJointTree "nodes" must be an array of objects.')
	except (TypeError, ValueError):
		raise Exception('AnimationJointTree data must be an object.')
	except KeyError as e:
		raise Exception('AnimationJointTree data doesn\'t contain element ' + str(e) + '.')

	if not nodes:
		raise Exception('AnimationJointTree contains no nodes.')

	builder = flatbuffers.Builder(0)

	nodeOffsets = []
	for node in nodes:
		if node.childIndices:
			AnimationJointNode.StartChildrenVector(builder, len(node.childIndices))
			for child in reversed(node.childIndices):
				builder.PrependUint32(child)
			childrenOffset = builder.EndVector()
		else:
			childrenOffset = 0

		nameOffset = builder.CreateString(node.name)

		AnimationJointNode.Start(builder)
		AnimationJointNode.AddName(builder, nameOffset)
		AnimationJointNode.AddScale(builder, CreateVector3f(builder, *node.scale))
		AnimationJointNode.AddRotation(builder, CreateQuaternion4f(builder, *node.rotation))
		AnimationJointNode.AddTranslation(builder, CreateVector3f(builder, *node.translation))
		AnimationJointNode.AddToLocalSpace(builder, CreateMatrix44f(builder,
			node.toLocalSpace[0][0], node.toLocalSpace[0][1], node.toLocalSpace[0][2],
			node.toLocalSpace[0][3], node.toLocalSpace[1][0], node.toLocalSpace[1][1],
			node.toLocalSpace[1][2], node.toLocalSpace[1][3], node.toLocalSpace[2][0],
			node.toLocalSpace[2][1], node.toLocalSpace[2][2], node.toLocalSpace[2][3],
			node.toLocalSpace[3][0], node.toLocalSpace[3][1], node.toLocalSpace[3][2],
			node.toLocalSpace[3][3]))
		AnimationJointNode.AddChildren(builder, childrenOffset)
		nodeOffsets.append(AnimationJointNode.End(builder))

	AnimationTree.StartJointNodesVector(builder, len(nodeOffsets))
	for offset in reversed(nodeOffsets):
		builder.PrependUOffsetTRelative(offset)
	nodesOffset = builder.EndVector()

	AnimationTree.Start(builder)
	AnimationTree.AddRootNodes(builder, 0)
	AnimationTree.AddJointNodes(builder, nodesOffset)
	builder.Finish(AnimationTree.End(builder))
	return builder.Output()
