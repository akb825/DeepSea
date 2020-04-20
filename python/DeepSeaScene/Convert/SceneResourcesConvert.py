# Copyright 2020 Aaron Barany
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

import base64
import os
import re
import struct
import subprocess
import sys
from tempfile import NamedTemporaryFile

import flatbuffers
from .FileOrDataConvert import convertFileOrData
from ..Buffer import *
from ..BufferMaterialData import *
from ..DrawGeometry import *
from ..FormatDecoration import *
from ..IndexBuffer import *
from ..MaterialType import *
from ..NamedMaterialData import *
from ..SceneNode import *
from ..SceneResources import *
from ..Shader import *
from ..ShaderData import *
from ..ShaderModule import *
from ..ShaderVariableGroupDesc import *
from ..Texture import *
from ..TextureBufferMaterialData import *
from ..TextureDim import *
from ..TextureFormat import *
from ..TextureInfo import *
from ..VertexAttribute import *
from ..VariableData import *
from ..VariableElement import *
from ..VertexBuffer import *
from ..VertexElementFormat import *
from ..VertexFormat import *

memoryHintsEnum = {
	'GPUOnly': 0x1,
	'Static': 0x2,
	'Dynamic': 0x4,
	'Stream': 0x8,
	'Draw': 0x10,
	'Read': 0x20,
	'Persistent': 0x40,
	'Coherent': 0x80,
	'Synchronize': 0x100
}

bufferUsageEnum = {
	'Index': 0x1,
	'Vertex': 0x2,
	'IndirectDraw': 0x4,
	'IndirectDispatch': 0x8,
	'UniformBlock': 0x10,
	'UniformBuffer': 0x20,
	'Texture': 0x40,
	'Image': 0x80,
	'CopyFrom': 0x100,
	'CopyTo': 0x200
}

textureUsageEnum = {
	'Texture': 0x1,
	'Image': 0x2,
	'SubpassInput': 0x4,
	'CopyFrom': 0x8,
	'CopyTo': 0x10,
	'OffscreenContinue': 0x20
}

modelVertexAttribEnum = {
	'Position': 0,
	'Position0': 0,
	'Position1': 1,
	'Normal': 2,
	'Color': 3,
	'Color0': 3,
	'Color1': 4,
	'FogCoord': 5,
	'Tangent': 6,
	'Bitangent': 7,
	'TexCoord0': 8,
	'TexCoord1': 9,
	'TexCoord2': 10,
	'TexCoord3': 11,
	'TexCoord4': 12,
	'TexCoord5': 13,
	'TexCoord6': 14,
	'TexCoord7': 15,
	'BlendIndices': 14,
	'BlendWeights': 15
}

def readData(dataStr):
	if dataStr.startswith('base64:'):
		dataPath = None
		dataContents = base64.b64decode(dataStr[7:])
	else:
		dataPath = dataStr
		with open(dataStr, 'rb') as stream:
			dataContents = stream.read()
	return dataPath, dataContents

def readVertexAttrib(attrib):
	if not isinstance(attrib, int):
		attribStr = str(attrib)
		attrib = modelVertexAttribEnum.get(attribStr)
		if attrib is None:
			raise Exception('Invalid vertex attribute "' + attribStr + '".')
	return attrib

def convertSceneResourcesBuffers(builder, convertContext, data):
	bufferOffsets = []
	try:
		for bufferData in data:
			try:
				name = str(bufferData['name'])

				try:
					usage = 0
					for usageEnum in bufferData['usage']:
						try:
							usage |= bufferUsageEnum[usageEnum]
						except KeyError as e:
							raise Exception('Invalid dsGfxBufferUsage enum value "' + str(e) + '".')
					if usage == 0:
						raise Exception('SceneResources buffer "usage" must not be empty.')
				except (ValueError, KeyError):
					raise Exception('SceneResources buffer "usage" must be an array of valid '
						'dsGfxBufferUsage enum values.')

				try:
					memoryHints = 0
					for memoryEnum in bufferData['memoryHints']:
						try:
							memoryHints |= memoryHintsEnum[memoryEnum]
						except KeyError as e:
							raise Exception('Invalid dsMemoryUsage enum value "' + str(e) + '".')
					if memoryHints == 0:
						raise Exception('SceneResources buffer "memoryHints" must not be empty.')
				except (ValueError, KeyError):
					raise Exception('SceneResources buffer "memoryHints" must be an array of valid '
						'dsGfxMemory enum values.')

				if 'data' in bufferData:
					dataStr = str(bufferData['data'])
					try:
						dataPath, dataContents = readData(dataStr)
					except TypeError:
						raise Exception(
							'SceneResources buffer "data" uses incorrect base64 encoding.')
					bufferSize = len(bufferData)
				else:
					bufferContents = None
					try:
						bufferSize = int(bufferData['size'])
					except ValueError:
						raise Exception('SceneResources buffer "size" must be an integer.')
					except KeyError:
						raise Exception('SceneResources buffer data must contain either "data" or '
							'"size" element.')
			except KeyError as e:
				raise Exception(
					'SceneResources buffer data doesn\'t contain element "' + str(e) + '".')

			nameOffset = builder.CreateString(name)
			dataType, dataOffset = convertFileOrData(builder, dataPath, dataContents,
				bufferData.get('output'), bufferData.get('outputRelativeDir'),
				bufferData.get('resourceType'))

			BufferStart(builder)
			BufferAddName(builder, nameOffset)
			BufferAddUsage(builder, usage)
			BufferAddMemoryHints(builder, memoryHints)
			BufferAddSize(builder, bufferSize)
			BufferAddDataType(builder, dataType)
			BufferAddData(builder, dataOffset)
			bufferOffsets.append(BufferEnd(builder))
	except (TypeError, ValueError):
		raise Exception('SceneResources "buffers" must be an array of objects.')

	SceneResourcesStartBuffersVector(builder, len(bufferOffsets))
	for offset in reversed(bufferOffsets):
		builder.PrependUOffsetTRelative(offset)
	return builder.EndVector(len(bufferOffsets))

def convertSceneResourcesTextures(builder, convertContext, data):
	def readInt(value, name, minVal):
		try:
			intVal = int(value)
			if intVal < minVal:
				raise Exception() # Common error handling in except block.
			return intVal
		except:
			raise Exception('Invalid texture ' + name + ' "' + str(value) + '".')

	def cleanup(tempFiles):
		for tempFile in tempFiles:
			if os.path.isfile(tempFile):
				os.remove(tempFile)

	textureOffsets = []
	try:
		for textureData in data:
			try:
				name = str(textureData['name'])

				try:
					usage = 0
					usageArray = textureData.get('usage', ['Texture'])
					for usageEnum in usageArray:
						try:
							usage |= textureUsageEnum[usageEnum]
						except KeyError as e:
							raise Exception('Invalid dsTextureUsage enum value "' + str(e) + '".')
					if usage == 0:
						raise Exception('SceneResources texture "usage" must not be empty.')
				except (ValueError, KeyError):
					raise Exception('SceneResources texture "usage" must be an array of valid '
						'dsTexgtureUsage enum values.')

				try:
					memoryHints = 0
					memoryHintsArray = textureData.get('memoryHints', ['GPUOnly', 'Static'])
					for memoryEnum in memoryHintsArray:
						try:
							memoryHints |= memoryHintsEnum[memoryEnum]
						except KeyError as e:
							raise Exception('Invalid dsMemoryUsage enum value "' + str(e) + '".')
					if memoryHints == 0:
						raise Exception('SceneResources texture "memoryHints" must not be empty.')
				except (ValueError, KeyError):
					raise Exception('SceneResources texture "memoryHints" must be an array of valid '
						'dsGfxMemory enum values.')

				if 'path' in textureData:
					path = str(textureData['path'])
					paths = None
					expectedDepth = 0
				elif 'pathArray' in textureData:
					path = None
					paths = textureData['pathArray']
					expectedDepth = len(paths)
				else:
					path = None
					paths = None
					expectedDepth = 0

				baseTextureInfo = textureData.get('textureData')
				if baseTextureInfo:
					textureInfo = object()
					try:
						formatStr = str(baseTextureInfo['format'])
						try:
							textureInfo.format = getattr(TextureFormat, formatStr)
						except AttributeError:
							raise Exception('Invalid texture format "' + formatStr + '".')

						decorationStr = str(baseTextureInfo['decoration'])
						try:
							textureInfo.decoration = getattr(FormatDecoration, decorationStr)
						except AttributeError:
							raise Exception(
								'Invalid texture format decoration "' + decorationStr + '".')

						dimensionStr = str(baseTextureInfo.get('dimension', 'Dim2D'))
						try:
							textureInfo.dimension = getattr(TextureDim, dimensionStr)
						except AttributeError:
							raise Exception('Invalid texture dimension "' + dimensionStr + '".')
						if textureInfo.dimension == TextureDim.DimCube:
							if path or (paths and len(paths) != max(textureInfo.depth, 1)*6):
								raise Exception('Unexpected number of images for cube map texture.')
							expectedDepth = expectedDepth/6

						validConvertValues = (None, 'nextpo2', 'nearestpo2')
						widthStr = str(baseTextureInfo.get('width'))
						if (path or paths) and widthStr in validConvertValues:
							textureInfo.width = widthStr
						elif widthStr is None:
							raise KeyError('width')
						else:
							textureInfo.width = readInt(widthStr, 'width', 1)

						heightStr = str(baseTextureInfo.get('height'))
						if (path or paths) and heightStr in validConvertValues:
							textureInfo.height = heightStr
						elif heightStr is None:
							raise KeyError('height')
						else:
							textureInfo.height = readInt(heightStr, 'height', 1)

						if textureInfo.width is None and textureInfo.height is None:
							raise Exception('If texture width or height is set, both must be set.')

						depthStr = str(baseTextureInfo.get('depth', expectedDepth))
						textureInfo.depth = readInt(heightStr, 'depth', 0)
						if (path or paths) and textureInfo.depth != expectedDepth:
							raise Exception(
								"Unexpected texture depth for number of image paths.")

						textureInfo.mipLevels = readInt(baseTextureInfo.get('mipLevels', 1),
							'mipLevels', 1)

						textureInfo.quality = baseTextureInfo.get('quality', 'normal')
						if str(textureInfo.quality).lower() not in \
								('lowest', 'low', 'normal', 'high', 'highest'):
							raise Exception(
								'Invalid texture quality "' + textureInfo.quality + '".')

						normalmapStr = str(aseTextureInfo.get('normalmap', 0.0))
						try:
							textureInfo.normalmap = float(normalmapStr)
						except:
							raise Exception(
								'Invalid texture normalmap height "' + normalmapStr + '".')

						textureInfo.swizzle = baseTextureInfo.get('swizzle')
						if textureInfo.swizzle and \
								not re.match('^[RGBAXrgbax]{4}$', textureInfo.swizzle):
							raise Exception(
								'Invalid texture swizzle "' + textureInfo.swizzle + '".')

						textureInfo.rotate = baseTextureInfo.get('rotate', '0')
						if textureInfo.rotate not in \
								('-270', '-180', '-90', '0', '90', '180', '270'):
							raise Exception(
								'Invalid texture rotation "' + textureInfo.rotate + '".')

						textureInfo.alpha = baseTextureInfo.get('alpha')
						if textureInfo.alpha and str(textureInfo.alpha).lower() not in \
								('none', 'standard', 'pre-multiplied', 'encoded'):
							raise Exception(
								'Invalid texture alpha type "' + textureInfo.rotate + '".')

						transformArray = baseTextureInfo.get('transforms', [])
						textureInfo.transforms = []
						try:
							for transform in transformArray:
								transformLower = transform.lower()
								if transformLower not in \
										('flipx', 'flipy', 'srgb', 'grayscale', 'pre-multiply'):
									raise Exception(
										'Invalid texture transform "' + transform + '".')
								textureInfo.transforms.append(transformLower)
						except (TypeError, ValueError):
							raise Exception('SceneResources texture transforms must be an array of '
								'transform strings.')

						if decorationStr == 'SRGB':
							textureInfo.transforms.append('srgb')
							decorationStr = 'UNorm'
					except KeyError as e:
						raise Exception(
							'SceneResources texture info doesn\'t contain element "' + str(e) + '".')
				else:
					textureInfo = None
			except KeyError as e:
				raise Exception(
					'SceneResources texture data doesn\'t contain element "' + str(e) + '".')

			# Convert if necessary.
			tempFiles = []
			if (path or paths) and textureInfo:
				needsArray = textureInfo.dimension in (TextureDim.Dim3D, TextureDim.DimCube) or \
					textureInfo.depth > 0
				if path and needsArray:
					raise Exception('SceneResources texture requires multiple texture paths '
						'set with "pathArray".')
				if paths and not needsArray:
					raise Exception('SceneResources texture requires a single texture path '
						'set with "pathArray".')

				commandLine = [convertContext.cuttlefish, '-q', '-f', formatStr,
					'-t', decorationStr, '-m', str(textureInfo.mipLevels),
					'-Q', textureInfo.quality, '-r', textureInfo.rotate]

				if convertContext.multithread:
					commandLine.append('-j')

				if textureInfo.width and textureInfo.height:
					commandLine.extend(['-r', str(textureInfo.width), str(textureInfo.height)])

				if textureInfo.alpha:
					commandLine.extend(['--alpha', textureInfo.alpha])

				commandLine.append('-d')
				if textureInfo.dimension == TextureDim.Dim1D:
					commandLine.append('1')
				elif textureInfo.dimension == TextureDim.Dim3D:
					commandLine.append('3')
				else:
					commandLine.append('2')

				if textureInfo.normalmap != 0.0:
					commandLine.extend(['-n', str(textureInfo.normalmap)])

				if textureInfo.swizzle:
					commandLine.extend('-s', textureInfo.swizzle)

				for transform in textureInfo.transforms:
					commandLine.append('--' + transform)

				# Handle all cases with a text file with the file names.
				with NamedTemporaryFile(mode = 'wt', delete = False) as tempFile:
					if path:
						tempFile.write(path + '\n')
						if textureInfo.dimension == TextureDim.DimCube:
							textureType = 'cube'
						else:
							textureType = 'image'
					else:
						for p in paths:
							tempFile.write(p + '\n')
						if textureInfo.dimension == TextureDim.DimCube:
							textureType = 'cube-array'
						else:
							textureType = 'array'
					tempFiles.append(tempFile.name)
					commandLine.extend(['-I', textureType, tempFile.name])
				# Create a temporary file if not present.
				textureFile = textureData.get('output')
				if not textureFile:
					try:
						with NamedTemporaryFile() as tempFile:
							textureFile = tempFile.name + '.pvr'
						tempFiles.append(textureFile)
					except:
						cleanup(tempFiles)
						raise
				commandLine.extend(['-o', textureFile])

				try:
					subprocess.check_call(commandLine)
				except:
					cleanup(tempFiles)
					raise
			elif path:
				textureFile = path
			elif paths:
				raise Exception('SceneResources texture "pathArray" may only be used when '
					'converting textures.')
			elif textureInfo:
				texturePath = None
			else:
				raise Exception('SceneResources texture doesn\'t have an "path" or '
					'"textureInfo" member.')

			nameOffset = builder.CreateString(name)
			try:
				dataType, dataOffset = convertFileOrData(builder, texturePath, None,
					textureData.get('output'), textureData.get('outputRelativeDir'),
					textureData.get('resourceType'))
				cleanup(tempFiles)
			except:
				cleanup(tempFiles)
				raise

			# Only add texture info if no texture data at all. Otherwise it's used only for
			# conversion.
			if not texturePath and textureInfo:
				TextureInfoStart(builder)
				TextureInfoAddFormat(builder, textureInfo.format)
				TextureInfoAddDecoration(builder, textureInfo.decoration)
				TextureInfoAddDimension(builder, textureInfo.dimension)
				TextureInfoAddWidth(builder, textureInfo.width)
				TextureInfoAddHeight(builder, textureInfo.height)
				TextureInfoAddDepth(builder, textureInfo.depth)
				TextureInfoAddMipLevels(builder, textureInfo.mipLevels)
				textureInfoOffset = TextureInfoEnd(builder)
			else:
				textureInfoOffset = 0

			TextureStart(builder)
			TextureAddName(builder, nameOffset)
			TextureAddUsage(builder, usage)
			TextureAddMemoryHints(builder, memoryHints)
			TextureAddDataType(builder, dataType)
			TextureAddData(builder, dataOffset)
			TextureAddTextureInfo(builder, textureInfoOffset)
			textureOffset = TextureEnd(builder)
	except (TypeError, ValueError):
		raise Exception('SceneResources "textures" must be an array of objects.')

	SceneResourcesStartTexturesVector(builder, len(textureOffsets))
	for offset in reversed(textureOffsets):
		builder.PrependUOffsetTRelative(offset)
	return builder.EndVector(len(textureOffsets))

def convertSceneResourcesShaderVariableGroupDescs(builder, convertContext, data):
	def readElement(elementData):
		try:
			name = str(elementData['name'])
			typeStr = str(elementData['type'])
			try:
				materialType = getattr(MaterialType, typeStr)
			except AttributeError:
				raise Exception('Invalid material type "' + typeStr + '".')
			if materialType >= MaterialType.Texture:
				raise Exception(
					'Shader variable groups may only contain primitive, vector, and matrix types.')

			countStr = str(elementData.get('count', 0))
			try:
				count = int(countStr)
				if count < 0:
					raise Exception() # Common error handling in except block.
			except:
				raise Exception(
					'Invalid shader variable group element count "' + str(countStr) + '".')

			return name, materialType, count
		except KeyError as e:
			raise Exception(
				'SceneResources shader variable group desc element data doesn\'t contain element "'
					+ str(e) + '".')

	groupOffsets = []
	try:
		for groupData in data:
			try:
				name = str(groupData['name'])

				elements = []
				try:
					for elementData in groupData['elements']:
						elements.append(readElement(elementData))
					if not elements:
						raise Exception('Shader variable group desc "elements" must not be empty.')
				except (TypeError, ValueError):
					raise Exception(
						'Shder variable group desc "elements" must be an array of objects.')
			except KeyError as e:
				raise Exception(
					'SceneResources shader variable group desc data doesn\'t contain element "' +
					str(e) + '".')

			nameOffset = builder.CreateString(name)
			elementOffsets = []
			for elementName, elementType, elementCount in elements:
				elementNameOffset = builder.CreateString(elementName)
				VariableElementStart(builder)
				VariableElementAddName(builder, elementNameOffset)
				VariableElementAddType(builder, elementType)
				VariableElementAddCount(builder, elementCount)
				elementOffsets.append(VariableElementEnd(builder))

			ShaderVariableGroupDescStartElementsVector(builder, len(elementOffsets))
			for offset in reversed(elementOffsets):
				builder.PrependUOffsetTRelative(offset)
			elementsOffset = builder.EndVector(len(elementOffsets))

			ShaderVariableGroupDescStart(builder)
			ShaderVariableGroupDescAddName(builder, nameOffset)
			ShaderVariableGroupDescAddElements(builder, elementsOffset)
			groupOffsets.append(ShaderVariableGroupDescEnd(builder))
	except (TypeError, ValueError):
		raise Exception('SceneResources "shaderVariableGroupDescs" must be an array of objects.')

	SceneResourcesStartShaderVariableGroupDescsVector(builder, len(groupOffsets))
	for offset in reversed(groupOffsets):
		builder.PrependUOffsetTRelative(offset)
	return builder.EndVector(len(groupOffsets))


def convertSceneResourcesShaderData(builder, convertContext, data, memberName):
	def readInt(value, name, minVal):
		try:
			intVal = int(value)
			if intVal < minVal:
				raise Exception() # Common error handling in except block.
			return intVal
		except:
			raise Exception('Invalid shader data ' + name + ' value "' + str(value) + '".')

	def convertVariableData(builder, materialType, dataArray):
		if len(dataArray) == 0:
			return 0, 0

		def packSingleElement(formatStr, name):
			dataBytes = bytearray()
			try:
				for element in dataArray:
					struct.pack_into(dataBytes, len(dataBytes), formatStr, element)
			except:
				if name[0] in ('a', 'e', 'i', 'o', 'u'):
					raise Exception('Shader data must be an ' + name + '.')
				else:
					raise Exception('Shader data must be a ' + name + '.')
			return dataBytes

		def packVectorElement(formatStr, name, expectedLen):
			dataBytes = bytearray()
			try:
				for elementArray in dataArray:
					if len(elementArray) != expectedLen:
						raise Exception() # Common error handling in except block.
					for element in elementArray:
						struct.pack_into(dataBytes, len(dataBytes), element)
			except:
				raise Exception('Shader data must be an array of ' + str(expectedLen) +
					' ' + name + 's.')
			return dataBytes

		def packMatrixElement(formatStr, name, expectedCol, expectedRow):
			dataBytes = bytearray()
			try:
				for colArray in dataArray:
					if len(colArray) != expectedCol:
						raise Exception() # Common error handling in except block.
					for col in colArray:
						if len(col) != expectedRow:
							raise Exception() # Common error handling in except block.
						for element in col:
							struct.pack_into(dataBytes, len(dataBytes), element)
			except:
				raise Exception('Shader data must be an array of ' + str(expectedCol) +
					' colomn arrays with ' + str(expectedRow) + ' ' + name + 's.')
			return dataBytes

		def packBool():
			dataBytes = bytearray()
			try:
				for element in dataArray:
					struct.pack_into(dataBytes, len(dataBytes), int(bool(element)))
			except:
				raise Exception('Shader data must be a bool.')
			return dataBytes

		def packBoolVector(expectedLen):
			dataBytes = bytearray()
			try:
				for elementArray in dataArray:
					if len(elementArray) != expectedLen:
						raise Exception() # Common error handling in except block.
					for element in elementArray:
						struct.pack_into(dataBytes, len(dataBytes), int(bool(element)))
			except:
				raise Exception('Shader data must be an array of ' + str(expectedLen) +
					' bools.')
			return dataBytes

		if materialType == MaterialType.Float:
			dataBytes = packSingleElement('f', 'float')
		elif materialType == MaterialType.Vec2:
			dataBytes = packVectorElement('f', 'float', 2)
		elif materialType == MaterialType.Vec3:
			dataBytes = packVectorElement('f', 'float', 3)
		elif materialType == MaterialType.Vec4:
			dataBytes = packVectorElement('f', 'float', 4)
		elif materialType == MaterialType.Double:
			dataBytes = packSingleElement('d', 'double')
		elif materialType == MaterialType.DVec2:
			dataBytes = packVectorElement('d', 'double', 2)
		elif materialType == MaterialType.DVec3:
			dataBytes = packVectorElement('d', 'double', 3)
		elif materialType == MaterialType.DVec4:
			dataBytes = packVectorElement('d', 'double', 4)
		elif materialType == MaterialType.Int:
			dataBytes = packSingleElement('i', 'int')
		elif materialType == MaterialType.IVec2:
			dataBytes = packVectorElement('i', 'int', 2)
		elif materialType == MaterialType.IVec3:
			dataBytes = packVectorElement('i', 'int', 3)
		elif materialType == MaterialType.IVec4:
			dataBytes = packVectorElement('i', 'int', 4)
		elif materialType == MaterialType.UInt:
			dataBytes = packSingleElement('I', 'unsigned int')
		elif materialType == MaterialType.UVec2:
			dataBytes = packVectorElement('I', 'unsigned int', 2)
		elif materialType == MaterialType.UVec3:
			dataBytes = packVectorElement('I', 'unsigned int', 3)
		elif materialType == MaterialType.UVec4:
			dataBytes = packVectorElement('I', 'unsigned int', 4)
		elif materialType == MaterialType.Bool:
			dataBytes = packBool()
		elif materialType == MaterialType.BVec2:
			dataBytes = packBoolVector(2)
		elif materialType == MaterialType.BVec3:
			dataBytes = packBoolVector(3)
		elif materialType == MaterialType.BVec4:
			dataBytes = packBoolVector(4)
		elif materialType == MaterialType.Mat2:
			dataBytes = packMatrixElement('f', 'float', 2, 2)
		elif materialType == MaterialType.Mat3:
			dataBytes = packMatrixElement('f', 'float', 3, 3)
		elif materialType == MaterialType.Mat4:
			dataBytes = packMatrixElement('f', 'float', 4, 4)
		elif materialType == MaterialType.Mat2x3:
			dataBytes = packMatrixElement('f', 'float', 2, 3)
		elif materialType == MaterialType.Mat2x4:
			dataBytes = packMatrixElement('f', 'float', 2, 4)
		elif materialType == MaterialType.Mat3x2:
			dataBytes = packMatrixElement('f', 'float', 3, 2)
		elif materialType == MaterialType.Mat3x4:
			dataBytes = packMatrixElement('f', 'float', 3, 4)
		elif materialType == MaterialType.Mat4x2:
			dataBytes = packMatrixElement('f', 'float', 4, 2)
		elif materialType == MaterialType.Mat4x3:
			dataBytes = packMatrixElement('f', 'float', 4, 3)
		elif materialType == MaterialType.DMat2:
			dataBytes = packMatrixElement('d', 'double', 2, 2)
		elif materialType == MaterialType.DMat3:
			dataBytes = packMatrixElement('d', 'double', 3, 3)
		elif materialType == MaterialType.DMat4:
			dataBytes = packMatrixElement('d', 'double', 4, 4)
		elif materialType == MaterialType.DMat2x3:
			dataBytes = packMatrixElement('d', 'double', 2, 3)
		elif materialType == MaterialType.DMat2x4:
			dataBytes = packMatrixElement('d', 'double', 2, 4)
		elif materialType == MaterialType.DMat3x2:
			dataBytes = packMatrixElement('d', 'double', 3, 2)
		elif materialType == MaterialType.DMat3x4:
			dataBytes = packMatrixElement('d', 'double', 3, 4)
		elif materialType == MaterialType.DMat4x2:
			dataBytes = packMatrixElement('d', 'double', 4, 2)
		elif materialType == MaterialType.DMat4x3:
			dataBytes = packMatrixElement('d', 'double', 4, 3)
		else:
			if len(dataArray) != 1:
				raise Exception('Shader data must not be an array for object types.')

			dataElement = dataArray[0]
			if materialType in (MaterialType.Texture, MaterialType.Image,
					MaterialType.VariableGroup):
				try:
					textureName = str(dataElement)
				except TypeError:
					raise Exception('Shader data must be a string.')

				tempBuilder = flatbuffers.Builder(0)
				textureOffset = tempBuilder.CreateString(textureName)
				NamedMaterialDataStart(tempBuilder)
				NamedMaterialDataAddName(tempBuilder, textureOffset)
				tempBuilder.Finish(NamedMaterialDataEnd(tempBuilder))
				dataBytes = tempBuilder.Output()
			elif materialType in (MaterialType.TextureBuffer, MaterialType.ImageBuffer):
				try:
					bufferName = str(dataElement['name'])

					formatStr = str(dataElement['format'])
					try:
						texFormat = getattr(TextureFormat, formatStr)
						if texFormat >= TextureFormat.BC1_RGB:
							raise Exception() # Common error handling in except block.
					except:
						raise Exception('Invalid texture buffer format "' + formatStr + '".')

					decorationStr = str(dataElement['decoration'])
					try:
						decoration = getattr(FormatDecoration, decorationStr)
					except AttributeError:
						raise Exception(
							'Invalid texture buffer format decoration "' + decorationStr + '".')

					offset = readInt(data.get('offset', 0), 'offset', 0)
					count = readInt(data['count'], 'count', 1)
				except KeyError as e:
					raise Exception(
						'SceneResources shader element data doesn\'t contain element "' +
						str(e) + '".')
				except (TypeError, ValueError):
					raise Exception(
						'SceneResources shader element data must be an object.')

				tempBuilder = flatbuffers.Builder(0)
				textureOffset = tempBuilder.CreateString(textureName)
				TextureBufferMaterialDataStart(tempBuilder)
				TextureBufferMaterialDataAddName(tempBuilder, textureOffset)
				TextureBufferMaterialDataAddFormat(tempBuilder, texFormat)
				TextureBufferMaterialDataAddDecoration(tempBuilder, decoration)
				TextureBufferMaterialDataAddOffset(tempBuilder, offset)
				TextureBufferMaterialDataAddCount(tempBuilder, count)
				tempBuilder.Finish(TextureBufferMaterialDataEnd(tempBuilder))
				dataBytes = tempBuilder.Output()
			elif materialType in (MaterialType.UniformBlock, MaterialType.UniformBuffer):
				try:
					bufferName = str(dataElement['name'])
					offset = readInt(data.get('offset', 0), 'offset', 0)
					size = readInt(data['size'], 'size', 1)
				except KeyError as e:
					raise Exception(
						'SceneResources shader element data doesn\'t contain element "' +
						str(e) + '".')
				except (TypeError, ValueError):
					raise Exception(
						'SceneResources shader element data must be an object.')

				tempBuilder = flatbuffers.Builder(0)
				textureOffset = tempBuilder.CreateString(textureName)
				BufferMaterialDataStart(tempBuilder)
				BufferMaterialDataAddName(tempBuilder, textureOffset)
				BufferMaterialDataAddOffset(tempBuilder, offset)
				BufferMaterialDataAddSize(tempBuilder, size)
				tempBuilder.Finish(BufferMaterialDataEnd(tempBuilder))
				dataBytes = tempBuilder.Output()

		return dataBytes, len(dataArray)

	def convertDataElement(builder, data):
		try:
			name = str(data['name'])
			typeStr = str(data['type'])
			try:
				materialType = getattr(MaterialType, typeStr)
			except AttributeError:
				raise Exception('Invalid material type "' + typeStr + '".')

			first = readInt(data.get('first', 0), 'first', 0)

			if 'data' in data and 'dataArray' in data:
				raise Exception(
					'SceneResources shader data element must contain one of "data" or "dataArray".')

			if 'data' in data:
				dataBytes, dataCount = convertVariableData(builder, materialType, [data['data']])
			elif 'dataArray' in data:
				try:
					dataBytes, dataCount = convertVariableData(
						builder, materialType, data['dataArray'])
				except (TypeError, ValueError):
					raise Exception(
						'Shder data element "dataArray" must be an array of data values.')
			else:
				raise Exception(
					'SceneResources shader data element must contain one of "data" or "dataArray".')
		except KeyError as e:
			raise Exception(
				'SceneResources shader data element doesn\'t contain element "' + str(e) + '".')

		nameOffset = builder.CreateString(name)
		dataOffset = builder.CreateByteVector(dataBytes)

		VariableDataStart(builder)
		VariableDataAddName(builder, nameOffset)
		VariableDataAddType(builder, materialType)
		VariableDataAddFirst(builder, first)
		VariableDataAddCount(builder, dataCount)
		VariableDataAddData(builder, dataOffset)
		return VariableDataEnd(builder)

	dataOffsets = []
	try:
		for dataElement in data:
			try:
				name = str(dataElement['name'])
				description = str(dataElement['description'])

				try:
					elementDataOffsets = []
					for dataValue in dataElement['data']:
						elementDataOffsets.append(convertDataElement(builder, dataValue))
				except (TypeError, ValueError):
					raise Exception(
						'SceneResources shader data "data" must be an array of objects.')
			except KeyError as e:
				raise Exception(
					'SceneResources shader data doesn\'t contain element "' + str(e) + '".')

			nameOffset = builder.CreateString(name)
			descriptionOffset = builder.CreateString(description)

			ShaderDataStartDataVector(builder, len(elementDataOffsets))
			for offset in reversed(elementDataOffsets):
				builder.PrependUOffsetTRelative(offset)
			dataOffset = builder.EndVector(len(elementDataOffsets))

			ShaderDataStart(builder)
			ShaderDataAddName(builder, nameOffset)
			ShaderDataAddDescription(builder, description)
			ShaderDataAddData(builder, dataOffset)
			dataOffsets.append(ShaderDataEnd(builder))
	except (TypeError, ValueError):
		raise Exception('SceneResources "' + memberName + '" must be an array of objects.')

	return dataOffsets

def convertSceneResourcesShaderVariableGroups(builder, convertContext, data):
	dataOffsets = convertSceneResourcesShaderData(
		builder, convertContext, data, 'shaderVariableGroups')

	SceneResourcesStartShaderVariableGroupsVector(builder, len(dataOffsets))
	for offset in reversed(dataOffsets):
		builder.PrependUOffsetTRelative(offset)
	return builder.EndVector(len(dataOffsets))

def convertSceneResourcesMaterialDescs(builder, convertContext, data):
	def readElement(elementData):
		try:
			name = str(elementData['name'])
			typeStr = str(elementData['type'])
			try:
				materialType = getattr(MaterialType, typeStr)
			except AttributeError:
				raise Exception('Invalid material type "' + typeStr + '".')

			countStr = str(elementData.get('count', 0))
			try:
				count = int(countStr)
				if count < 0:
					raise Exception() # Common error handling in except block.
			except:
				raise Exception(
					'Invalid material element count "' + str(countStr) + '".')

			return name, materialType, count
		except KeyError as e:
			raise Exception(
				'SceneResources material desc element data doesn\'t contain element "' +
					str(e) + '".')

	groupOffsets = []
	try:
		for groupData in data:
			try:
				name = str(groupData['name'])

				elements = []
				try:
					for elementData in groupData['elements']:
						elements.append(readElement(elementData))
					if not elements:
						raise Exception('Material desc "elements" must not be empty.')
				except (TypeError, ValueError):
					raise Exception('Material desc "elements" must be an array of objects.')
			except KeyError as e:
				raise Exception(
					'Material desc data doesn\'t contain element "' + str(e) + '".')

			nameOffset = builder.CreateString(name)
			elementOffsets = []
			for elementName, elementType, elementCount in elements:
				elementNameOffset = builder.CreateString(elementName)
				VariableElementStart(builder)
				VariableElementAddName(builder, elementNameOffset)
				VariableElementAddType(builder, elementType)
				VariableElementAddCount(builder, elementCount)
				elementOffsets.append(VariableElementEnd(builder))

			ShaderVariableGroupDescStartElementsVector(builder, len(elementOffsets))
			for offset in reversed(elementOffsets):
				builder.PrependUOffsetTRelative(offset)
			elementsOffset = builder.EndVector(len(elementOffsets))

			ShaderVariableGroupDescStart(builder)
			ShaderVariableGroupDescAddName(builder, nameOffset)
			ShaderVariableGroupDescAddElements(builder, elementsOffset)
			groupOffsets.append(ShaderVariableGroupDescEnd(builder))
	except (TypeError, ValueError):
		raise Exception('SceneResources "materialDescs" must be an array of objects.')

	SceneResourcesStartMaterialDescsVector(builder, len(groupOffsets))
	for offset in reversed(groupOffsets):
		builder.PrependUOffsetTRelative(offset)
	return builder.EndVector(len(groupOffsets))

def convertSceneResourcesMaterials(builder, convertContext, data):
	dataOffsets = convertSceneResourcesShaderData(builder, convertContext, data, 'materials')

	SceneResourcesStartMaterialsVector(builder, len(dataOffsets))
	for offset in reversed(dataOffsets):
		builder.PrependUOffsetTRelative(offset)
	return builder.EndVector(len(dataOffsets))

def convertSceneResourcesShaderModules(builder, convertContext, data):
	moduleOffsets = []
	try:
		for moduleData in data:
			try:
				name = str(moduleData['name'])

				moduleStr = str(moduleData['module'])
				try:
					modulePath, moduleContents = readData(moduleStr)
				except TypeError:
					raise Exception(
						'SceneResources shader module "module" uses incorrect base64 encoding.')
			except KeyError as e:
				raise Exception('Shader module data doesn\'t contain element "' + str(e) + '".')

			nameOffset = builder.CreateString(name)
			dataType, dataOffset = convertFileOrData(builder, modulePath, moduleContents,
				moduleData.get('output'), moduleData.get('outputRelativeDir'),
				moduleData.get('resourceType'))

			ShaderModuleStart(builder)
			ShaderModuleAddName(builder, nameOffset)
			ShaderModuleAddDataType(builder, dataType)
			ShaderModuleAddData(builder, dataOffset)
			moduleOffsets.append(ShaderModuleEnd(builder))
	except (TypeError, ValueError):
		raise Exception('SceneResources "shaderModules" must be an array of objects.')

	SceneResourcesStartShaderModulesVector(builder, len(moduleOffsets))
	for offset in reversed(moduleOffsets):
		builder.PrependUOffsetTRelative(offset)
	return builder.EndVector(len(moduleOffsets))

def convertSceneResourcesShaders(builder, convertContext, data):
	shaderOffsets = []
	try:
		for shaderData in data:
			try:
				name = str(shaderData['name'])
				module = str(shaderData['module'])
				pipeline = str(shaderData.get('pipelineName', name))
				materialDesc = str(shaderData['materialDesc'])
			except KeyError as e:
				raise Exception('Shader data doesn\'t contain element "' + str(e) + '".')

			nameOffset = builder.CreateString(name)
			moduleOffset = builder.CreateString(module)
			pipelineOffset = builder.CreateString(pipeline)
			materialDescOffset = builder.CreateString(materialDesc)

			ShaderStart(builder)
			ShaderAddName(builder, nameOffset)
			ShaderAddShaderModule(builder, moduleOffset)
			ShaderAddPipelineName(builder, pipelineOffset)
			ShaderAddMaterialDesc(builder, materialDescOffset)
			shaderOffsets.append(ShaderEnd(builder))
	except (TypeError, ValueError):
		raise Exception('SceneResources "shaders" must be an array of objects.')

	SceneResourcesStartShadersVector(builder, len(shaderOffsets))
	for offset in reversed(shaderOffsets):
		builder.PrependUOffsetTRelative(offset)
	return builder.EndVector(len(shaderOffsets))

def convertSceneResourcesDrawGeometries(builder, convertContext, data):
	def readVertexBuffer(vertexBufferData):
		def readInt(value, name, minVal):
			try:
				intVal = int(value)
				if intVal < minVal:
					raise Exception() # Common error handling in except block.
				return intVal
			except:
				raise Exception('Invalid vertex buffer ' + name + ' "' + str(value) + '".')

		try:
			vertexBuffer = object()
			vertexBuffer.name = str(vertexBufferData['name'])
			vertexBuffer.offset = readInt(vertexBufferData.get('offset', 0), 'offset', 0)
			vertexBuffer.count = readInt(vertexBufferData['count'], 'count', 1)

			formatData = vertexBufferData['format']
			try:
				vertexBuffer.format = object()
				vertexBuffer.format.attributes = []
				try:
					for attributeData in formatData['attributes']:
						try:
							attribute = object()

							vertexBuffer.attrib = readVertexAttrib(attributeData['attrib'])

							formatStr = attributeData['format']
							try:
								attribute.format = getattr(VertexElementFormat, formatStr)
							except AttributeError:
								raise Exception('Invalid vertex format "' + formatStr + '".')

							decorationStr = attributeData['decoration']
							try:
								attribute.decoration = getattr(FormatDecoration, decorationStr)
							except AttributeError:
								raise Exception(
									'Invalid vertex format decoration "' + decorationStr + '".')

							vertexBuffer.format.attributes.append(attribute)
						except KeyError as e:
							raise Exception(
								'Vertex buffer format attribute doesn\'t contain element "' +
								str(e) + '".')
				except (TypeError, ValueError):
					raise Exception(
						'Vertex buffer format "attributes" must be an array of objects.')

				if not vertexBuffer.format.attributes:
					raise Exception('Vertex format doesn\'t contain any attributes.')

				vertexBuffer.format.instanced = bool(formatData.get('instanced', False))
			except KeyError as e:
				raise Exception('Vertex buffer format doesn\'t contain element "' + str(e) + '".')

			return vertexBuffer
		except KeyError as e:
			raise Exception('Vertex buffer data doesn\'t contain element "' + str(e) + '".')

	def readIndexBuffer(indexBufferData):
		def readInt(value, name, minVal):
			try:
				intVal = int(value)
				if intVal < minVal:
					raise Exception() # Common error handling in except block.
				return intVal
			except:
				raise Exception('Invalid index buffer ' + name + ' "' + str(value) + '".')

		try:
			indexBuffer = object()
			indexBuffer.name = str(indexBufferData['name'])
			indexBuffer.offset = readInt(indexBufferData.get('offset', 0), 'offset', 0)
			indexBuffer.count = readInt(indexBufferData['count'], 'count', 1)
			indexBuffer.indexSize = readInt(indexBufferData['indexSize'], 'indexSize', 0)
			if indexBuffer.indexSize not in (2, 4):
				raise Exception('Index buffer size must be 2 or 4.')

			return indexBuffer
		except KeyError as e:
			raise Exception('Index buffer data doesn\'t contain element "' + str(e) + '".')

	maxVertexBuffers = 4
	geometryOffsets = []
	try:
		for geometryData in data:
			try:
				name = str(geometryData['name'])

				vertexBuffers = []
				try:
					for vertexBufferData in geometryData['vertexBuffers']:
						vertexBuffers.append(readVertexBuffer(vertexBufferData))
				except (TypeError, ValueError):
					raise Exception('Draw geometry "vertexBuffers" must be an array of objects.')

				if not 0 < len(vertexBuffers) <= maxVertexBuffers:
					raise Exception('Draw geometry must have between 1 and ' +
						str(maxVertexBuffers) + ' vertex buffers.')

				indexBufferData = geometryData.get('indexBuffer')
				if indexBufferData:
					indexBuffer = readIndexBuffer(indexBufferData)
				else:
					indexBuffer = None
			except KeyError as e:
				raise Exception('Draw geometry data doesn\'t contain element "' + str(e) + '".')

			nameOffset = builder.CreateString(name)
			vertexBufferOffsets = []
			for vertexBuffer in vertexBuffers:
				vertexBufferNameOffset = builder.CreateString(vertexBuffer.name)

				attributeOffsets = []
				for attribute in vertexBuffer.format.attributes:
					attributeOffsets.append(
						CreateVertexAttribute(builder, attribute.attrib, attribute.format,
							attribute.decoration))

				VertexFormatStartAttributesVector(builder, len(attributeOffsets))
				for offset in reversed(attributeOffsets):
					builder.PrependUOffsetTRelative(offset)
				attributesOffset = builder.EndVector(len(attributeOffsets))

				VertexFormatStart(builder)
				VertexFormatAddAttributes(builder, attributesOffset)
				VertexFormatAddInstanced(builder, vertexBuffer.format.instanced)
				formatOffset = VertexFormatEnd(builder)

				VertexBufferStart(builder)
				VertexBufferAddName(builder, vertexBufferNameOffset)
				VertexBufferAddOffset(builder, vertexBuffer.offset)
				VertexBufferAddCount(builder, vertexBuffer.count)
				VertexBufferAddFormat(builder, formatOffset)
				vertexBufferOffsets.append(VertexBufferEnd(builder))

			DrawGeometryStartVertexBuffersVector(builder, len(vertexBufferOffsets))
			for offset in reversed(vertexBufferOffsets):
				builder.PrependUOffsetTRelative(offset)
			vertexBuffersOffset = builder.EndVector(len(vertexBufferOffsets))

			if indexBuffer:
				indexBufferNameOffset = builder.CreateString(indexBuffer.name)

				IndexBufferStart(builder)
				IndexBufferAddName(builder, indexBufferNameOffset)
				IndexBufferAddOffset(builder, indexBuffer.offset)
				IndexBufferAddCount(builder, indexBuffer.count)
				IndexBufferAddIndexSize(builder, indexBuffer.indexSize)
				indexBufferOffset = IndexBufferEnd(builder)
			else:
				indexBufferOffset = 0

			DrawGeometryStart(builder)
			DrawGeometryAddName(builder, nameOffset)
			DrawGeometryAddVertexBuffers(builder, vertexBuffersOffset)
			DrawGeometryAddIndexBuffer(builder, indexBufferOffset)
			geometryOffsets.append(DrawGeometryEnd(builder))
	except (TypeError, ValueError):
		raise Exception('SceneResources "drawGeometries" must be an array of objects.')

	SceneResourcesStartDrawGeometriesVector(builder, len(geometryOffsets))
	for offset in reversed(geometryOffsets):
		builder.PrependUOffsetTRelative(offset)
	return builder.EndVector(len(geometryOffsets))

def convertSceneResourcesNodes(builder, convertContext, data):
	nodeOffsets = []
	try:
		for nodeData in data:
			try:
				name = str(nodeData['name'])
				nodeType = str(nodeData['type'])
				nodeInfo = nodeData['data']
			except KeyError as e:
				raise Exception('Scene node data doesn\'t contain element "' + str(e) + '".')

			nameOffset = builder.CreateString(name)
			nodeDataOffset = convertContext.convertNode(builder, nodeType, nodeInfo)

			SceneNodeStart(builder)
			SceneNodeAddName(builder, nameOffset)
			SceneNodeAddNode(builder, nodeDataOffset)
			nodeOffsets.append(SceneNodeEnd(builder))
	except (TypeError, ValueError):
		raise Exception('SceneResources "drawGeometries" must be an array of objects.')

	SceneResourcesStartSceneNodesVector(builder, len(nodeOffsets))
	for offset in reversed(nodeOffsets):
		builder.PrependUOffsetTRelative(offset)
	return builder.EndVector(len(nodeOffsets))

def convertSceneResources(convertContext, data):
	"""
	Converts SceneResources. The data map is expected to contain the following optional elements,
	where empty arrays at the top level may be ommitted:
	- buffers: array of buffers to embed. Each element of the array has the following members:
	  - name: string name of the buffer.
	  - usage: array of usage flags. See the dsGfxBufferUsage enum for values, removing the type
	    prefix. At least one must be provided.
	  - memoryHints: array of memory hints. See the dsGfxMemory enum for values, removing the type
	    prefix. At least one must be provided.
	  - size: the size of the buffer. This is only used if no data is provided.
	  - data: path to the buffer data or base64 encoded data prefixed with "base64:". This may be
	    ommitted to leave the buffer data uninitialized.
	  - output: the path to the output the buffer. This can be ommitted if no input path is provided
	    or if the buffer is embedded.
	  - outputRelativeDir: the directory relative to output path. This will be removed from the path
	    before adding the reference.
	  - resourceType: the resource type. See the dsFileResourceType for values, removing the type
	    prefix. Defaults to "Embedded".
	- texture: array of textures to include. Each element of the array has the following members:
	  - name: the name of the texture.
	  - usage: array of usage flags. See the dsGfxBufferUsage enum for values, removing the type
	    prefix. Defaults to ["Texture"]. If set, at least one must be provided.
	  - memoryHints: array of memory hints. See the dsGfxMemory enum for values, removing the type
	    prefix. Defaults to ["GPUOnly", "Static"]. If set, at least one must be provided.
	  - path: path to the texture image. This may be ommitted if no initial texture data is used.
	  - pathArray: array of paths to texture images. Use this in place of "path" for texture arrays
	    or cubemaps.
	  - output: the path to the output the texture. This can be ommitted if no input path is
	    provided or if the texture is embedded. When converting textures, the extension should
	    match the desired output container format.
	  - outputRelativeDir: the directory relative to output path. This will be removed from the path
	    before adding the reference.
	  - resourceType: the resource type. See the dsFileResourceType for values, removing the type
	    prefix. Defaults to "Embedded".
	  - textureInfo: the info to describe the texture. This must be provided if no image path is
	    provided or if the image from the input path is to be converted. This is expected to have
	    the following elements:
	    - format: the texture format. See the dsGfxFormat enum for values, removing the type prefix.
	      The decorator values may not be used.
	    - decoration: the decoration for the format. See the dsGfxFormat enum for values, removing
	      the type prefix. Only the decorator values may be used.
	    - dimension: the dimension of the texture. See the dsTextureDim enum for values, removing
	      the type prefix and starting with "Dim". "Dim2D" is used by default.
	    - width: the width of the texture in pixels. When converting, may also be the string
	      nextpo2 or nearestpo2. When converting, this can be ommitted to use the original image
	      width.
	    - height: the height of the texture in pixels. When converting, may also be the string
	      nextpo2 or nearestpo2. When converting, this can be ommitted to use the original image
	      height.
	    - depth: the depth or array layers of the texture. If 0 or ommitted, this is not a texture
	      array.
	    (the following elements are only used for texture conversion)
	    - mipLevels: the number of mipmap levels.
	    - quality: the quality to use during conversion. May be one of lowest, low, normal, high,
	      or highest. Defaults to normal.
	    - normalmap: float value for a height to use to convert to a normalmap.
	    - swizzle: string of R, G, B, A, or X values to swizzle the color channels.
	    - rotate: angle to rotate. Must be a multile of 90 degrees in the range [-270, 270].
	    - alpha: the alpha mode to use. Must be: none, standard, pre-multiplied, or encoded. Default
	      value is standard.
	    - transforms: array of transforms to apply. Valid values are: flipx, flipy, srgb,
	      grayscale, and pre-multiply. Note that srgb is implied if the decorator is srgb.
	- shaderVariableGroupDescs: array of shader variable group descriptions to include. Each
      element of the array has the following members:
	  - name: the name of the shader variable group description.
	  - elements: array of elements for the shader variable group. Each element of the array has
	    the following members:
	    - name: the name of the element.
	    - type: the type of the element. See dsMaterialType enum for values, removing the type
	      prefix.
	    - count: the number of array elements. If 0 or ommitted, this is not an array.
	- shaderVariableGroups: array of shader variable groups to include. Each element of the array
	  has the following members:
	  - name: the name of the element.
	  - description: the name of the description defined in shaderVariableGroupDescs. The
	    description may be in a different scene resources package.
	  - data: array of data elements to set. Each element of the array has the following members:
	    - name: the name of the data element.
	    - type: the type of the element. See the dsMaterialType enum for values, removing the type
	      prefix.
	    - first: the index of the first element to set when it's an array. Defaults to 0 if not set.
	    - data: the data to set, with the contents depending on the "type" that was set.
	      - Vector types are arrays, while matrix types are arrays of column vector arrays.
	      - Textures, images, and variable groups have the string name of the resource.
	      - Buffers are objects with the following members:
	        - name: the name of the buffer.
	        - offset: integer byte offset into the buffer. Defaults to 0.
	        - size: the integer bytes to bind within the buffer.
	      - Texture buffers and image buffers are objects with the following members:
	        - name: the name of the buffer.
	        - format: the texture format. See the dsGfxFormat enum for values, removing the type
	          prefix. The decorator and compressed values may not be used.
	        - decoration: the decoration for the format. See the dsGfxFormat enum for values,
	          removing the type prefix. Only the decorator values may be used.
		    - offset: integer byte offset into the buffer. Defaults to 0.
	        - count: integer number of texels in the buffer.
	    - dataArray: this may be set in place of the data member to provide an array of data
	      elements rather than a single one.
	- materialDescs: array of material descriptions to include. Each element of the array has the
	  following members:
	  - name: the name of the material description.
	  - elements: array of elements for the material. Each element of the array has the following
	    members:
	    - name: the name of the element.
	    - type: the type of the element. See dsMaterialType enum for values, removing the type
	      prefix.
	    - count: the number of array elements. If 0 or ommitted, this is not an array.
		- binding: the binding type for the element. See the dsMaaterialBinding enum for values,
	      removing the type prefix. This is only used for texture, image, buffer, and shader
		  variable group types.
	    - shaderVariableGroupDesc: the name of the shader variable group description when the type
	      is a shader variable group. The description may be in a different scene resources package.
	- materials: array of materials to include. See shaderVariableGroups for a description of the
	  array members, except the "description" element is for a material description rather than a
	  shader variable group description.
	- shaderModules: array of shader modules to include. Each element of the array has the following
	  members:
	  - name: the name of the shader module.
	  - module: path to the shader module or base64 encoded data prefixed with "base64:". The module
	    is expected to have been compiled with Modular Shader Language (MSL).
	  - output: the path to the location to copy the shader module to. This can be ommitted to embed
	    the shader module directly.
	  - outputRelativeDir: the directory relative to output path. This will be removed from the path
	    before adding the reference.
	  - resourceType: the resource type. See the dsFileResourceType for values, removing the type
	    prefix. Defaults to "Embedded".
	- shaders: array of shaders to include. Each element of the array has the following members:
	  - name: the name of the shader.
	  - shaderModule: the name of the shader module the shader resides in. The shader module may be
	    in a different scene resources package.
	  - pipelineName: the name of the shader pipeline within the shader module.
	  - materialDesc: The name of the material description for materials that will be used with the
	    shader. The material may be in a different scene resources package.
	- drawGeometries: array of draw geometries to include. Each element of the array has the
	  following members:
	  - name: the name of the draw geometry.
	  - vertexBuffers: array of vertex buffers. This can have up to 4 elements with the following
	    members:
	    - name: the name of the buffer to use. The buffer may be in a different scene resources
	      package.
	    - offset: the offset in bytes into the buffer to the first vertex. Defaults to 0.
	    - count: the number of vertices in the buffer.
	    - format: the vertex format. This is a dict with the following members:
	      - attributes: array of attributes for the format. Each element has the following members:
			- attrib: the attribute. This can either be an enum value from dsVertexAttrib, removing
			  the type prefix, or the integer for the attribute.
	        - format: the attribute format. See the dsGfxFormat enum for values, removing the type
	          prefix. Only the "standard" formats may be used.
	        - decoration: the decoration for the format. See the dsGfxFormat enum for values,
	          removing the type prefix. Only the decorator values may be used.
	      - instanced: true if the vertex data is instanced. Defaults to false.
	  - indexBuffer: the index buffer. If not set, the draw geometry isn't indexed. This is a dict
	    with the following members:
	    - name: the name of the buffer to use. The buffer may be in a different scene resources
	      package.
	    - offset: the offset in bytes into the buffer to the first index. Defaults to 0.
	    - count: the number of indices in the buffer.
	    - indexSize: the size of the index in bytes. This must be either 2 or 4.
	- sceneNodes: array of scene nodes to include. Each element of the array has the following
	  members:
	  - name: the name of the node.
	  - type: the name of the node type.
	  - data: the data for the node. What this member contains (e.g. a string or a dict with other
	    members) depends on the node type.
	"""
	builder = flatbuffers.Builder(0)

	try:
		if 'buffers' in data:
			buffersOffset = convertSceneResourcesBuffers(builder, convertContext, data['buffers'])
		else:
			buffersOffset = 0

		if 'textures' in data:
			texturesOffset = convertSceneResourcesTextures(builder, convertContext,
				data['textures'])
		else:
			texturesOffset = 0

		if 'shaderVariableGroupDescs' in data:
			shaderVariableGroupDescsOffset = convertSceneResourcesShaderVariableGroupDescs(builder,
				convertContext, data['shaderVariableGroupDescs'])
		else:
			shaderVariableGroupDescsOffset = 0

		if 'shaderVariableGroups' in data:
			shaderVariableGroupDescsOffset = convertSceneResourcesShaderVariableGroups(builder,
				convertContext, data['shaderVariableGroups'])
		else:
			shaderVariableGroupsOffset = 0

		if 'materialDescs' in data:
			materialDescsOffset = convertSceneResourcesMaterialDescs(
				builder, convertContext, data['materialDescs'])
		else:
			materialDescsOffset = 0

		if 'materials' in data:
			materialsOffset = convertSceneResourcesMaterials(
				builder, convertContext, data['materials'])
		else:
			materialsOffset = 0

		if 'shaderModules' in data:
			shaderModulesOffset = convertSceneResourcesShaderModules(
				builder, convertContext, data['shaderModules'])
		else:
			shaderModulesOffset = 0

		if 'shaders' in data:
			shadersOffset = convertSceneResourcesShaders(
				builder, convertContext, data['shaders'])
		else:
			shadersOffset = 0

		if 'drawGeometries' in data:
			drawGeometriesOffset = convertSceneResourcesDrawGeometries(
				builder, convertContext, data['drawGeometries'])
		else:
			drawGeometriesOffset = 0

		if 'sceneNodes' in data:
			nodesOffset = convertSceneResourcesNodes(builder, convertContext, data['sceneNodes'])
		else:
			nodesOffset = 0
	except (TypeError, ValueError):
		raise Exception('SceneResources must be an object.')

	SceneResourcesStart(builder)
	SceneResourcesAddBuffers(builder, buffersOffset)
	SceneResourcesAddTextures(builder, texturesOffset)
	SceneResourcesAddShaderVariableGroupDescs(builder, shaderVariableGroupDescsOffset)
	SceneResourcesAddShaderVariableGroups(builder, shaderVariableGroupsOffset)
	SceneResourcesAddMaterialDescs(builder, materialDescsOffset)
	SceneResourcesAddMaterials(builder, materialsOffset)
	SceneResourcesAddShaderModules(builder, shaderModulesOffset)
	SceneResourcesAddShaders(builder, shadersOffset)
	SceneResourcesAddDrawGeometries(builder, drawGeometriesOffset)
	SceneResourcesAddSceneNodes(builder, nodesOffset)
	builder.Finish(SceneResourcesEnd(builder))
	return builder.Output()
