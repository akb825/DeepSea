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

from DeepSeaAnimation import AnimationNode
from DeepSeaAnimation import AnimationTree
from DeepSeaAnimation import Quaternion4f
from DeepSeaAnimation import Vector3f

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
	of root nodes, and should return an array of AnimationTreeNode objects for the contents of the
	animation tree.
	An exception will be raised if the type is already registered.
	"""
	if not hasattr(convertContext, 'animationTreeTypeMap'):
		convertContext.animationTreeTypeMap = dict()

	if typeName in convertContext.modelTypeMap:
		raise Exception('Animation tree type "' + typeName + '" is already registered.')
	convertContext.animationTreeTypeMap[typeName] = convertFunc
