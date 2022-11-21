# Copyright 2022 Aaron Barany
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
from TestParticles import LightFlicker

def convertLightFlicker(convertContext, data):
	try:
		timeRangeData = data['timeRange']
		try:
			if len(timeRangeData) != 2:
				raise Exception()

			timeRange = [float(f) for f in timeRangeData]
		except:
			raise Exception('LightFlicker "timeRange" must be an array of 2 floats.')

		intensityRangeData = data['intensityRange']
		try:
			if len(intensityRangeData) != 2:
				raise Exception()

			intensityRange = [float(f) for f in intensityRangeData]
		except:
			raise Exception('LightFlicker "intensityRange" must be an array of 2 floats.')
	except KeyError as e:
		raise Exception('LightFlicker doesn\'t contain element "' + str(e) + '".')
	except (TypeError, ValueError):
		raise Exception('LightFlicker must be an object.')

	builder = flatbuffers.Builder(0)
	LightFlicker.Start(builder)
	LightFlicker.AddMinTime(builder, timeRange[0])
	LightFlicker.AddMaxTime(builder, timeRange[1])
	LightFlicker.AddMinIntensity(builder, intensityRange[0])
	LightFlicker.AddMaxIntensity(builder, intensityRange[1])
	builder.Finish(LightFlicker.End(builder))
	return builder.Output()

def deepSeaSceneExtension(convertContext):
	convertContext.addItemListType('LightFlicker', convertLightFlicker)
