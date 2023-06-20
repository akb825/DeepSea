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

from .KeyframeAnimationConvert import Channel, Keyframes, addKeyframeAnimationType
from DeepSeaAnimation.AnimationComponent import AnimationComponent
from DeepSeaAnimation.AnimationInterpolation import AnimationInterpolation
from DeepSeaScene.Convert.GLTFModel import \
	extractGLTFOrGLBBuffersViewsAccessors, readGLTFData, readGLBData

def convertGLTFOrGLBKeyframeAnimation(path, jsonData, binData, animations):
	try:
		namedAnimations = dict()
		jsonAnimations = jsonData['animations']
		try:
			for animation in jsonAnimations:
				name = animation.get('name')
				if name:
					namedAnimations[name] = animation
		except (TypeError, ValueError):
			raise Exception('Animations must be an array of objects for GLTF file "' + path + '".')

		nodeNames = []
		jsonNodes = jsonData['nodes']
		try:
			for node in jsonNodes:
				nodeNames.append(node.get('name'))
		except (TypeError, ValueError):
			raise Exception('Nodes must be an array of objects for GLTF file "' + path + '".')
	except (TypeError, ValueError):
		raise Exception('Root value in GLTF file "' + path + '" must be an object.')
	except KeyError as e:
		raise Exception('GLTF file "' + path + '" doesn\'t contain element "' + str(e) + '".')

	buffers, views, accessors = extractGLTFOrGLBBuffersViewsAccessors(path, jsonData, binData)

	def constructKeyframes(name, jsonAnimation):
		try:
			jsonSamplers = jsonAnimation['samplers']
			samplers = []
			try:
				for jsonSampler in jsonSamplers:
					keyframeAccessor = jsonSampler['input']
					channelAccessor = jsonSampler['output']
					interpolationStr = str(jsonSampler['interpolation'])
					if interpolationStr == 'STEP':
						interpolation = AnimationInterpolation.Step
					elif interpolationStr == 'LINEAR':
						interpolation = AnimationInterpolation.Linear
					elif interpolationStr == 'CUBICSPLINE':
						interpolation = AnimationInterpolation.Cubic
					else:
						raise Exception('Animation "' + name +
							'" contains invalid interpolation type "' + interpolationStr +
							'" in GLTF file "' + path + '".')
					samplers.append((keyframeAccessor, channelAccessor, interpolation))
			except (TypeError, ValueError):
				raise Exception('Animation "' + name +
					'" "samplers" must be an array of objects in GTLF file "' + path + '".')
			except KeyError as e:
				raise Exception(
					'Animation "' + name + '" "samplers" doesn\'t contain element "' + str(e) +
					'" in GLTF file "' + path + '".')

			keyframeChannels = dict()
			jsonChannels = jsonAnimation['channels']
			try:
				for jsonChannel in jsonChannels:
					jsonTarget = jsonChannel['target']
					nodeIndex = jsonTarget['node']

					nodeName = None
					try:
						nodeName = nodeNames[nodeIndex]
					except:
						pass
					if not nodeName:
						raise Exception('Animation "' + name + '" node "' + str(nodeIndex) +
						'" has no corresponding name in GLTF file "' + path + '".')

					samplerIndex = jsonChannel['sampler']
					try:
						keyframeAccessor, channelAccessor, interpolation = samplers[samplerIndex]
					except:
						raise Exception('Animation "' + name + '" sampler "' + str(samplerIndex) +
							'" is invalid in GLTF file "' + path + '".')

					componentStr = jsonTarget['path']
					if componentStr == 'translation':
						component = AnimationComponent.Translation
					elif componentStr == 'scale':
						component = AnimationComponent.Scale
					elif componentStr == 'rotation':
						component = AnimationComponent.Rotation
					else:
						raise Exception('Animation "' + name + '" contains invalid component "' +
							componentStr + '" in GLTF file "' + path + '".')

					thisKeyframeChannels = keyframeChannels.get(keyframeAccessor)
					if not thisKeyframeChannels:
						thisKeyframeChannels = []
						keyframeChannels[keyframeAccessor] = thisKeyframeChannels
					thisKeyframeChannels.apppend(
						(nodeName, component, interpolation, channelAccessor))
			except (TypeError, ValueError):
				raise Exception('Animation "' + name +
					'" "channels" must be an array of objects in GTLF file "' + path + '".')
			except KeyError as e:
				raise Exception(
					'Animation "' + name + '" "channels" doesn\'t contain element "' + str(e) +
					'" in GLTF file "' + path + '".')
		except (TypeError, ValueError):
			raise Exception(
				'Animation "' + name + '" must be an object in GTLF file "' + path + '".')
		except KeyError as e:
			raise Exception(
				'Animation "' + name + '" doesn\'t contain element "' + str(e) +
				'" in GLTF file "' + path + '".')

		animationKeyframes = []
		for keyframeAccessor, channels in keyframeChannels.items():
			try:
				keyframeData = accessors[keyframeAccessor]
			except:
				raise Exception('Animation "' + name + '" keyframe accessor "' +
					str(keyframeAccessor) + ' is invalid in GLTF file "' + path + '".')
			if keyframeData.type != 'X32' or keyframeData.decorator != 'Float':
				raise Exception('Animation "' + name +
					'" keyframe data must be an array of floats in GLTF file "' + path + '".')

			keyframeBytes = keyframeData.extractData()
			keyframeValues = []
			for i in range(keyframeData.count):
				keyframeValues.append(struct.unpack_from('f', keyframeBytes, i*4)[0])
			keyframeCount = len(keyframeValues)

			thisKeyframeChannels = []
			for nodeName, component, interpolation, channelAccessor in channels:
				try:
					channelData = accessors[channelAccessor]
				except:
					raise Exception('Animation "' + name + '" channel accessor "' +
					str(channelAccessor) + ' is invalid in GLTF file "' + path + '".')

				if not channelData.type.startswith('X32') or channelData.decorator != 'Float':
					raise Exception('Animation "' + name +
						'" channel data must be an array of floats in GLTF file "' + path + '".')

				channelValueCount = channelData.count*(channelData.itemSize//4)
				componentCount = 4 if component == AnimationComponent.Rotation else 3
				perKeyframeCount = 3 if interpolation == AnimationInterpolation.Cubic else 1
				if channelValueCount != keyframeCount*componentCount*perKeyframeCount:
					raise Exception('Animation "' + name +
						'" channel data is of invalid size in GLTF file "' + path + '".')

				componentSize = componentCount*4
				componentFormat = 'f'*componentCount

				channelByes = channelData.extractData()
				channelValues = []
				for i in range(keyframeCount*perKeyframeCount):
					channelValues.append(
						struct.unpack_from(componentFormat, keyframeBytes, i*componentSize))

				thisKeyframeChannels.append(
					Channel(nodeName, component, interpolation, channelValues))
			animationKeyframes.append(Keyframes(keyframeValues, thisKeyframeChannels))
		return animationKeyframes

	keyframes = []
	for animationName in animations:
		try:
			animation = namedAnimations[animationName]
		except KeyError:
			raise Exception('Animation "' + animationName + '" not in GLTF file "' + path + '".')
		keyframes.append(constructKeyframes(animationName, animation))

	return keyframes 

def convertGLTFKeyframeAnimation(convertContext, path, animations):
	"""
	Converts a glTF animation tree based on a list of animation names.
	"""
	data = readGLTFData(path)
	return convertGLTFOrGLBKeyframeAnimation(path, data, None, animations)

def convertGLBKeyframeAnimation(convertContext, path, animations):
	"""
	Converts a GLB keyframe animation based on a list of animation names.
	"""
	jsonData, binData = readGLBData(path)
	return convertGLTFOrGLBKeyframeAnimation(path, jsonData, binData, animations)

def registerGLTFKeyframeAnimationType(convertContext):
	"""
	Registers the GLTF keyframe animation type under the name "gltf" and GLB under the name "glb".
	"""
	addKeyframeAnimationType(convertContext, 'gltf', convertGLTFKeyframeAnimation)
	addKeyframeAnimationType(convertContext, 'glb', convertGLBKeyframeAnimation)
