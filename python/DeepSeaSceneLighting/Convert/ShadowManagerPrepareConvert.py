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
from .. import SceneShadowManagerPrepare

def convertShadowManagerPrepare(convertContext, data):
	"""
	Converts a ShadowManagerPrepare. The data map is expected to contain the following elements:
	- shadowManager: name of the shadow manager instance to prepare.
	"""
	try:
		shadowManager = str(data['shadowManager'])
	except KeyError as e:
		raise Exception('ShadowManagerPrepare doesn\'t contain element ' + str(e) + '.')
	except (AttributeError, TypeError, ValueError):
		raise Exception('ShadowManagerPrepare must be an object.')

	builder = flatbuffers.Builder(0)

	shadowManagerOffset = builder.CreateString(shadowManager)

	SceneShadowManagerPrepare.Start(builder)
	SceneShadowManagerPrepare.AddShadowManager(builder, shadowManagerOffset)
	builder.Finish(SceneShadowManagerPrepare.End(builder))
	return builder.Output()
