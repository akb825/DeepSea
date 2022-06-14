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

import flatbuffers
import math

def convertTransform(data, objectName, elementName):
	"""
	Converts a list of transforms into a matrix. The data is an array of objects with the following
	elements:
	- type: the type of transform. May be Rotate, Scale, Translate, or Matrix.
	- value: the value of transform based on the type:
	  - Rotate: array of 3 floats for the X, Y, and Z rotation in degrees.
	  - Scale: array of 3 floats for the scale value along the X, Y, and Z axes.
	  - Translate: array of 3 floats for the translation along X, Y, and Z.
	  - Matrix: a 4x4 array of floats for a matrix. Each inner array is a column of the matrix.
	"""
	def multiplyMatrix(a, b):
		return (
			(
				a[0][0]*b[0][0] + a[1][0]*b[0][1] + a[2][0]*b[0][2] + a[3][0]*b[0][3],
				a[0][1]*b[0][0] + a[1][1]*b[0][1] + a[2][1]*b[0][2] + a[3][1]*b[0][3],
				a[0][2]*b[0][0] + a[1][2]*b[0][1] + a[2][2]*b[0][2] + a[3][2]*b[0][3],
				a[0][3]*b[0][0] + a[1][3]*b[0][1] + a[2][3]*b[0][2] + a[3][3]*b[0][3]
			),
			(
				a[0][0]*b[1][0] + a[1][0]*b[1][1] + a[2][0]*b[1][2] + a[3][0]*b[1][3],
				a[0][1]*b[1][0] + a[1][1]*b[1][1] + a[2][1]*b[1][2] + a[3][1]*b[1][3],
				a[0][2]*b[1][0] + a[1][2]*b[1][1] + a[2][2]*b[1][2] + a[3][2]*b[1][3],
				a[0][3]*b[1][0] + a[1][3]*b[1][1] + a[2][3]*b[1][2] + a[3][3]*b[1][3]
			),
			(
				a[0][0]*b[2][0] + a[1][0]*b[2][1] + a[2][0]*b[2][2] + a[3][0]*b[2][3],
				a[0][1]*b[2][0] + a[1][1]*b[2][1] + a[2][1]*b[2][2] + a[3][1]*b[2][3],
				a[0][2]*b[2][0] + a[1][2]*b[2][1] + a[2][2]*b[2][2] + a[3][2]*b[2][3],
				a[0][3]*b[2][0] + a[1][3]*b[2][1] + a[2][3]*b[2][2] + a[3][3]*b[2][3],
			),
			(
				a[0][0]*b[3][0] + a[1][0]*b[3][1] + a[2][0]*b[3][2] + a[3][0]*b[3][3],
				a[0][1]*b[3][0] + a[1][1]*b[3][1] + a[2][1]*b[3][2] + a[3][1]*b[3][3],
				a[0][2]*b[3][0] + a[1][2]*b[3][1] + a[2][2]*b[3][2] + a[3][2]*b[3][3],
				a[0][3]*b[3][0] + a[1][3]*b[3][1] + a[2][3]*b[3][2] + a[3][3]*b[3][3]
			)
		)

	def rotateMatrix(x, y, z):
		xRad = math.radians(x)
		yRad = math.radians(y)
		zRad = math.radians(z)

		sinX = math.sin(xRad)
		cosX = math.cos(xRad)
		sinY = math.sin(yRad)
		cosY = math.cos(yRad)
		sinZ = math.sin(zRad)
		cosZ = math.cos(zRad)

		return (
			(cosY*cosZ, cosY*sinZ, -sinY, 0.0),
			(sinX*sinY*cosZ - cosX*sinZ, cosX*cosZ + sinX*sinY*sinZ, sinX*cosY, 0.0),
			(sinX*sinZ + cosX*sinY*cosZ, cosX*sinY*sinZ - sinX*cosZ, cosX*cosY, 0.0),
			(0.0, 0.0, 0.0, 1.0)
		)

	def scaleMatrix(x, y, z):
		return (
			(x, 0.0, 0.0, 0.0),
			(0.0, y, 0.0, 0.0),
			(0.0, 0.0, z, 0.0),
			(0.0, 0.0, 0.0, 1.0)
		)

	def translateMatrix(x, y, z):
		return (
			(1.0, 0.0, 0.0, 0.0),
			(0.0, 1.0, 0.0, 0.0),
			(0.0, 0.0, 1.0, 0.0),
			(x, y, z, 1.0)
		)

	matrix = (
		(1.0, 0.0, 0.0, 0.0),
		(0.0, 1.0, 0.0, 0.0),
		(0.0, 0.0, 1.0, 0.0),
		(0.0, 0.0, 0.0, 1.0)
	)
	try:
		for transformInfo in data:
			transformType = str(transformInfo['type'])
			transformValue = transformInfo['value']
			if transformType == 'Rotate':
				try:
					if len(transformValue) != 3:
						raise Exception()

					transform = rotateMatrix(float(transformValue[0]), float(transformValue[1]),
						float(transformValue[2]))
				except:
					raise Exception('Rotate transform value must be an array of 3 floats.')
			elif transformType == 'Scale':
				try:
					if len(transformValue) != 3:
						raise Exception()

					transform = scaleMatrix(float(transformValue[0]), float(transformValue[1]),
						float(transformValue[2]))
				except:
					raise Exception('Scale transform value must be an array of 3 floats.')
			elif transformType == 'Translate':
				try:
					if len(transformValue) != 3:
						raise Exception()

					transform = translateMatrix(float(transformValue[0]),
						float(transformValue[1]), float(transformValue[2]))
				except:
					raise Exception('Translate transform value must be an array of 3 floats.')
			elif transformType == 'Matrix':
				try:
					if len(transformValue) != 4:
						raise Exception()

					for col in transformValue:
						if len(transformValue) != 4:
							raise Exception()

					transform = ((float(col[0]), float(col[1]), float(col[2]), float(col[3]))
						for col in transformValue)
				except:
					raise Exception('Matrix transform value must be a 4x4 array of floats.')
			else:
				raise Exception('Invalid transform type "' + transformType + '".')

			matrix = multiplyMatrix(matrix, transform)
		
		matrixValues = []
		for col in matrix:
			matrixValues.extend(col)
	except KeyError as e:
		raise Exception(
			objectName + ' "' + elementName + '" doesn\'t contain element ' + str(e) + '.')
	except (TypeError, ValueError):
		raise Exception(objectName + ' "' + elementName + '" must be an array of objects.')

	matrixValues = []
	for col in matrix:
		matrixValues.extend(col)
	return matrixValues
