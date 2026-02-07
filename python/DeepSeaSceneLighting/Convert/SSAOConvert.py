# Copyright 2021-2026 Aaron Barany
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
from .. import SceneSSAO

def convertSSAO(convertContext, data, inputDir):
	"""
	Converts a SceneSSAO. The data map is expected to contain the following elements:
	shader: the shader to compute the ambient occlusion with.
	material: the material to use with the shader.
	"""
	try:
		shader = str(data['shader'])
		material = str(data['material'])
	except KeyError as e:
		raise Exception('SSAO doesn\'t contain element ' + str(e) + '.')
	except (AttributeError, TypeError, ValueError):
		raise Exception('SSAO must be an object.')

	builder = flatbuffers.Builder(0)

	shaderOffset = builder.CreateString(shader)
	materialOffset = builder.CreateString(material)

	SceneSSAO.Start(builder)
	SceneSSAO.AddShader(builder, shaderOffset)
	SceneSSAO.AddMaterial(builder, materialOffset)
	builder.Finish(SceneSSAO.End(builder))
	return builder.Output()
