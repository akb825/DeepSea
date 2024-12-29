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

import flatbuffers
from .Helpers import eulerToQuaternion, readFloat
from DeepSeaPhysics.CustomMassProperties import CustomMassProperties
from DeepSeaPhysics.Matrix33f import CreateMatrix33f
from DeepSeaPhysics.Quaternion4f import CreateQuaternion4f
from DeepSeaPhysics.Vector3f import CreateVector3f
from DeepSeaPhysics import MassProperties
from DeepSeaPhysics import ShiftedMass

def convertCustomMassProperties(convertContext, data, builder):
	if not data:
		return CustomMassProperties.NONE, 0

	rotationPointShiftData = data.get('rotationPointShift')
	centeredInertiaData = data.get('centeredInertia')
	if rotationPointShiftData:
		if not isinstance(rotationPointShiftData, list) or len(rotationPointShiftData) != 3:
			raise Exception('ShiftedMass "rotationPointShift" must be an array of three floats.')

		rotationPointShift = (readFloat(value, "rotation point shift")
			for value in rotationPointShiftData)
		mass = readFloat(data.get('mass', -1), 'mass', 0)

		ShiftedMass.Start(builder)
		ShiftedMass.AddRotationPointShift(builder, CreateVector3f(builder, *rotationPointShift))
		ShiftedMass.AddMass(builder, mass)
		return CustomMassProperties.ShiftedMass, ShiftedMass.End(builder)
	elif centeredInertiaData:
		if not isinstance(centeredInertiaData, list) or len(centeredInertiaData) != 3:
			raise Exception('MassProperties "centeredInertia" must be a 3x3 array of floats.')
		for arr in centeredInertiaData:
			if not isinstance(arr, list) or len(arr) != 3:
				raise Exception('MassProperties "centeredInertia" must be a 3x3 array of floats.')

		centeredInertiaList = []
		for i in range(0, 3):
			for j in range(0, 3):
				centeredInertiaList.append(
					readFloat(centeredInertiaData[i][j], 'centered inertia data'))
		centeredInertia = tuple(centeredInertiaList)

		centerOfMassData = data.get('centerOfMass')
		if centerOfMassData:
			if not isinstance(centerOfMassData, list) or len(centerOfMassData) != 3:
				raise Exception('MassProperties "centerOfMass" must be an array of three floats.')

			centerOfMass = (readFloat(value, 'center of mass') for value in centerOfMassData)
		else:
			centerOfMass = None

		mass = readFloat(data['mass'], 'mass', 0)

		inertiaTranslateData = data.get('inertiaTranslate')
		if inertiaTranslateData:
			if not isinstance(inertiaTranslateData, list) or len(inertiaTranslateData) != 3:
				raise Exception(
					'MassProperties "inertiaTranslate" must be an array of three floats.')

			inertiaTranslate = (readFloat(value, 'inertia translate')
				for value in inertiaTranslateData)
		else:
			inertiaTranslate = None

		inertiaRotateData = data.get('inertiaRotate')
		if inertiaRotateData:
			if not isinstance(inertiaRotateData, list) or len(inertiaRotateData) != 3:
				raise Exception(
					'MassProperties "inertiaRotate" must be an array of three floats.')

			inertiaRotate = eulerToQuaternion(*(readFloat(value, 'inertia rotate')
				for value in inertiaRotateData))
		else:
			inertiaRotate = None

		MassProperties.Start(builder)
		MassProperties.AddCenteredInertia(builder, CreateMatrix33f(builder, *centeredInertia))
		MassProperties.AddCenterOfMass(builder,
			CreateVector3f(builder, *centerOfMass) if centerOfMass else 0)
		MassProperties.AddMass(builder, mass)
		MassProperties.AddInertiaTranslate(builder,
			CreateVector3f(builder, *inertiaTranslate) if inertiaTranslate else 0)
		MassProperties.AddInertiaRotate(builder,
			CreateQuaternion4f(builder, *inertiaRotate) if inertiaRotate else 0)
		return CustomMassProperties.MassProperties, MassProperties.End(builder)
	else:
		raise KeyError('rotationPointShift or centeredInertia')
