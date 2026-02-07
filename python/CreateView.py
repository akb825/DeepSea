#!/usr/bin/env python
# Copyright 2020-2026 Aaron Barany
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
import sys
from importlib import import_module

from DeepSeaScene.Convert.ConvertContext import ConvertContext
from DeepSeaScene.Convert.ViewConvert import convertView

def createViewConvertContext(customExtensions=None):
	"""
	Creates a ConvertContext for views with the default set of extensions.

	:param customExtensions: List of custom extensions to add, which will be loaded as modules.
	"""
	convertContext = ConvertContext()
	if customExtensions:
		for extension in customExtensions:
			import_module(extension).deepSeaSceneExtension(convertContext)

	return convertContext

if __name__ == '__main__':
	parser = argparse.ArgumentParser(description =
		'Create a vew to be used by Deep Sea.')
	parser.add_argument('-i', '--input', required = True,
		help = 'input json description of the resources')
	parser.add_argument('-d', '--input-directory', help = 'explicit directory to use for relatve '
		"paths; defaults to the input file's directory")
	parser.add_argument('-o', '--output', required = True,
		help = 'output file name, typically with the extension ".dsv"')
	parser.add_argument('-e', '--extensions', nargs = '*', default = [],
		help = 'list of module names for extensions. Eeach extension should have a '
			'deepSeaSceneExtension(convertContext) function to register the custom types with the'
			'convert context.')

	args = parser.parse_args()
	convertContext = createViewConvertContext(args.extensions)

	if args.input_directory:
		inputDir = args.input_directory
	else:
		inputDir = os.path.dirname(args.input)

	try:
		with open(args.input) as f:
			data = json.load(f)

		with open(args.output, 'wb') as f:
			f.write(convertView(convertContext, data, inputDir))
	except Exception as e:
		print(args.input + ': error: ' + str(e), file=sys.stderr)
		exit(1)
