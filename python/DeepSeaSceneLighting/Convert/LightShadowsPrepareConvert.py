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
from .. import LightShadowsPrepare

def convertLightShadowsPrepare(convertContext, data):
	"""
	Converts a LightShadowsPrepare. The data map is expected to contain the following elements:
	- lightShadows: name of the light shadows instance to prepare.
	- transformGroup: name of the transform group. This may be omitted if only used for instance
	  variables. (e.g. deferred lighting)
	"""
	try:
		lightShadows = str(data['lightShadows'])
		transformGroup = str(data.get('transformGroup', ''))
	except KeyError as e:
		raise Exception('LightShadowsPrepare doesn\'t contain element ' + str(e) + '.')
	except (AttributeError, TypeError, ValueError):
		raise Exception('LightShadowsPrepare must be an object.')

	builder = flatbuffers.Builder(0)

	lightShadowsOffset = builder.CreateString(lightShadows)
	if transformGroup:
		transformGroupOffset = builder.CreateString(transformGroup)
	else:
		transformGroupOffset = 0

	LightShadowsPrepare.Start(builder)
	LightShadowsPrepare.AddLightShadows(builder, lightShadowsOffset)
	LightShadowsPrepare.AddTransformGroup(builder, transformGroupOffset)
	builder.Finish(LightShadowsPrepare.End(builder))
	return builder.Output()
