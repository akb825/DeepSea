#!/usr/bin/env python
# Copyright 2018-2025 Aaron Barany
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
from DeepSeaVectorDraw import FaceGroup
from DeepSeaVectorDraw.FileOrData import FileOrData
from DeepSeaVectorDraw import FileReference
from DeepSeaVectorDraw import Font
from DeepSeaVectorDraw.FontCacheSize import FontCacheSize
from DeepSeaVectorDraw.FontQuality import FontQuality
from DeepSeaVectorDraw import RawData
from DeepSeaVectorDraw import Resource
from DeepSeaVectorDraw import VectorResources as VectorResourcesFB

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

		json format:
		{
			"textures":
			[
				{
					"name": "<name used to reference the texture>" (optional, defaults to path
						filename without extension),
					"path": "<path to image>",
					"format": "<texture format; see cuttlefish help for details>",
					"type": "<texture channel type; see cuttlefish help for details>",
					"srgb": <true|false> (optional, defaults to false),
					"size": [<width>, <height>] (optional),
					"quality": "<lowest|low|normal|high|highest>" (optional),
					"container": "<pvr|dds|ktx>" (optional, defaults to pvr),
					"embed": <true|false to embed in resource file> (optional, defaults to false)
				},
				...
			],
			"faceGroups":
			[
				{
					"name": "<name used to reference the face group>",
					"faces":
					[
						{
							"name": "<name used to reference the font>" (optional),
							"path": "<path to font file>"
						},
						...
					]
				},
				...
			],
			"fonts":
			[
				{
					"name": "<name used to reference the font>",
					"faceGroup": "<faceGroup name">,
					"faces":
					[
						"<font in faceGroup>",
						...
					],
					"quality": "<low|medium|high|veryhigh>",
					"cacheSize": "<small|large>" (optional, defaults to large),
					"embed": <true|false to embed in resource file> (optional, defaults to false)
				}
			]
		}
		"""

		self.basePath = basePath
		self.textures = contents.get('textures', [])
		self.faceGroups = contents.get('faceGroups', [])
		self.fonts = contents.get('fonts', [])

		faceGroupFaces = {}
		for faceGroup in self.faceGroups:
			name = faceGroup['name']
			faceGroupFaces[name] = {}
			for face in faceGroup['faces']:
				faceGroupFaces[name][face['name']] = face['path']
		for font in self.fonts:
			if font['faceGroup'] not in faceGroupFaces:
				raise Exception('Face group "' + font['faceGroup'] + '" not present.')
			faces = faceGroupFaces[font['faceGroup']]
			for face in font['faces']:
				if face not in faces:
					raise Exception(
						'Face "' + face + '" not in face group "' + font['faceGroup'] + '"')

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

		def createResourceDir():
			if not os.path.exists(resourceDir):
				os.makedirs(resourceDir)

		def createResourceData(filePath, outputName, removeEmbedded = False):
			if outputName:
				if os.sep != '/':
					outputName = outputName.replace(os.sep, '.')
				pathOffset = builder.CreateString(outputName)
				FileReference.Start(builder)
				FileReference.AddPath(builder, pathOffset)
				return FileOrData.FileReference, FileReference.End(builder)
			else:
				with open(filePath, 'r+b') as dataFile:
					dataOffset = builder.CreateByteVector(dataFile.read())
				if removeEmbedded:
					os.remove(filePath)
				RawData.Start(builder)
				RawData.AddData(builder, dataOffset)
				return FileOrData.RawData, RawData.End(builder)

		builder = flatbuffers.Builder(0)

		textureOffsets = []
		with tempfile.NamedTemporaryFile() as tempFile:
			for texture in self.textures:
				path = texture['path']
				name = texture.get('name')
				if name is None:
					name = os.path.splitext(os.path.basename(path))[0]
				container = texture.get('container')
				if container:
					extension = '.' + container
				else:
					extension = '.pvr'
				if texture.get('embed'):
					textureOutputPath = tempFile.name + extension
					outputName = None
				else:
					createResourceDir()
					outputName = os.path.join(resourceDirName, name + extension)
					textureOutputPath = os.path.join(root, outputName)

				commandLine = [self.cuttlefish, '-i', os.path.join(self.basePath, path),
					'-o', textureOutputPath, '-f', texture['format'], '-t', texture['type']]
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

				if not quiet:
					print('Converting texture "' + path + '"...')
				sys.stdout.flush()

				try:
					subprocess.check_call(commandLine)
					nameOffset = builder.CreateString(name)
					dataType, dataOffset = createResourceData(textureOutputPath, outputName, True)
				except:
					if os.path.isfile(textureOutputPath):
						os.remove(textureOutputPath)
					raise

				Resource.Start(builder)
				Resource.AddName(builder, nameOffset)
				Resource.AddDataType(builder, dataType)
				Resource.AddData(builder, dataOffset)
				textureOffsets.append(Resource.End(builder))

		VectorResourcesFB.StartTexturesVector(builder, len(textureOffsets))
		for offset in reversed(textureOffsets):
			builder.PrependUOffsetTRelative(offset)
		texturesOffset = builder.EndVector()

		faceGroupOffsets = []
		for faceGroup in self.faceGroups:
			nameOffset = builder.CreateString(faceGroup['name'])

			faces = faceGroup['faces']
			faceOffsets = []
			for face in faces:
				name = face['name']
				path = face['path']
				if face.get('embed'):
					outputName = None
					fontOutputPath = os.path.join(self.basePath, path)
				else:
					outputName = os.path.join(resourceDirName,
						name + os.path.splitext(path)[1])
					fontOutputPath = os.path.join(root, outputName)
					createResourceDir()
					shutil.copyfile(os.path.join(self.basePath, path), fontOutputPath)

				faceNameOffset = builder.CreateString(name)
				dataType, dataOffset = createResourceData(fontOutputPath, outputName)

				Resource.Start(builder)
				Resource.AddName(builder, faceNameOffset)
				Resource.AddDataType(builder, dataType)
				Resource.AddData(builder, dataOffset)
				faceOffsets.append(Resource.End(builder))

			FaceGroup.StartFacesVector(builder, len(faceOffsets))
			for offset in reversed(faceOffsets):
				builder.PrependUOffsetTRelative(offset)
			facesOffset = builder.EndVector()

			FaceGroup.Start(builder)
			FaceGroup.AddName(builder, nameOffset)
			FaceGroup.AddFaces(builder, facesOffset)
			faceGroupOffsets.append(FaceGroup.End(builder))

		VectorResourcesFB.StartFaceGroupsVector(builder, len(faceGroupOffsets))
		for offset in reversed(faceGroupOffsets):
			builder.PrependUOffsetTRelative(offset)
		faceGroupsOffset = builder.EndVector()

		fontOffsets = []
		qualityValues = {'low': FontQuality.Low, 'medium': FontQuality.Medium, \
			'high': FontQuality.High, 'veryhigh': FontQuality.VeryHigh}
		cacheValues = {'small': FontCacheSize.Small, 'large': FontCacheSize.Large}
		for font in self.fonts:
			nameOffset = builder.CreateString(font['name'])
			faceGroupOffset = builder.CreateString(font['faceGroup'])

			faces = font['faces']
			faceOffsets = []
			for face in faces:
				faceOffsets.append(builder.CreateString(face))

			Font.StartFacesVector(builder, len(faceOffsets))
			for offset in reversed(faceOffsets):
				builder.PrependUOffsetTRelative(offset)
			facesOffset = builder.EndVector()

			Font.Start(builder)
			Font.AddName(builder, nameOffset)
			Font.AddFaceGroup(builder, faceGroupOffset)
			Font.AddFaces(builder, facesOffset)
			Font.AddQuality(builder, qualityValues[font['quality'].lower()])
			cacheSize = font.get('cacheSize')
			if cacheSize:
				cacheSize = cacheValues[cacheSize.lower()]
			else:
				cacheSize = FontCacheSize.Large
			Font.AddCacheSize(builder, cacheSize)
			fontOffsets.append(Font.End(builder))

		VectorResourcesFB.StartFontsVector(builder, len(fontOffsets))
		for offset in reversed(fontOffsets):
			builder.PrependUOffsetTRelative(offset)
		fontsOffset = builder.EndVector()

		VectorResourcesFB.Start(builder)
		VectorResourcesFB.AddTextures(builder, texturesOffset)
		VectorResourcesFB.AddFaceGroups(builder, faceGroupsOffset)
		VectorResourcesFB.AddFonts(builder, fontsOffset)
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
