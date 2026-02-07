# Copyright 2020-2026 Aaron Barany
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
from .. import SceneLightSetPrepare

def convertLightSetPrepare(convertContext, data, inputDir):
	"""
	Converts a LightSetPrepare. The data map is expected to contain the following elements:
	- lightSet: name of the light set to prepare.
	- intensityThreshold: the threshold below which the light is considered out of view. If unset
	  this will use the default.
	"""
	try:
		lightSet = str(data['lightSet'])

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

	lightSetOffset = builder.CreateString(lightSet)

	SceneLightSetPrepare.Start(builder)
	SceneLightSetPrepare.AddLightSet(builder, lightSetOffset)
	SceneLightSetPrepare.AddIntensityThreshold(builder, intensityThreshold)
	builder.Finish(SceneLightSetPrepare.End(builder))
	return builder.Output()
