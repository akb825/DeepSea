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

from .AnimationTreeConvert import AnimationTreeNode, addAnimationTreeType
from DeepSeaScene.Convert.GLTFModel import readGLTFData, readGLBData

def convertGLTFOrGLBAnimationTree(path, jsonData, rootNodes):
	try:
		namedNodes = dict()
		jsonNodes = jsonData['nodes']
		try:
			for node in jsonNodes:
				name = node.get('name')
				if name:
					namedNodes[name] = node
		except (TypeError, ValueError):
			raise Exception('Nodes must be an array of objects for GLTF file "' + path + '".')
	except (TypeError, ValueError):
		raise Exception('Root value in GLTF file "' + path + '" must be an object.')
	except KeyError as e:
		raise Exception('GLTF file "' + path + '" doesn\'t contain element "' + str(e) + '".')

	def constructNodeAndChildren(name, jsonNode):
		if 'matrix' in jsonNode:
			raise Exception('Animation node "' + name + '" must not have single "matrix" transform '
				'for GLTF file "' + path + '".')

		jsonScale = jsonNode.get('scale')
		if jsonScale:
			try:
				scale = (float(jsonScale[0]), float(jsonScale[1]), float(jsonScale[2]))
			except:
				raise Exception('Animation node "scale" must be an array of 3 floats for GLTF file "' +
					path + '".')
		else:
			scale = (1.0, 1.0, 1.0)

		jsonRotation = jsonNode.get('rotation')
		if jsonRotation:
			try:
				rotation = (float(jsonRotation[0]), float(jsonRotation[1]), float(jsonRotation[2]),
					float(jsonRotation[3]))
			except:
				raise Exception('Animation node "rotation" must be an array of 4 floats for GLTF '
					'file "' + path + '".')
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

		node = AnimationTreeNode(name, scale, rotation, translation)
		childIndices = jsonNode.get('children', [])
		try:
			for childIndex in childIndices:
				childJson = jsonNodes[childIndex]
				childName = childJson.get('name')
				if not childName:
					raise Exception(
						'Nodes part of an animation tree must contain names for GLTF file "' +
						path + '".')
				node.children.append(constructNodeAndChildren(childName, childJson))
		except (TypeError, ValueError):
			raise Exception('Node "children" must be an array of ints for node "' + name +
				'" for GLTF file "' + path + '".')
		except IndexError:
			raise Exception('Invalid child index for node "' + name + '" for GLTF file "' + path +
				'".')

		return node

	rootAnimationNodes = []
	for rootName in rootNodes:
		rootNode = namedNodes.get(rootName)
		if not rootNode:
			raise Exception('Root node "' + rootName + '" not found in GLTF file "' + path + '".')
		rootAnimationNodes.append(constructNodeAndChildren(rootName, rootNode))

	return rootAnimationNodes

def convertGLTFAnimationTree(convertContext, path, rootNodes):
	"""
	Converts a glTF animation tree based on a list of root node names.

	Limitations:
	- All children of the root nodes must be named.
	- Transforms must be separated into "scale", "rotation", and "translation". A single "matrix"
	  isn't supported.
	"""
	data = readGLTFData(path)
	return convertGLTFOrGLBAnimationTree(path, data, rootNodes)

def convertGLBAnimationTree(convertContext, path, rootNodes):
	"""
	Converts a GLB animation tree based on a list of root node names.

	Limitations:
	- All children of the root nodes must be named.
	- Transforms must be separated into "scale", "rotation", and "translation". A single "matrix"
	  isn't supported.
	"""
	data, _ = readGLBData(path)
	return convertGLTFOrGLBAnimationTree(path, data, rootNodes)

def registerGLTFAnimationTreeType(convertContext):
	"""
	Registers the GLTF animation tree type under the name "gltf" and GLB under the name "glb".
	"""
	addAnimationTreeType(convertContext, 'gltf', convertGLTFAnimationTree)
	addAnimationTreeType(convertContext, 'glb', convertGLBAnimationTree)
