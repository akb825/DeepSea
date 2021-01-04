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
from DeepSeaScene.Vector3f import *
from TestScene.LightData import *

def convertLightData(convertContext, data):
	try:
		variableGroupDescName = str(data['variableGroupDesc'])

		directionInfo = data['direction']
		try:
			if len(directionInfo) != 3:
				raise Exception()

			direction = [float(f) for f in directionInfo]
		except:
			raise Exception('LightData "direction" must be an array of 3 floats.')

		colorInfo = data['color']
		try:
			if len(colorInfo) != 3:
				raise Exception()

			color = [float(f) for f in colorInfo]
		except:
			raise Exception('LightData "color" must be an array of 3 floats.')

		ambientInfo = data['ambient']
		try:
			if len(ambientInfo) != 3:
				raise Exception()

			ambient = [float(f) for f in ambientInfo]
		except:
			raise Exception('LightData "ambient" must be an array of 3 floats.')
	except KeyError as e:
		raise Exception('LightData doesn\'t contain element "' + str(e) + '".')
	except (TypeError, ValueError):
		raise Exception('LightData must be an object.')

	builder = flatbuffers.Builder(0)
	variableGroupDescNameOffset = builder.CreateString(variableGroupDescName)
	LightDataStart(builder)
	LightDataAddVariableGroupDesc(builder, variableGroupDescNameOffset)
	LightDataAddDirection(builder, CreateVector3f(builder, *direction))
	LightDataAddColor(builder, CreateVector3f(builder, *color))
	LightDataAddAmbient(builder, CreateVector3f(builder, *ambient))
	builder.Finish(LightDataEnd(builder))
	return builder.Output()

def deepSeaSceneExtension(convertContext):
	convertContext.addGlobalDataType('LightData', convertLightData)
