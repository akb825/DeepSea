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

from .. import ParticleNode

def convertParticleNode(convertContext, data, outputDir):
	"""
	Converts a ParticleNode. The data map is expected to contain the following elements:
	- particleEmitterFactory: the name of the factory to create particle emitters with.
	- itemLists: array of item list names to add the node to.
	"""
	try:
		particleEmitterFactory = str(data['particleEmitterFactory'])
		itemLists = data.get('itemLists')
	except (TypeError, ValueError):
		raise Exception('ParticleNode data must be an object.')
	except KeyError as e:
		raise Exception('ParticleNode data doesn\'t contain element ' + str(e) + '.')

	builder = flatbuffers.Builder(0)
	particleEmitterFactoryOffset = builder.CreateString(particleEmitterFactory)

	if itemLists:
		itemListOffsets = []
		try:
			for item in itemLists:
				itemListOffsets.append(builder.CreateString(str(item)))
		except (TypeError, ValueError):
			raise Exception('ParticleNode "itemLists" must be an array of strings.')

		ParticleNode.StartItemListsVector(builder, len(itemListOffsets))
		for offset in reversed(itemListOffsets):
			builder.PrependUOffsetTRelative(offset)
		itemListsOffset = builder.EndVector()
	else:
		itemListsOffset = 0

	ParticleNode.Start(builder)
	ParticleNode.AddParticleEmitterFactory(builder, particleEmitterFactoryOffset)
	ParticleNode.AddItemLists(builder, itemListsOffset)
	builder.Finish(ParticleNode.End(builder))
	return builder.Output()
