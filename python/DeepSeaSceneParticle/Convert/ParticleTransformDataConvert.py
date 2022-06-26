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
from .. import ParticleTransformData

def convertParticleTransformData(convertContext, data):
	"""
	Converts an ParticleTransformData. The data map is expected to contain the following elements:
	- variableGroupDesc: string name for the shader variable group to use.
	"""
	try:
		variableGroupDescName = str(data['variableGroupDesc'])
	except KeyError as e:
		raise Exception('ParticleTransformData doesn\'t contain element ' + str(e) + '.')
	except (TypeError, ValueError):
		raise Exception('ParticleTransformData must be an object.')

	builder = flatbuffers.Builder(0)
	variableGroupDescNameOffset = builder.CreateString(variableGroupDescName)
	ParticleTransformData.Start(builder)
	ParticleTransformData.AddVariableGroupDesc(builder, variableGroupDescNameOffset)
	builder.Finish(ParticleTransformData.End(builder))
	return builder.Output()
