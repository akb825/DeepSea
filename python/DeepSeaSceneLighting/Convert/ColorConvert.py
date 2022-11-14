# Copyright 2022 Aaron Barany
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

import math

def readFloat(value, name, minVal = None, maxVal = None):
	try:
		floatVal = float(value)
		if (minVal is not None and floatVal < minVal) or \
				(maxVal is not None and floatVal > maxVal):
			raise Exception() # Common error handling in except block.
		return floatVal
	except:
		raise Exception('Invalid ' + name + ' value "' + str(value) + '".')

def readColor(value, name, srgb):
	if not isinstance(value, list) or len(value) != 3:
		raise Exception('SceneLight ' + name + ' must be an array of three floats.')

	color = [readFloat(value[0], name + ' red channel'),
		readFloat(value[1], name + ' green channel'),
		readFloat(value[2], name + ' blue channel')]
	if srgb:
		for i in range(0, 3):
			if color[i] <= 0.04045:
				color[i] = color[i]/12.92
			else:
				color[i] = pow((color[i] + 0.055)/1.055, 2.4)
	return color
