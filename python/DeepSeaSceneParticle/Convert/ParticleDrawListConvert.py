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

from .. import ParticleDrawList

def convertParticleDrawList(convertContext, data):
	"""
	Converts a ParticleDrawList. The data map is expected to contain the following elements:
	- instanceData: optional list of instance data to include with the particle draw list. Each
	  element of the array has the following members:
	  - type: the name of the instance data type.
	  - Remaining members depend on the value of "type".
	- cullList: optional string name of the item list to handle culling. If unset no culling is
	  used.
	"""
	try:
		instanceDataInfo = data.get('instanceData', [])
		instanceData = []
		try:
			for info in instanceDataInfo:
				try:
					instanceData.append((str(info['type']), info))
				except KeyError as e:
					raise Exception(
						'ParticleDrawListList "instanceData" doesn\'t contain element "' + str(e) +
						'".')
		except (TypeError, ValueError):
			raise Exception('ParticleDrawList "instanceData" must be an array of objects.')

		cullList = str(data.get('cullList', ''))
	except KeyError as e:
		raise Exception('ParticleDrawList doesn\'t contain element ' + str(e) + '.')
	except (AttributeError, TypeError, ValueError):
		raise Exception('ParticleDrawList must be an object.')

	builder = flatbuffers.Builder(0)

	instanceDataOffsets = []
	for instanceType, instance in instanceData:
		instanceDataOffsets.append(convertContext.convertInstanceData(builder, instanceType,
			instance))

	if instanceDataOffsets:
		ParticleDrawList.StartInstanceDataVector(builder, len(instanceDataOffsets))
		for offset in reversed(instanceDataOffsets):
			builder.PrependUOffsetTRelative(offset)
		instanceDataOffset = builder.EndVector()
	else:
		instanceDataOffset = 0

	if cullList:
		cullListOffset = builder.CreateString(cullList)
	else:
		cullListOffset = 0

	ParticleDrawList.Start(builder)
	ParticleDrawList.AddInstanceData(builder, instanceDataOffset)
	ParticleDrawList.AddCullList(builder, cullListOffset)
	builder.Finish(ParticleDrawList.End(builder))
	return builder.Output()
