#!/usr/bin/env python
# Copyright 2018-2020 Aaron Barany
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
import locale
import os

from DeepSeaVectorDraw.Convert.SVG import convertSVG

if __name__ == '__main__':
	# Avoid parsing issues due to locale.
	locale.setlocale(locale.LC_ALL, 'C')

	parser = argparse.ArgumentParser(description =
		'Convert an SVG to a vector image to be used by Deep Sea.')
	parser.add_argument('-i', '--input', required = True,
		help = 'input svg to convert')
	parser.add_argument('-o', '--output', required = True,
		help = 'output file name, typically with the extension ".dsvi"')
	parser.add_argument('-f', '--default-font', default = 'serif',
		help = 'default font when none is specified')

	args = parser.parse_args()
	name = os.path.splitext(os.path.basename(args.output))[0]
	outputData = convertSVG(args.input, name, args.default_font)

	with open(args.output, 'wb') as f:
		f.write(outputData)
