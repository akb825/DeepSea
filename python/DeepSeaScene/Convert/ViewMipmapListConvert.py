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
from .. import ViewMipmapList

def convertViewMipmapList(convertContext, data):
	"""
	Converts a ViewMipmapList. The data map is expected to contain the following elements:
	- textures: list of the textures in the view to generate mipmaps for.
	"""
	try:
		textures = data['textures']
		if not isinstance(textures, list):
			raise Exception('FullScreenResolve textures must be a list of strings.')
	except KeyError as e:
		raise Exception('FullScreenResolve doesn\'t contain element ' + str(e) + '.')
	except (AttributeError, TypeError, ValueError):
		raise Exception('FullScreenResolve must be an object.')

	builder = flatbuffers.Builder(0)
	textureOffsets = []
	for texture in textures:
		textureOffsets.append(builder.CreateString(texture))

	ViewMipmapList.StartTexturesVector(builder, len(textures))
	for offset in reversed(textureOffsets):
		builder.PrependUOffsetTRelative(offset)
	texturesOffset = builder.EndVector()

	ViewMipmapList.Start(builder)
	ViewMipmapList.AddTextures(builder, texturesOffset)
	builder.Finish(ViewMipmapList.End(builder))
	return builder.Output()
