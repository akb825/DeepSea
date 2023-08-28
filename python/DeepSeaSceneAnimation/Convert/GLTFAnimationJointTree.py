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

import struct
from .AnimationJointTreeConvert import AnimationJointTreeNode, addAnimationJointTreeType
from DeepSeaScene.Convert.GLTFModel import extractGLTFOrGLBBuffersViewsAccessors, readGLTFData, \
	readGLBData

def convertGLTFOrGLBAnimationJointTree(path, jsonData, binData, rootNodes):
	try:
		namedNodes = dict()
		indexNodes = dict()
		jsonNodes = jsonData['nodes']
		i = 0
		try:
			for node in jsonNodes:
				name = node.get('name')
				if name:
					namedNodes[name] = node
					indexNodes[i] = node
				i += 1
		except (TypeError, ValueError):
			raise Exception('Nodes must be an array of objects for GLTF file "' + path + '".')

		jsonSkins = jsonData['skins']
	except (TypeError, ValueError):
		raise Exception('Root value in GLTF file "' + path + '" must be an object.')
	except KeyError as e:
		raise Exception('GLTF file "' + path + '" doesn\'t contain element "' + str(e) + '".')

	buffers, bufferViews, accessors = extractGLTFOrGLBBuffersViewsAccessors(path, jsonData, binData)

	matrixStride = 16
	matrixSize = matrixStride*4
	indexMap = dict()
	animationNodes = []
	for rootName in rootNodes:
		rootNode = namedNodes.get(rootName)
		if not rootNode:
			raise Exception('Root node "' + rootName + '" not found in GLTF file "' + path + '".')

		try:
			skin = jsonSkins[int(rootNode['skin'])]
		except (KeyError, TypeError, ValueError, IndexError):
			raise Exception('Root node "' + rootName +
				'" doesn\'t have valid skin index in GLTF file "' + path + '".')

		try:
			joints = skin['joints']
			if not isinstance(joints, list):
				raise Exception('Skin "' + rootName +
					'" joints must be an array of ints for GLTF file "' + path + '".')

			inverseBindMatrices = skin['inverseBindMatrices']
			try:
				toLocalSpaceAccessor = accessors[int(inverseBindMatrices)]
				if (toLocalSpaceAccessor.decorator != 'Float' or
						toLocalSpaceAccessor.itemSize*toLocalSpaceAccessor.count !=
						len(joints)*matrixSize):
					raise Exception(
						'Skin inverseBindMatrices accessor for "' + rootName +
						'" must hold one 4x4 float matrix for each joint in GLTF file "' + path +
						'".')
			except (TypeError, ValueError, IndexError):
				raise Exception('Skin "' + rootName +
					'" doesn\'t have inverseBindMatrix index in GLTF file "' + path + '".')
		except (KeyError, TypeError, ValueError, IndexError):
			raise Exception('Root node "' + rootName +
				'" doesn\'t have valid skin index in GLTF file "' + path + '".')
		except (TypeError, ValueError):
			raise Exception('Skin "' + rootName + '" must be an object for GLTF file "' + path
			+ '".')
		except KeyError as e:
			raise Exception('Skin "' + rootName + '" doesn\'t contain element "' + str(e) +
				'" for GLTF file "' + path + '".')

		toLocalSpaceData = toLocalSpaceAccessor.extractData()
		for jointIndex in joints:
			try:
				jointIndex = int(jointIndex)
			except ValueError:
				raise Exception('Skin "' + rootName +
					'" joints must be an array of ints for GLTF file "' + path + '".')

			if jointIndex in indexMap:
				raise Exception('Animation node "' + indexNodes[jointIndex]['name'] +
					'" used multiple times in GLTF file "' + path + '".')

			jsonNode = indexNodes.get(jointIndex)
			if not jsonNode:
				raise Exception('Animation joint not found in skin "' + rootName +
					'" in GLTF file "' + path + '".')

			indexMap[jointIndex] = len(animationNodes)

			name = jsonNode.get('name')
			jsonScale = jsonNode.get('scale')
			if jsonScale:
				try:
					scale = (float(jsonScale[0]), float(jsonScale[1]), float(jsonScale[2]))
				except:
					raise Exception(
						'Animation node "scale" must be an array of 3 floats for GLTF file "' +
						path + '".')
			else:
				scale = (1.0, 1.0, 1.0)

			jsonRotation = jsonNode.get('rotation')
			if jsonRotation:
				try:
					rotation = (float(jsonRotation[0]), float(jsonRotation[1]), float(jsonRotation[2]),
						float(jsonRotation[3]))
				except:
					raise Exception(
						'Animation node "rotation" must be an array of 4 floats for GLTF file "' +
						path + '".')
			else:
				rotation = (0.0, 0.0, 0.0, 1.0)

			jsonTranslation = jsonNode.get('translation')
			if jsonTranslation:
				try:
					translation = (float(jsonTranslation[0]), float(jsonTranslation[1]),
						float(jsonTranslation[2]))
				except:
					raise Exception(
						'Animation node "translation" must be an array of 3 floats for GLTF file "' +
						path + '".')
			else:
				translation = (0.0, 0.0, 0.0)

			jsonChildIndices = jsonNode.get('children', [])
			childIndices = []
			try:
				for childIndex in jsonChildIndices:
					# Store the original indices at first, then resolve once we've read in all
					# joints.
					childIndices.append(int(childIndex))
			except (TypeError, ValueError):
				raise Exception('Node "children" must be an array of ints for node "' + name +
					'" for GLTF file "' + path + '".')

			baseOffset = jointIndex*matrixSize
			toLocalSpace = (
				struct.unpack_from('ffff', toLocalSpaceData, baseOffset),
				struct.unpack_from('ffff', toLocalSpaceData, baseOffset + matrixStride),
				struct.unpack_from('ffff', toLocalSpaceData, baseOffset + matrixStride*2),
				struct.unpack_from('ffff', toLocalSpaceData, baseOffset + matrixStride*3)
			)

			animationNodes.append(AnimationJointTreeNode(name, scale, rotation, translation,
				toLocalSpace, childIndices))

	# Now that we've read in all joints, need to remap child indices based on the final array.
	for node in animationNodes:
		try:
			node.childIndices = [indexMap[child] for child in node.childIndices]
		except IndexError:
			raise Exception('Invalid child index for node "' + node.name +
				'" for GLTF file "' + path + '".')

	return animationNodes

def convertGLTFAnimationJointTree(convertContext, path, rootNodes):
	"""
	Converts a glTF animation joint tree based on a list of root node names.

	Limitations:
	- All children of the root nodes must be named.
	- Transforms must be separated into "scale", "rotation", and "translation". A single "matrix"
	  isn't supported.
	"""
	data = readGLTFData(path)
	return convertGLTFOrGLBAnimationJointTree(path, data, None, rootNodes)

def convertGLBAnimationJointTree(convertContext, path, rootNodes):
	"""
	Converts a GLB animation joint tree based on a list of root node names.

	Limitations:
	- All children of the root nodes must be named.
	- Transforms must be separated into "scale", "rotation", and "translation". A single "matrix"
	  isn't supported.
	"""
	jsonData, binData = readGLBData(path)
	return convertGLTFOrGLBAnimationJointTree(path, jsonData, binData, rootNodes)

def registerGLTFAnimationJointTreeType(convertContext):
	"""
	Registers the GLTF animation Joint tree type under the name "gltf" and GLB under the name "glb".
	"""
	addAnimationJointTreeType(convertContext, 'gltf', convertGLTFAnimationJointTree)
	addAnimationJointTreeType(convertContext, 'glb', convertGLBAnimationJointTree)
