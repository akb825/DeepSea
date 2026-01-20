#!/usr/bin/env python
# Copyright 2018-2026 Aaron Barany
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

from __future__ import print_function
import argparse
import json
import os
import shutil
import subprocess
import sys
import tempfile

import flatbuffers
from DeepSeaVectorDraw.Convert.SVG import convertSVG
from DeepSeaVectorDraw import FaceGroup
from DeepSeaVectorDraw.FileOrData import FileOrData
from DeepSeaVectorDraw import FileReference
from DeepSeaVectorDraw import Font
from DeepSeaVectorDraw import FontFace
from DeepSeaVectorDraw.FontCacheSize import FontCacheSize
from DeepSeaVectorDraw.FontQuality import FontQuality
from DeepSeaVectorDraw.IconType import IconType
from DeepSeaVectorDraw import RawData
from DeepSeaVectorDraw import TextIcon
from DeepSeaVectorDraw import TextIconGroup
from DeepSeaVectorDraw import TextIcons
from DeepSeaVectorDraw import TextureResource
from DeepSeaVectorDraw.Vector2f import CreateVector2f
from DeepSeaVectorDraw import VectorImageResource
from DeepSeaVectorDraw import VectorResource
from DeepSeaVectorDraw.VectorResourceUnion import VectorResourceUnion
from DeepSeaVectorDraw import VectorResources as VectorResourcesFB

def nameFromPath(path):
	return os.path.splitext(os.path.basename(path))[0]

class VectorResources:
	"""Class containing the information for vector resources to be saved in FlatBuffer format."""
	def __init__(self, cuttlefishTool = 'cuttlefish'):
		"""
		Constructs this in the default state.

		cuttlefish is used to convert images into textures. By default it will check for
		"cuttlefish" on PATH, but the path may be passed into the constructor.
		"""

		self.cuttlefish = cuttlefishTool

	def load(self, contents, basePath = None):
		"""
		Loads the information for the VectorResources from a map.

		basePath: The base path to load relative paths from, typically relative to the original
		file.

		The JSON format should have the following elements:
		- resources: array of objects with the following members:
		  - type: the type of the resource. The following sections document what types are supported
		    and the members it contains:
		    - "Texture": texture referenced within a vector image or text icon.
		      - name: name used to reference the texture. Defaults to path filename without
		        extension
		      - path: path to the input image.
		      - format: texture format. See cuttlefish help for supported formats.
		      - channelType: texture channel type. See cuttlefish help for supported types.
		      - srgb: whether to perform sRGB to linear conversion when interpreting the texture.
		        Defaults to false.
		      - size: array with the target width and height. Defaults to the dimensions of the
		        image.
		      - quality: quality of encoding, one of lowest, low, normal, high, or highest. Defaults
		        to normal.
		      - container: the texture container format, one of pvr, dds, or ktx. Defaults to pvr.
		      - embed: hether to embed embed directly in the resources file. Defaults to false.
		    - "VectorImage": vector image that can be looked up by name.
		      - name: name used to reference the vector image. Defaults to the path filename without
		        extension.
		      - path: path to the input SVG.
		      - defaultFont: the default font to use when none is specified for a text element.
		        Defaults to serif.
		      - targetSize: array with the target width and height. Defaults to the dimensions of
		        the image.
		      - embed: hether to embed embed directly in the resources file. Defaults to false.
		    - "TextureTextIcons", "VectorTextIcons": icons used to replace specific codepoints with
		       either a texture or vector image rather than the glyph from a font face.
		      - name: name used to reference the text icons.
		      - icons: array of array of objects for the individual icons. The outer array groups
		        icons together for faster lookup, where all values between the minimum and maximum
		        codepoints are considered one block of icon glyphs. Each element of the inner array
		        is expected to have the following members:
		        - codepoint: the integer codepoint for the Unicode value to assign the icon to.
		        - icon: the name of the texture or vector image (depending on the parent object's
		          type) to use for the icon.
		        - advance: the amount to advance text after the icon, normalized to be typically
		          in the range [0, 1].
		        - bounds: 2D array for the minimum and maximum bounds for the icon in normalized
		          values typically in the range [0, 1].
		    - "FaceGroup": font faces that can be used by a font.
		      - name: name used to reference the face group.
		      - faces: array of objects for the faces. Each element is expected to ahve the
		        following members:
		        - name: the name of the face to reference by a font. Defaults to the filename of the
		          font file without the extension.
		        - path: the path to the font file.
		        - embed: hether to embed embed directly in the resources file. Defaults to false.
		    - "Font": font for displaying of text.
		      - name: the name used to reference the font.
		      - faceGroup: the face group that provides the faces used by the font.
		      - faces: array of strings for the names of the faces to use within the face group.
		      - icons: the name of text icons to use. Defaults to unset, meaning no icons are used.
		      - quality: the quality of the font as one of low, medium, igh, or veryhigh.
		      - cacheSize: the size of the cache as one of small or large. Defaults to large.
		"""
		self.basePath = basePath
		try:
			self.resources = contents['resources']
		except KeyError as e:
			raise Exception("VectorResources doesn't contain element " + str(e) + '.')
		except (AttributeError, TypeError, ValueError):
			raise Exception('VectorResources must be an object.')

		textures = set()
		vectorImages = set()
		textIcons = set()
		faceGroupFaces = {}
		try:
			for resource in self.resources:
				resourceType = resource['type']
				if resourceType in ('Texture', 'VectorImage'):
					name = resource.get('name')
					if name is None:
						name = nameFromPath(resource['path'])
					if resourceType == 'Texture':
						textures.add(name)
					else:
						vectorImages.add(name)
				elif resourceType in ('TextureTextIcons', 'VectorTextIcons'):
					textIcons.add(resource['name'])
					if resourceType == 'TextureTextIcons':
						iconType = 'texture'
						iconPool = textures
					else:
						iconType = 'vector image'
						iconPool = vectorImages
					iconGroups = resource['icons']
					try:
						for iconGroup in iconGroups:
							for icon in iconGroup:
								iconName = icon['icon']
								if iconName not in iconPool:
									raise Exception(
										'Icon ' + iconType + ' "' + iconName + '" not present.')
					except KeyError as e:
						raise Exception("Vector text icon doesn't contain element " + str(e) + '.')
					except (AttributeError, TypeError, ValueError):
						raise Exception(
							'Vector text icons "icons" must be an array of array of objects.')
				elif resourceType == 'FaceGroup':
					name = resource['name']
					faceNames = set()
					faceGroupFaces[name] = faceNames
					faces = resource['faces']
					try:
						for face in faces:
							faceName = face.get('name')
							if name is None:
								faceName = nameFromPath(face['path'])
							faceNames.add(faceName)
					except KeyError as e:
						raise Exception("Face group face doesn't contain element " + str(e) + '.')
					except (AttributeError, TypeError, ValueError):
						raise Exception('Face group "faces" must be an array of objects.')
				elif resourceType == 'Font':
					faceGroup = resource['faceGroup']
					if faceGroup not in faceGroupFaces:
						raise Exception('Face group "' + str(faceGroup) + '" not present.')
					availableFaces = faceGroupFaces[faceGroup]
					faces = resource['faces']
					try:
						for face in faces:
							if face not in availableFaces:
								raise Exception('Face "' + str(face) + '" not in face group "' +
									faceGroup + '"')
					except (AttributeError, TypeError, ValueError):
						raise Exception('Font "faces" must be an array of strings.')

					icons = resource.get('icons')
					if icons is not None and icons not in textIcons:
						raise Exception('Text icons "' + str(icons) + '" not present.')
		except KeyError:
			raise Exception("Vector resource doesn't contain element " + str(e) + '.')
		except (AttributeError, TypeError, ValueError):
			raise Exception('VectorResources "resources" must be an array of objects.')

	def loadJson(self, json, basePath = None):
		"""Loads from a string containing json data. See load() for expected json format."""
		self.load(json.loads(json), basePath)

	def loadStream(self, stream, basePath = None):
		"""Loads from a stream containing json data. See load() for expected json format."""
		self.load(json.load(stream), basePath)

	def loadFile(self, jsonFile):
		"""Loads from a json file. See load() for expected json format."""
		with open(jsonFile) as f:
			self.load(json.load(f), os.path.dirname(jsonFile))

	def save(self, outputPath, quiet = False, multithread = True):
		"""
		Saves the vector resources.

		This will create a directory named "<filename w/o extension>_resources" in order to hold
		the textures and font files. When moving the resources around, the file and directory
		should stay together.
		"""
		(root, filename) = os.path.split(outputPath)
		resourceDirName = os.path.splitext(filename)[0] + '_resources'
		resourceDir = os.path.join(root, resourceDirName)

		builder = flatbuffers.Builder(0)

		def createResourceDir():
			if not os.path.exists(resourceDir):
				os.makedirs(resourceDir)

		def createResourceData(filePathOrBytes, outputName, removeEmbedded = False):
			if outputName:
				if os.sep != '/':
					outputName = outputName.replace(os.sep, '/')
				pathOffset = builder.CreateString(outputName)
				FileReference.Start(builder)
				FileReference.AddPath(builder, pathOffset)
				return FileOrData.FileReference, FileReference.End(builder)
			else:
				if isinstance(filePathOrBytes, bytes):
					dataOffset = builder.CreateByteVector(filePathOrBytes)
				else:
					with open(filePathOrBytes, 'r+b') as dataFile:
						dataOffset = builder.CreateByteVector(dataFile.read())
					if removeEmbedded:
						os.remove(filePathOrBytes)
				RawData.Start(builder)
				RawData.AddData(builder, dataOffset)
				return FileOrData.RawData, RawData.End(builder)

		def convertTexture(texture):
			with tempfile.NamedTemporaryFile() as tempFile:
				try:
					path = texture['path']
					name = texture.get('name')
					if name is None:
						name = nameFromPath(path)
					container = texture.get('container', 'pvr')
					extension = '.' + container
					if texture.get('embed'):
						textureOutputPath = tempFile.name + extension
						outputName = None
					else:
						createResourceDir()
						outputName = os.path.join(resourceDirName, name + extension)
						textureOutputPath = os.path.join(root, outputName)

					commandLine = [self.cuttlefish, '-i', os.path.join(self.basePath, path),
						'-o', textureOutputPath, '-f', texture['format'], '-t',
						texture['channelType']]
					if quiet:
						commandLine.append('-q')
					if multithread:
						commandLine.append('-j')
					if texture.get('srgb'):
						commandLine.append('--srgb')
					size = texture.get('size')
					if size:
						commandLine.extend(['-r', size[0], size[1]])
					quality = texture.get('quality')
					if quality:
						commandLine.extend(['-Q', quality.lower()])
				except KeyError as e:
					raise Exception("Vector resource doesn't contain element " + str(e) + '.')

				if not quiet:
					print('Converting texture "' + path + '"...')
				sys.stdout.flush()

				try:
					subprocess.check_call(commandLine)
					dataType, dataOffset = createResourceData(textureOutputPath, outputName, True)
				except:
					if os.path.isfile(textureOutputPath):
						os.remove(textureOutputPath)
					raise

				TextureResource.Start(builder)
				TextureResource.AddDataType(builder, dataType)
				TextureResource.AddData(builder, dataOffset)
				textureOffset = TextureResource.End(builder)

				nameOffset = builder.CreateString(name)
				VectorResource.Start(builder)
				VectorResource.AddName(builder, nameOffset)
				VectorResource.AddResourceType(builder, VectorResourceUnion.TextureResource)
				VectorResource.AddResource(builder, textureOffset)
				return VectorResource.End(builder)

		def convertVectorImage(vectorImage):
			try:
				path = vectorImage['path']
				name = vectorImage.get('name')
				if name is None:
					name = nameFromPath(path)
				defaultFont = vectorImage.get('defaultFont', 'serif')

				targetSizeData = vectorImage.get('targetSize')
				try:
					if targetSizeData:
						targetSize = (float(targetSizeData[0]), float(targetSizeData[1]))
					else:
						targetSize = None
				except (AttributeError, TypeError, ValueError):
					raise Exception(
						'Vector image resource "targetSize" must be an array of two floats.')
			except KeyError as e:
				raise Exception("Vector resource doesn't contain element " + str(e) + '.')

			vectorImageBytes = convertSVG(path, name, defaultFont)
			if vectorImage.get('embed'):
				outputName = None
			else:
				outputName = os.path.join(resourceDirName, name + '.dsvi')
				outputPath = os.path.join(root, outputName)
				createResourceDir()
				with open(outputPath, 'wb') as f:
					f.write(vectorImageBytes)

			dataType, dataOffset = createResourceData(vectorImageBytes, outputName)
			VectorImageResource.Start(builder)
			VectorImageResource.AddDataType(builder, dataType)
			VectorImageResource.AddData(builder, dataOffset)
			VectorImageResource.AddTargetSize(builder,
				CreateVector2f(builder, *targetSize) if targetSize else 0)
			vectorImageOffset = VectorImageResource.End(builder)

			nameOffset = builder.CreateString(name)
			VectorResource.Start(builder)
			VectorResource.AddName(builder, nameOffset)
			VectorResource.AddResourceType(builder, VectorResourceUnion.VectorImageResource)
			VectorResource.AddResource(builder, vectorImageOffset)
			return VectorResource.End(builder)

		def convertTextIcons(textIcons):
			try:
				name = textIcons['name']
				iconType = IconType.Texture if textIcons['type'] == 'TextureTextIcons' \
					else IconType.VectorImage
				iconGroupOffsets = []
				icons = textIcons['icons']
				try:
					for iconGroup in icons:
						iconOffsets = []
						for icon in iconGroup:
							codepoint = icon['codepoint']
							iconName = icon['icon']
							advance = icon['advance']
							bounds = icon['bounds']

							try:
								boundsMin = (float(bounds[0][0]), float(bounds[0][1]))
								boundsMax = (float(bounds[1][0]), float(bounds[1][1]))
							except (AttributeError, TypeError, ValueError):
								raise Exception('Text icon "bounds" must be a 2x2 array of floats.')

							iconNameOffset = builder.CreateString(iconName)
							TextIcon.Start(builder)
							TextIcon.AddCodepoint(builder, codepoint)
							TextIcon.AddIcon(builder, iconNameOffset)
							TextIcon.AddAdvance(builder, advance)
							TextIcon.AddBoundsMin(builder, CreateVector2f(builder, *boundsMin))
							TextIcon.AddBoundsMax(builder, CreateVector2f(builder, *boundsMax))
							iconOffsets.append(TextIcon.End(builder))

						TextIconGroup.StartIconsVector(builder, len(iconOffsets))
						for offset in reversed(iconOffsets):
							builder.PrependUOffsetTRelative(offset)
						iconsOffset = builder.EndVector()

						TextIconGroup.Start(builder)
						TextIconGroup.AddIcons(builder, iconsOffset)
						iconGroupOffsets.append(TextIconGroup.End(builder))
				except KeyError as e:
					raise Exception("Vector text icon doesn't contain element " + str(e) + '.')
				except (AttributeError, TypeError, ValueError):
					raise Exception(
						'Vector text icons "icons" must be an array of array of objects.')
			except KeyError as e:
				raise Exception("Vector resource doesn't contain element " + str(e) + '.')

			TextIcons.StartIconsVector(builder, len(iconGroupOffsets))
			for offset in reversed(iconGroupOffsets):
				builder.PrependUOffsetTRelative(offset)
			iconGroupsOffset = builder.EndVector()

			nameOffset = builder.CreateString(name)
			TextIcons.Start(builder)
			TextIcons.AddName(builder, nameOffset)
			TextIcons.AddType(builder, iconType)
			TextIcons.AddIcons(builder, iconGroupsOffset)
			return TextIcons.End(builder)

		def convertFaceGroup(faceGroup):
			try:
				name = faceGroup['name']
				faces = faceGroup['faces']
				faceOffsets = []
				try:
					for face in faces:
						faceName = face['name']
						path = face['path']
						if face.get('embed'):
							outputName = None
							fontOutputPath = os.path.join(self.basePath, path)
						else:
							outputName = os.path.join(resourceDirName,
								faceName + os.path.splitext(path)[1])
							fontOutputPath = os.path.join(root, outputName)
							createResourceDir()
							shutil.copyfile(os.path.join(self.basePath, path), fontOutputPath)

						faceNameOffset = builder.CreateString(faceName)
						dataType, dataOffset = createResourceData(fontOutputPath, outputName)

						FontFace.Start(builder)
						FontFace.AddName(builder, faceNameOffset)
						FontFace.AddDataType(builder, dataType)
						FontFace.AddData(builder, dataOffset)
						faceOffsets.append(FontFace.End(builder))
				except KeyError as e:
					raise Exception("Face group face doesn't contain element " + str(e) + '.')
				except (AttributeError, TypeError, ValueError):
					raise Exception('Face group "faces" must be an array of objects.')
			except KeyError as e:
				raise Exception("Vector resource doesn't contain element " + str(e) + '.')

			FaceGroup.StartFacesVector(builder, len(faceOffsets))
			for offset in reversed(faceOffsets):
				builder.PrependUOffsetTRelative(offset)
			facesOffset = builder.EndVector()

			FaceGroup.Start(builder)
			FaceGroup.AddFaces(builder, facesOffset)
			faceGroupOffset = FaceGroup.End(builder)

			nameOffset = builder.CreateString(name)
			VectorResource.Start(builder)
			VectorResource.AddName(builder, nameOffset)
			VectorResource.AddResourceType(builder, VectorResourceUnion.FaceGroup)
			VectorResource.AddResource(builder, faceGroupOffset)
			return VectorResource.End(builder)

		qualityValues = {'low': FontQuality.Low, 'medium': FontQuality.Medium,
			'high': FontQuality.High, 'veryhigh': FontQuality.VeryHigh}
		cacheValues = {'small': FontCacheSize.Small, 'large': FontCacheSize.Large}

		def convertFont(font):
			try:
				name = font['name']
				faceGroup = font['faceGroup']

				faces = font['faces']
				faceOffsets = []
				try:
					for face in faces:
						faceOffsets.append(builder.CreateString(face))
				except (AttributeError, TypeError, ValueError):
					raise Exception('Font "faces" must be an array of strings.')

				qualityStr = str(font['quality'])
				cacheSizeStr = str(font.get('cacheSize', 'large'))

				try:
					quality = qualityValues[qualityStr.lower()]
				except KeyError:
					raise Exception('Unknown font quality "' + qualityStr + '".')

				try:
					cacheSize = cacheValues[cacheSizeStr.lower()]
				except KeyError:
					raise Exception('Unknown cache size quality "' + cacheSizeStr + '".')
			except KeyError as e:
				raise Exception("Vector resource doesn't contain element " + str(e) + '.')

			Font.StartFacesVector(builder, len(faceOffsets))
			for offset in reversed(faceOffsets):
				builder.PrependUOffsetTRelative(offset)
			facesOffset = builder.EndVector()

			faceGroupOffset = builder.CreateString(faceGroup)
			Font.Start(builder)
			Font.AddFaceGroup(builder, faceGroupOffset)
			Font.AddFaces(builder, facesOffset)
			Font.AddQuality(builder, quality)
			Font.AddCacheSize(builder, cacheSize)
			fontOffset = Font.End(builder)

			nameOffset = builder.CreateString(name)
			VectorResource.Start(builder)
			VectorResource.AddName(builder, nameOffset)
			VectorResource.AddResourceType(builder, VectorResourceUnion.Font)
			VectorResource.AddResource(builder, fontOffset)
			return VectorResource.End(builder)

		resourceOffsets = []
		for resource in self.resources:
			resourceType = resource['type']
			if resourceType == 'Texture':
				resourceOffsets.append(convertTexture(resource))
			elif resourceType == 'VectorImage':
				resourceOffsets.append(convertVectorImage(resource))
			elif resourceType in ('TextureTextIcons', 'VectorTextIcons'):
				resourceOffsets.append(convertTextIcons(resource))
			elif resourceType == 'FaceGroup':
				resourceOffsets.append(convertFaceGroup(resource))
			elif resourceType == 'Font':
				resourceOffsets.append(convertFont(resource))
			else:
				raise Exception('Unknown vector resource type "' + resourceType + '".')

		VectorResourcesFB.StartResourcesVector(builder, len(resourceOffsets))
		for offset in reversed(resourceOffsets):
			builder.PrependUOffsetTRelative(offset)
		resourcesOffset = builder.EndVector()

		VectorResourcesFB.Start(builder)
		VectorResourcesFB.AddResources(builder, resourcesOffset)
		builder.Finish(VectorResourcesFB.End(builder))

		with open(outputPath, 'wb') as f:
			f.write(builder.Output())

if __name__ == '__main__':
	parser = argparse.ArgumentParser(description =
		'Create vector resources to be used by Deep Sea.')
	parser.add_argument('-i', '--input', required = True,
		help = 'input json description of the resources')
	parser.add_argument('-o', '--output', required = True,
		help = 'output file name, typically with the extension ".dsvr"')
	parser.add_argument('-c', '--cuttlefish', default = 'cuttlefish',
		help = 'path to the cuttlefish tool for texture conversion')
	parser.add_argument('-j', '--multithread', default = False, action = 'store_true',
		help = 'multithread texture conversion')

	args = parser.parse_args()

	try:
		resources = VectorResources(args.cuttlefish)
		resources.loadFile(args.input)
		resources.save(args.output)
	except Exception as e:
		print(args.input + ': error: ' + str(e), file=sys.stderr)
		exit(1)
