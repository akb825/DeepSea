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

from DeepSeaAnimation import AnimationKeyframes
from DeepSeaAnimation import KeyframeAnimationChannel
from DeepSeaAnimation import KeyframeAnimation
from DeepSeaAnimation.AnimationComponent import AnimationComponent
from DeepSeaAnimation.AnimationInterpolation import AnimationInterpolation
from DeepSeaAnimation.Vector4f import CreateVector4f

class Channel:
	"""
	Channel within a keyframe animation.
	"""
	def __init__(self, node, component, interpolation, values):
		"""
		Initializes the channel.
		Values should be an array of tuples, where the inner tuple is 3 elements for translation and
		scale or 4 elements for rotation as a quaternion. The outer array should have one element
		for each keyframe time for linear or step interpolation, or three elements for each keyframe
		time for cubic interpolation.
		"""
		self.node = node
		self.component = component
		self.interpolation = interpolation
		self.values = values

		assert isinstance(self.values, list)
		for value in self.values:
			assert isinstance(value, tuple)
			if self.component == AnimationComponent.Rotation:
				assert len(value) == 4
			else:
				assert len(value) == 3

class Keyframes:
	"""
	Keyframes for a keyframe animation, including the keyframe times and channels.
	"""
	def __init__(self, keyframeTimes, channels):
		"""
		Initializes the keyframes.
		"""
		self.keyframeTimes = keyframeTimes
		self.channels = channels

		assert isinstance(self.keyframeTimes, list)
		assert isinstance(self.channels, list)
		for channel in self.channels:
			assert isinstance(channel, Channel)
			if channel.interpolation == AnimationInterpolation.Cubic:
				assert len(channel.values) == len(self.keyframeTimes)*3
			else:
				assert len(channel.values) == len(self.keyframeTimes)

def addKeyframeAnimationType(convertContext, typeName, convertFunc):
	"""
	Adds a keyframe animation type with the name and the convert function.

	The function should take the ConvertContext, path to the keyframe animation to convert, and the
	list of animation channel names, and should return an array of AnimationTreeNode objects for the contents of
	the animation tree.

	An exception will be raised if the type is already registered.
	"""
	if not hasattr(convertContext, 'keyframeAnimationTypeMap'):
		convertContext.keyframeAnimationTypeMap = dict()

	if typeName in convertContext.keyframeAnimationTypeMap:
		raise Exception('Keyframe animation type "' + typeName + '" is already registered.')
	convertContext.keyframeAnimationTypeMap[typeName] = convertFunc

def convertKeyframeAnimation(convertContext, data):
	"""
	Converts a keyframe animation for a scene. The data map is expected to contain the following
	elements:
	- file: file with the keyframe animation. If omitted, the data will be provided inline.
	- fileType: the name of the type, such as "gltf". If omitted, the type is inferred from the
	  file extension.
	- keyframes: array of keyframes and channels that comprise the animation. If "file" is set, this
	  will be the list of keyframe animation names. If "file" is not set, each element of the array
	  has the following members:
	  - keyframeTimes: array of times for each keyframe in seconds.
	  - channels: array of channels associated with the keyframe times. Each element of the array
	    has the following members:
	    - node: the name of the node the channel applies to.
	    - component: the component to apply the value to. See the dsAnimationComponent enum for
	      values, removing the type prefix.
	    - interpolation: the interpolation method of the. See the dsAnimationInterpolation enum for
	      values, removing the type prefix.
	    - values: The values for the animation component as an array of float arrays. The inner
	      arrays have 3 elements for translation and scale, or 4 elements for a quaternion for
	      rotation. The outer array has either one element for each keyframe time for step and
	      linear interpolation, or three elements for each keyframe time for cubic interpolation.
	      The three cubic values are the in tangent, value, and out tangent, respectively.
	"""
	def readFloat(value, name):
		try:
			floatVal = float(value)
			return floatVal
		except:
			raise Exception('Invalid ' + name + ' value "' + str(value) + '".')

	def readTranslationScaleValues(value, name):
		if not isinstance(value, list) or len(value) != 3:
			raise Exception('KeyframeAnimation "' + name + '" must be an array of three floats.')

		return (readFloat(value[0], name), readFloat(value[1], name), readFloat(value[2], name))

	def readRotationValues(value):
		if not isinstance(value, list) or len(value) != 4:
			raise Exception('KeyframeAnimation "rotation" must be an array of four floats.')

		name = 'rotation'
		return (readFloat(value[0], name), readFloat(value[1], name), readFloat(value[2], name),
			readFloat(value[3], name))

	try:
		path = str(data.get('file', ''))
		keyframesData = data['keyframes']

		if not keyframesData:
			raise Exception('KeyframeAnimation contains no keyframes.')

		if path:
			fileType = str(data.get('fileType', ''))
			if not fileType:
				fileType = os.path.splitext(path)[1]
				if fileType:
					fileType = fileType[1:]
				if not fileType:
					raise Exception('KeyframeAnimation file has no known file type.')

			convertFunc = convertContext.keyframeAnimationTypeMap.get(fileType) \
				if hasattr(convertContext, 'keyframeAnimationTypeMap') else None
			if not convertFunc:
				raise Exception('KeyframeAnimation tree type "' + fileType +
					'" hasn\'t been registered.')

			if not isinstance(keyframesData, list):
				raise Exception('KeyframeAnimation "keyframes" must be a list of strings.')
			for keyframe in keyframesData:
				if not isinstance(keyframe, str):
					raise Exception('KeyframeAnimation "keyframes" must be a list of strings.')

			keyframes = convertFunc(convertContext, path, keyframesData)
		else:
			keyframes = []
			try:
				for keyframeData in keyframesData:
					keyframeTimesData = keyframeData['keyframeTimes']
					keyframeTimes = []
					try:
						for keyframeTimeData in keyframeTimesData:
							keyframeTimes.append(float(keyframeTimeData))
					except (TypeError, ValueError):
						raise Exception(
							'KeyframeAnimation "keyframeTimes" must be an array of floats.')

					channelsData = keyframeData['channels']
					channels = []
					try:
						for channelData in channelsData:
							node = str(channelData['node'])

							componentStr = str(channelData['component'])
							try:
								component = getattr(AnimationComponent, componentStr)
							except AttributeError:
								raise Exception('Invalid animation component "' + componentStr +
									'".')

							interpolationStr = str(channelData['interpolation'])
							try:
								interpolation = getattr(AnimationInterpolation, interpolationStr)
							except AttributeError:
								raise Exception('Invalid animation interpolation "' +
									interpolationStr + '".')

							valuesData = channelData['values']
							if not isinstance(valuesData):
								raise Exception(
									'KeyframeAnimation "values" must be an array of float arrays.')

							if interpolation == AnimationInterpolation.Cubic:
								if len(valuesData) != len(keyframeTimes)*3:
									raise Exception('KeyframeAnimation "values" must contain three '
										'elements for each keyframe time.')
							elif len(valuesData) != len(keyframeTimes)*3:
								raise Exception('KeyframeAnimation "values" must contain one '
									'element for each keyframe time.')

							values = []
							if component == AnimationComponent.Translation:
								for valueData in valuesData:
									values.append(
										readTranslationScaleValues(valueData, 'translation'))
							elif component == AnimationComponent.Scale:
								for valueData in valuesData:
									values.append(
										readTranslationScaleValues(valueData, 'scale'))
							elif component == AnimationComponent.Rotation:
								for valueData in valuesData:
									values.append(readRotationValues(valueData))

							channels.append(Channel(node, component, interpolation, values))
					except (TypeError, ValueError):
						raise Exception(
							'KeyframeAnimation "channels" must be an array of objects.')
					except KeyError as e:
						raise Exception('KeyframeAnimation "channels" doesn\'t contain element ' +
							str(e) + '.')

					keyframes.append(Keyframes(keyframeTimes, channels))
			except (TypeError, ValueError):
				raise Exception('KeyframeAnimation "keyframes" must be an array of objects.')
			except KeyError as e:
				raise Exception('KeyframeAnimation "keyframes" doesn\'t contain element ' +
					str(e) + '.')
	except (TypeError, ValueError):
		raise Exception('KeyframeAnimation data must be an object.')
	except KeyError as e:
		raise Exception('KeyframeAnimation data doesn\'t contain element ' + str(e) + '.')

	if not keyframes:
		raise Exception('KeyframeAnimation contains no keyframes.')

	builder = flatbuffers.Builder(0)

	keyframesOffsets = []
	for keyframe in keyframes:
		channelOffsets = []
		for channel in keyframe.channels:
			nodeOffset = builder.CreateString(channel.node)

			KeyframeAnimationChannel.StartValuesVector(builder, len(channel.values))
			for value in reversed(channel.values):
				builder.PrependUOffsetTRelative(CreateVector4f(builder, value[0], value[1],
					value[2], value[3] if len(value) == 4 else 0.0))
			valuesOffset = builder.EndVector()

			KeyframeAnimationChannel.Start(builder)
			KeyframeAnimationChannel.AddNode(builder, nodeOffset)
			KeyframeAnimationChannel.AddComponent(builder, channel.component)
			KeyframeAnimationChannel.AddInterpolation(builder, channel.interpolation)
			KeyframeAnimationChannel.AddValues(builder, valuesOffset)
			channelOffsets.append(KeyframeAnimationChannel.End(builder))

		AnimationKeyframes.StartChannelsVector(builder, len(channelOffsets))
		for offset in reversed(channelOffsets):
			builder.PrependUOffsetTRelative(offset)
		channelsOffset = builder.EndVector()

		AnimationKeyframes.StartKeyframeTimesVector(builder, len(keyframe.keyframeTimes))
		for time in reversed(keyframe.keyframeTimes):
			builder.PrependFloat32(time)
		keyframeTimesOffset = builder.EndVector()

		AnimationKeyframes.Start(builder)
		AnimationKeyframes.AddKeyframeTimes(builder, keyframeTimesOffset)
		AnimationKeyframes.AddChannels(builder, channelsOffset)
		keyframesOffsets.append(AnimationKeyframes.End(builder))

	KeyframeAnimation.StartKeyframesVector(builder, len(keyframesOffsets))
	for offset in reversed(keyframesOffsets):
		builder.PrependUOffsetTRelative(offset)
	keyframesOffset = builder.EndVector()

	KeyframeAnimation.Start(builder)
	KeyframeAnimation.AddKeyframes(builder, keyframesOffset)
	builder.Finish(KeyframeAnimation.End(builder))
	return builder.Output()
