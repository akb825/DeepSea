# Copyright 2023-2026 Aaron Barany
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

from .. import SkinningData

def convertSkinningData(convertContext, data, inputDir):
	"""
	Converts a SkinningData. The data map is expected to contain the following elements:
	- textureInfoDesc: the name of the shader variable group description when texture buffer or
	  texture skinning is used. This is ignored when buffers are used, which can be used in
	  conjunction with conditional loading of materials to support both buffer and texture/texture
	  buffer skinning with the same configuration.
	"""
	try:
		textureInfoDesc = str(data.get('textureInfoDesc', ''))
	except (TypeError, ValueError):
		raise Exception('SkinningData data must be an object.')
	except KeyError as e:
		raise Exception('SkinningData data doesn\'t contain element ' + str(e) + '.')

	builder = flatbuffers.Builder(0)

	if textureInfoDesc:
		textureInfoDescOffset = builder.CreateString(textureInfoDesc)
	else:
		textureInfoDescOffset = 0

	SkinningData.Start(builder)
	SkinningData.AddTextureInfoDesc(builder, textureInfoDescOffset)
	builder.Finish(SkinningData.End(builder))
	return builder.Output()
