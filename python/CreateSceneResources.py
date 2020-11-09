#!/usr/bin/env python
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

from __future__ import print_function
import argparse
import json
import sys
from importlib import import_module

from DeepSeaScene.Convert.ConvertContext import ConvertContext
from DeepSeaScene.Convert.SceneResourcesConvert import convertSceneResources

from DeepSeaSceneLighting.Convert.LightSetConvert import convertLightSet

from DeepSeaVectorDrawScene.Convert.TextConvert import convertText
from DeepSeaVectorDrawScene.Convert.TextNodeConvert import convertTextNode
from DeepSeaVectorDrawScene.Convert.VectorImageConvert import convertVectorImage
from DeepSeaVectorDrawScene.Convert.VectorImageNodeConvert import convertVectorImageNode
from DeepSeaVectorDrawScene.Convert.VectorResourcesConvert import convertVectorResources
from DeepSeaVectorDrawScene.Convert.VectorShadersConvert import convertVectorShaders

if __name__ == '__main__':
	parser = argparse.ArgumentParser(description =
		'Create scene resources to be used by Deep Sea.')
	parser.add_argument('-i', '--input', required = True,
		help = 'input json description of the resources')
	parser.add_argument('-o', '--output', required = True,
		help = 'output file name, typically with the extension ".dssr"')
	parser.add_argument('-c', '--cuttlefish', default = 'cuttlefish',
		help = 'path to the cuttlefish tool for texture conversion')
	parser.add_argument('-v', '--vfc', default = 'vfc',
		help = 'path to the vfc tool for vertex format conversion')
	parser.add_argument('-j', '--multithread', default = False, action = 'store_true',
		help = 'multithread texture conversion')
	parser.add_argument('-e', '--extensions', nargs = '*', default = [],
		help = 'list of module names for extensions. Eeach extension should have a '
			'deepSeaSceneExtension(convertContext) function to register the custom types with the'
			'convert context.')

	args = parser.parse_args()
	convertContext = ConvertContext(args.cuttlefish, args.vfc, args.multithread)

	# Lighting scene types.
	convertContext.addCustomResourceType('LightSet', convertLightSet)

	# Vector draw scene types.
	convertContext.addNodeType('TextNode', convertTextNode)
	convertContext.addNodeType('VectorImageNode', convertVectorImageNode)
	convertContext.addCustomResourceType('Text', convertText)
	convertContext.addCustomResourceType('VectorImage', convertVectorImage)
	convertContext.addCustomResourceType('VectorResources', convertVectorResources)
	convertContext.addCustomResourceType('VectorShaders', convertVectorShaders)

	for extension in args.extensions:
		import_module(extension).deepSeaSceneExtension(convertContext)

	try:
		with open(args.input) as f:
			data = json.load(f)

		with open(args.output, 'wb') as f:
			f.write(convertSceneResources(convertContext, data))
	except Exception as e:
		print(args.input + ': error: ' + str(e), file=sys.stderr)
		exit(1)
