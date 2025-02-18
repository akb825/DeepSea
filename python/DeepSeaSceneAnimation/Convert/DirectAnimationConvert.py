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

from DeepSeaAnimation import DirectAnimationChannel
from DeepSeaAnimation import DirectAnimation
from DeepSeaAnimation.AnimationComponent import AnimationComponent
from DeepSeaAnimation.Convert.Quaternion import eulerToQuaternion
from DeepSeaAnimation.Vector4f import CreateVector4f

class Object:
	pass

def convertDirectAnimation(convertContext, data, outputDir):
	"""
	Converts a direct animation for a scene. The data map is expected to contain the following
	elements:
	- channels: array of channels for the animation. Each element of the array has the following
	  members:
	  - node: the name of the node to apply the value to.
	  - component: the component to apply the value to. See the dsAnimationComponent enum for
	    values, removing the type prefix.
	  - value: the value for the channel as an array of three floats based on the component:
	    - "Translation": x, y, z offset.
	    - "Scale": x, y, z scale factors.
	    - "Rotation": x, y, z Euler angles in degrees.
	"""
	def readFloat(value, name):
		try:
			floatVal = float(value)
			return floatVal
		except:
			raise Exception('Invalid ' + name + ' value "' + str(value) + '".')

	try:
		channelsData = data['channels']
		channels = []
		try:
			for channelData in channelsData:
				try:
					channel = Object()
					channel.node = str(channelData['node'])

					componentStr = str(channelData['component'])
					try:
						channel.component = getattr(AnimationComponent, componentStr)
					except AttributeError:
						raise Exception('Invalid animation component "' + componentStr + '".')

					valueList = channelData['value']
					if not isinstance(valueList, list) or len(valueList) != 3:
						raise Exception('DirectAnimation "value" must be an array of three floats.')

					x = readFloat(valueList[0], 'value x')
					y = readFloat(valueList[1], 'value y')
					z = readFloat(valueList[2], 'value z')
					if channel.component == AnimationComponent.Rotation:
						channel.value = eulerToQuaternion(x, y, z)
					else:
						channel.value = (x, y, z, 0.0)
				except KeyError as e:
					raise Exception('LightSet light doesn\'t contain element ' + str(e) + '.')

				channels.append(channel)
		except (TypeError, ValueError):
			raise Exception('DirectAnimation "channels" must be an array of objects.')
	except KeyError as e:
		raise Exception('DirectAnimation doesn\'t contain element ' + str(e) + '.')
	except (AttributeError, TypeError, ValueError):
		raise Exception('DirectAnimation must be an object.')

	if not channels:
		raise Exception('DirectAnimation must contain at least one channel.')

	builder = flatbuffers.Builder(0)

	channelOffsets = []
	for channel in channels:
		nodeOffset = builder.CreateString(channel.node)

		DirectAnimationChannel.Start(builder)
		DirectAnimationChannel.AddNode(builder, nodeOffset)
		DirectAnimationChannel.AddComponent(builder, channel.component)
		DirectAnimationChannel.AddValue(builder, CreateVector4f(builder, *channel.value))
		channelOffsets.append(DirectAnimationChannel.End(builder))

	DirectAnimation.StartChannelsVector(builder, len(channelOffsets))
	for offset in reversed(channelOffsets):
		builder.PrependUOffsetTRelative(offset)
	channelsOffset = builder.EndVector()

	DirectAnimation.Start(builder)
	DirectAnimation.AddChannels(builder, channelsOffset)
	builder.Finish(DirectAnimation.End(builder))
	return builder.Output()
