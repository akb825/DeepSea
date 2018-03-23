#!/usr/bin/python

from __future__ import print_function
import argparse
import json
import os
import shutil
import subprocess

import flatbuffers
from DeepSeaVectorDraw.FaceGroup import *
from DeepSeaVectorDraw.Font import *
from DeepSeaVectorDraw.Resource import *
from DeepSeaVectorDraw.ResourceSet import *

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
					"name": "<name used to reference the texture>",
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
					"quality": "<Low|Medium|High|VeryHigh>",
					"faces":
					[
						{
							"name": "<name used to reference the font>",
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
					"faceGroups": "<faceGroup name">,
					"faces":
					[
						"<font in faceGroup>",
						...
					]
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
			faceGroupFaces[name] = set(faceGroup['fonts'])
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

	def save(self, path, quiet = False, multithread = True):
		"""
		Saves the vector resources.

		This will create a directory named "<filename w/o extension>_resources" in order to hold
		the textures and font files. When moving the resources around, the file and directory
		should stay together.
		"""

		(root, filename) = os.path.split(path)
		resourceDirName = os.path.splitext(filename)[0] + '_resources'
		resourceDir = os.path.join(root, resourceDirName)
		if not os.path.exists(resourceDir):
			os.makedirs(resourceDir)

		builder = flatbuffers.Builder(0)
		ResourceSetStart(builder)
		ResourceSetStartTexturesVector(builder, len(self.textures))
		for texture in self.textures:
			name = texture['name']
			path = texture['path']
			if 'container' in texture:
				extension = '.' + texture['container']
			else:
				extension = '.pvr'
			outputName = os.path.join(resourceDirName, name + extension)
			outputPath = os.path.join(root, outputName)

			commandLine = [self.cuttlefish, '-i', os.path.join(self.basePath, path),
				'-o', outputPath, '-f', texture['format'], '-t', texture['type']]
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

			ResourceStart(builder)
			ResourceAddName(builder, builder.CreateString(name))
			ResourceAddPath(builder, builder.CreateString(outputName))
			ResourceEnd(builder)
		ResourceSetAddTextures(builder, builder.EndVector(len(self.textures)))

		qualityValues = {'low': 0, 'medium': 1, 'high': 2, 'veryhigh': 3}
		ResourceSetStartFaceGroupsVector(builder, len(self.faceGroups))
		for faceGroup in self.faceGroups:
			FaceGroupStart(builder)
			FaceGroupAddName(builder, builder.CreateString(faceGroup['name']))
			FaceGroupAddQuality(builder, qualityValues[faceGroup['quality'].lower()])

			faces = faceGroup['faces']
			FaceGroupStartFacesVector(builder, len(faces))
			for face in faces:
				name = face['name']
				path = face['path']
				outputName = os.path.join(resourceDirName, name + os.path.splitext(extension)[1])
				outputPath = os.path.join(root, outputName)
				shutil.copyfile(os.path.join(self.basePath, path), outputPath)

				ResourceStart(builder)
				ResourceAddName(builder, builder.CreateString(name))
				ResourceAddPath(builder, builder.CreateString(outputName))
				ResourceEnd(builder)
			FaceGroupAddFaces(builder, builder.EndVector(len(faces)))
			FaceGroupEnd(builder)
		ResourceSetAddFaceGroups(builder, builder.EndVector(len(self.faceGroups)))

		ResourceSetStartFontsVector(builder, len(self.fonts))
		for font in self.fonts:
			FontStart(builder)
			FontAddName(builder, builder.CreateString(font['name']))
			FontAddFaceGroup(builder, builder.CreateString(font['faceGroup']))

			faces = font['faces']
			FontStartFacesVector(builder, len(faces))
			for face in faces:
				builder.CreateString(face)
			FontAddFaces(builder, builder.EndVector(len(faces)))
			FontEnd(builder)
		ResourceSetAddFonts(builder, builder.EndVector(len(self.fonts)))

		builder.Finish(ResourceSetEnd(builder))

		with open(path, 'wb') as f:
			f.write(builder.Output)

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
