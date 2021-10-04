# Copyright 2021 Aaron Barany
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
from .. import SceneShadowInstanceData

def convertShadowInstanceData(convertContext, data):
	"""
	Converts a ShadowInstanceData. The data map is expected to contain the following
	elements:
	- shadowManager: name of the shadow manager that contains the shadows to bind instance data.
	- shadows: name of the shadows within the shadow manager to bind instance data.
	- transformGroupName: the name of the shader variable group to bind as instance data for the
	  shadow transform.
	"""
	try:
		shadowManager = str(data['shadowManager'])
		shadows = str(data['shadows'])
		transformGroupName = str(data['transformGroupName'])
	except KeyError as e:
		raise Exception('ShadowInstanceData doesn\'t contain element ' + str(e) + '.')
	except (TypeError, ValueError):
		raise Exception('ShadowInstanceData must be an object.')

	builder = flatbuffers.Builder(0)
	shadowManagerOffset = builder.CreateString(shadowManager)
	shadowsOffset = builder.CreateString(shadows)
	transformGroupNameOffset = builder.CreateString(transformGroupName)

	SceneShadowInstanceData.Start(builder)
	SceneShadowInstanceData.AddShadowManager(builder, shadowManagerOffset)
	SceneShadowInstanceData.AddShadows(builder, shadowsOffset)
	SceneShadowInstanceData.AddTransformGroupName(builder, transformGroupNameOffset)
	builder.Finish(SceneShadowInstanceData.End(builder))
	return builder.Output()
