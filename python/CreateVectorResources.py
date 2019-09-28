#!/usr/bin/python
# Copyright 2018 Aaron Barany
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

import flatbuffers
from DeepSeaVectorDraw.FaceGroup import *
from DeepSeaVectorDraw.Font import *
from DeepSeaVectorDraw.FontCacheSize import *
from DeepSeaVectorDraw.FontQuality import *
from DeepSeaVectorDraw.Resource import *
from DeepSeaVectorDraw.VectorResources import *

class VectorResources:
	"""Class containing the information for vector resources to be saved in FlatBuffer format."""
	def __init__(self, cuttlefishTool = 'cuttlefish'):
		"""
		Constructs this in the default state.

		cuttlefish is used to convert images into textures. By default it will check for
		"cuttlefish" on PATH, but the path may be passed into the constructor.
		"""

		self.cuttlefish = cuttlefishTool

	def load(self, stream, basePath = None):
		"""
		Loads the information for the VectorResources from a stream.

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
					"quality": "<lowest|low|normal|high|highest>" (optional)
					"container": "<pvr|dds|ktx>" (optional, defaults to pvr)
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
					"cacheSize": "<small|large>" (optional, defaults to large)
				}
			]
		}
		"""

		self.basePath = basePath
		contents = json.load(stream)
		if 'textures' in contents:
			self.textures = contents['textures']
		if 'faceGroups' in contents:
			self.faceGroups = contents['faceGroups']
		if 'fonts' in contents:
			self.fonts = contents['fonts']

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
					raise Exception('Face "' + face + '" not in face group "' + font['faceGroup'] +
						'"')

	def loadFile(self, jsonFile):
		"""Loads from a json file. See load() for expected json format."""
		with open(jsonFile) as f:
			self.load(f, os.path.dirname(jsonFile))

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
		if not os.path.exists(resourceDir):
			os.makedirs(resourceDir)

		builder = flatbuffers.Builder(0)

		textureOffsets = []
		for texture in self.textures:
			path = texture['path']
			if 'name' in texture:
				name = texture['name']
			else:
				name = os.path.splitext(os.path.basename(path))[0]
			if 'container' in texture:
				extension = '.' + texture['container']
			else:
				extension = '.pvr'
			outputName = os.path.join(resourceDirName, name + extension)
			textureOutputPath = os.path.join(root, outputName)

			commandLine = [self.cuttlefish, '-i', os.path.join(self.basePath, path),
				'-o', textureOutputPath, '-f', texture['format'], '-t', texture['type']]
			if quiet:
				commandLine.append('-q')
			if multithread:
				commandLine.append('-j')
			if 'srgb' in texture and texture['srgb']:
				commandLine.append('--srgb')
			if 'size' in texture:
				size = texture['size']
				commandLine.extend(['-r', size[0], size[1]])
			if 'quality' in texture:
				commandLine.extend(['-Q', texture['quality'].lower()])

			if not quiet:
				print('Converting texture "' + path + '"...')
			subprocess.check_call(commandLine)

			nameOffset = builder.CreateString(name)
			pathOffset = builder.CreateString(outputName.replace('\\', '/'))

			ResourceStart(builder)
			ResourceAddName(builder, nameOffset)
			ResourceAddPath(builder, pathOffset)
			textureOffsets.append(ResourceEnd(builder))

		VectorResourcesStartTexturesVector(builder, len(textureOffsets))
		for offset in reversed(textureOffsets):
			builder.PrependUOffsetTRelative(offset)
		texturesOffset = builder.EndVector(len(textureOffsets))

		faceGroupOffsets = []
		for faceGroup in self.faceGroups:
			nameOffset = builder.CreateString(faceGroup['name'])

			faces = faceGroup['faces']
			faceOffsets = []
			for face in faces:
				name = face['name']
				path = face['path']
				outputName = os.path.join(resourceDirName, name + os.path.splitext(extension)[1])
				fontOutputPath = os.path.join(root, outputName)
				shutil.copyfile(os.path.join(self.basePath, path), fontOutputPath)

				faceNameOffset = builder.CreateString(name)
				pathOffset = builder.CreateString(outputName.replace('\\', '/'))

				ResourceStart(builder)
				ResourceAddName(builder, faceNameOffset)
				ResourceAddPath(builder, pathOffset)
				faceOffsets.append(ResourceEnd(builder))

			FaceGroupStartFacesVector(builder, len(faceOffsets))
			for offset in reversed(faceOffsets):
				builder.PrependUOffsetTRelative(offset)
			facesOffset = builder.EndVector(len(faceOffsets))

			FaceGroupStart(builder)
			FaceGroupAddName(builder, nameOffset)
			FaceGroupAddFaces(builder, facesOffset)
			faceGroupOffsets.append(FaceGroupEnd(builder))

		VectorResourcesStartFaceGroupsVector(builder, len(faceGroupOffsets))
		for offset in reversed(faceGroupOffsets):
			builder.PrependUOffsetTRelative(offset)
		faceGroupsOffset = builder.EndVector(len(faceGroupOffsets))

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

			FontStartFacesVector(builder, len(faceOffsets))
			for offset in reversed(faceOffsets):
				builder.PrependUOffsetTRelative(offset)
			facesOffset = builder.EndVector(len(faceOffsets))

			FontStart(builder)
			FontAddName(builder, nameOffset)
			FontAddFaceGroup(builder, faceGroupOffset)
			FontAddFaces(builder, facesOffset)
			FontAddQuality(builder, qualityValues[font['quality'].lower()])
			if 'cacheSize' in font:
				cacheSize = cacheValues[font['cacheSize'].lower()]
			else:
				cacheSize = FontCacheSize.Large
			FontAddCacheSize(builder, cacheSize)
			fontOffsets.append(FontEnd(builder))

		VectorResourcesStartFontsVector(builder, len(fontOffsets))
		for offset in reversed(fontOffsets):
			builder.PrependUOffsetTRelative(offset)
		fontsOffset = builder.EndVector(len(fontOffsets))

		VectorResourcesStart(builder)
		VectorResourcesAddTextures(builder, texturesOffset)
		VectorResourcesAddFaceGroups(builder, faceGroupsOffset)
		VectorResourcesAddFonts(builder, fontsOffset)
		builder.Finish(VectorResourcesEnd(builder))

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
	resources = VectorResources(args.cuttlefish)
	resources.loadFile(args.input)
	resources.save(args.output)
