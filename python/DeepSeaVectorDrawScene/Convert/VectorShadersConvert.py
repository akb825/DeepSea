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

from .. import MaterialElement
from .. import VectorShaders

from DeepSeaScene.Convert.FileOrDataConvert import convertFileOrData, readDataOrPath
from DeepSeaScene.MaterialBinding import MaterialBinding
from DeepSeaScene.MaterialType import MaterialType
from DeepSeaScene import VersionedShaderModule

class Object:
	pass

def convertVectorShaders(convertContext, data):
	"""
	Converts vector shaders used in a scene. The data map is expected to contain the following
	elements:
	- modules: array of versioned shader modules. The appropriate model based on the graphics API
	  version being used will be chosen at runtime. Each element of the array has the following
	  members:
	  - version: the version of the shader as a standard config. (e.g. glsl-4.1, spirv-1.0)
	  - module: path to the shader module or base64 encoded data prefixed with "base64:". The
	    module is expected to have been compiled with Modular Shader Language (MSL).
	  - output: the path to the location to copy the shader module to. This can be omitted to
	    embed the shader module directly.
	  - outputRelativeDir: the directory relative to output path. This will be removed from the
	    path before adding the reference.
	  - resourceType: the resource type. See the dsFileResourceType for values, removing the type
	    prefix. Defaults to "Embedded".
	- extraElements: list of extra meterial elements to add for the material description. Each
	  element of the array has the following members:
	  - name: the name of the element.
	  - type: the type of the element. See dsMaterialType enum for values, removing the type prefix.
	  - count: the number of array elements. If 0 or omitted, this is not an array.
	  - binding: the binding type for the element. See the dsMaterialBinding enum for values,
	    removing the type prefix. This is only used for texture, image, buffer, and shader variable
	    group types.
	  - shaderVariableGroupDesc: the name of the shader variable group description when the type
	    is a shader variable group.
	- materialDesc: the name of the material description to register. This can be referenced by
	  other objects, such as creating materials used for drawing vector images.
	- fillColor: the name of the shader for filling with a solid color. Defaults to
	  "dsVectorFillColor".
	- fillLinearGradient: the name of the shader for filling with a linear gradient. Defaults to
	  "dsVectorFillLinearGradient".
	- fillRadialGradient: the name of the shader for filling with a radial gradient. Defaults to
	  "dsVectorFillRadialGradient".
	- line: the name of the shader for a line with a color or gradient. Defaults to
	  "dsVectorLine".
	- image: the name of the shader for a texture applied as an image. Defaults to "dsVectorImage".
	- textColor: name of the shader for standard single-color text. Defaults to "dsVectorTextColor".
	- textColorOutline: name of the shader for standard single-color text with a single-colored
	  outline. Defaults to "dsVectorTextColorOutline".
	- textGradient: name of the shader for text using a gradient. Defaults to
	  "dsVectorTextGradient".
	- textGradientOutline: name of the shader for text with an outline using a gradient. Defaults to
	  "dsVectorTextGradientOutline".
	"""
	def createOptionalString(builder, string):
		if string:
			return builder.CreateString(string)
		else:
			return 0

	builder = flatbuffers.Builder(0)

	try:
		modules = data['modules']
		versionedModules = []
		try:
			for versionedModuleData in modules:
				version = str(versionedModuleData['version'])
				moduleStr = str(versionedModuleData['module'])
				try:
					modulePath, moduleContents = readDataOrPath(moduleStr)
				except TypeError:
					raise Exception(
						'VectorShaders shader module "module" uses incorrect base64 encoding.')

				dataType, dataOffset = convertFileOrData(builder, modulePath,
					moduleContents, versionedModuleData.get('output'),
					versionedModuleData.get('outputRelativeDir'),
					versionedModuleData.get('resourceType'))
				versionedModules.append((version, dataType, dataOffset))
		except KeyError as e:
			raise Exception('Versioned shader module data doesn\'t contain element ' +
				str(e) + '.')
		except (TypeError, ValueError):
			raise Exception('Versioned shader module list must be an array of objects.')

		extraElementsData = data.get('extraElements', [])
		extraElements = []
		try:
			for elementData in extraElementsData:
				element = Object()
				element.name = str(elementData['name'])

				typeStr = str(elementData['type'])
				try:
					element.type = getattr(MaterialType, typeStr)
				except AttributeError:
					raise Exception('Invalid material type "' + typeStr + '".')

				countValue = elementData.get('count', 0)
				try:
					element.count = int(countValue)
					if element.count < 0:
						raise Exception() # Common error handling in except block.
				except:
					raise Exception(
						'Invalid vector shader element count "' + str(countValue) + '".')

				bindingStr = str(elementData['binding'])
				try:
					element.binding = getattr(MaterialBinding, bindingStr)
				except AttributeError:
					raise Exception('Invalid material binding "' + bindingStr + '".')

				element.shaderVariableGroupDesc = str(
					elementData.get('shaderVariableGroupDesc', ''))
				extraElements.append(element)
		except KeyError as e:
			raise Exception('Versioned shader module data doesn\'t contain element ' +
				str(e) + '.')
		except (TypeError, ValueError):
			raise Exception('Versioned shader module list must be an array of objects.')

		materialDescName = str(data['materialDesc'])

		fillColor = str(data.get('fillColor', ''))
		fillLinearGradient = str(data.get('fillLinearGradient', ''))
		fillRadialGradient = str(data.get('fillRadialGradient', ''))
		line = str(data.get('line', ''))
		image = str(data.get('image', ''))
		textColor = str(data.get('textColor', ''))
		textColorOutline = str(data.get('textColorOutline', ''))
		textGradient = str(data.get('textGradient', ''))
		textGradientOutline = str(data.get('textGradientOutline', ''))
	except KeyError as e:
		raise Exception('VectorShaders doesn\'t contain element ' + str(e) + '.')
	except (AttributeError, TypeError, ValueError):
		raise Exception('VectorShaders must be an object.')

	modulesOffsets = []
	for version, dataType, dataOffset in versionedModules:
		versionOffset = builder.CreateString(version)

		VersionedShaderModule.Start(builder)
		VersionedShaderModule.AddVersion(builder, versionOffset)
		VersionedShaderModule.AddDataType(builder, dataType)
		VersionedShaderModule.AddData(builder, dataOffset)
		modulesOffsets.append(VersionedShaderModule.End(builder))

	VectorShaders.StartModulesVector(builder, len(modulesOffsets))
	for offset in reversed(modulesOffsets):
		builder.PrependUOffsetTRelative(offset)
	modulesOffset = builder.EndVector()

	extraElementsOffsets = []
	for element in extraElements:
		elementNameOffset = builder.CreateString(element.name)
		shaderVariableGroupDescOffset = createOptionalString(builder,
			element.shaderVariableGroupDesc)

		MaterialElement.Start(builder)
		MaterialElement.AddName(builder, elementNameOffset)
		MaterialElement.AddType(builder, element.type)
		MaterialElement.AddCount(builder, element.count)
		MaterialElement.AddBinding(builder, element.binding)
		MaterialElement.AddShaderVariableGroupDesc(builder, shaderVariableGroupDescOffset)
		extraElementsOffsets.append(MaterialElement.End(builder))

	VectorShaders.StartExtraElementsVector(builder, len(extraElementsOffsets))
	for offset in reversed(extraElementsOffsets):
		builder.PrependUOffsetTRelative(offset)
	extraElementsOffset = builder.EndVector()

	materialDescNameOffset = builder.CreateString(materialDescName)
	fillColorOffset = createOptionalString(builder, fillColor)
	fillLinearGradientOffset = createOptionalString(builder, fillLinearGradient)
	fillRadialGradientOffset = createOptionalString(builder, fillRadialGradient)
	lineOffset = createOptionalString(builder, line)
	imageOffset = createOptionalString(builder, image)
	textColorOffset = createOptionalString(builder, textColor)
	textColorOutlineOffset = createOptionalString(builder, textColorOutline)
	textGradientOffset = createOptionalString(builder, textGradient)
	textGradientOutlineOffset = createOptionalString(builder, textGradientOutline)

	VectorShaders.Start(builder)
	VectorShaders.AddModules(builder, modulesOffset)
	VectorShaders.AddExtraElements(builder, extraElementsOffset)
	VectorShaders.AddMaterialDesc(builder, materialDescNameOffset)
	VectorShaders.AddFillColor(builder, fillColorOffset)
	VectorShaders.AddFillLinearGradient(builder, fillLinearGradientOffset)
	VectorShaders.AddFillRadialGradient(builder, fillRadialGradientOffset)
	VectorShaders.AddLine(builder, lineOffset)
	VectorShaders.AddImage(builder, imageOffset)
	VectorShaders.AddTextColor(builder, textColorOffset)
	VectorShaders.AddTextColorOutline(builder, textColorOutlineOffset)
	VectorShaders.AddTextGradient(builder, textGradientOffset)
	VectorShaders.AddTextGradientOutline(builder, textGradientOutlineOffset)
	builder.Finish(VectorShaders.End(builder))
	return builder.Output()
