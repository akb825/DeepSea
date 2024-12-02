# Copyright 2024 Aaron Barany
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

def readFloat(value, name, minValue = None):
	try:
		floatVal = float(value)
		if minValue is not None and floatVal < minValue:
			raise Exception()
		return floatVal
	except:
		raise Exception('Invalid ' + name + ' value "' + str(value) + '".')

def readInt(value, name, minValue = None):
	try:
		intVal = int(value)
		if minValue is not None and intVal < minValue:
			raise Exception()
		return intVal
	except:
		raise Exception('Invalid ' + name + ' value "' + str(value) + '".')
