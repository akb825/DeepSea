# Copyright 2020-2021 Aaron Barany
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
from .. import LightSetPrepare

def convertLightSetPrepare(convertContext, data):
	"""
	Converts a LightSetPrepare. The data map is expected to contain the following elements:
	- lightSets: array of light set names to prepare.
	- intensityThreshold: the threshold below which the light is considered out of view. If unset
	  this will use the default.
	"""
	try:
		lightSets = data['lightSets']
		if not isinstance(lightSets, list):
			raise Exception('LightSetPrepare "lightSets" must be an array of strings.')

		try:
			intensityThresholdStr = data.get('intensityThreshold', 0.0)
			intensityThreshold = float(intensityThresholdStr)
		except:
			raise Exception('Invalid intensityThreshold float value "' +
				str(intensityThresholdStr) + '".')
	except KeyError as e:
		raise Exception('LightSetPrepare doesn\'t contain element ' + str(e) + '.')
	except (AttributeError, TypeError, ValueError):
		raise Exception('LightSetPrepare must be an object.')

	builder = flatbuffers.Builder(0)

	lightSetOffsets = []
	for lightSet in lightSets:
		lightSetOffsets.append(builder.CreateString(lightSet))

	LightSetPrepare.StartLightSetsVector(builder, len(lightSetOffsets))
	for offset in reversed(lightSetOffsets):
		builder.PrependUOffsetTRelative(offset)
	lightSetsOffset = builder.EndVector()

	LightSetPrepare.Start(builder)
	LightSetPrepare.AddLightSets(builder, lightSetsOffset)
	LightSetPrepare.AddIntensityThreshold(builder, intensityThreshold)
	builder.Finish(LightSetPrepare.End(builder))
	return builder.Output()
