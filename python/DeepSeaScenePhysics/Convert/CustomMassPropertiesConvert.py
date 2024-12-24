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
			raise Exception('ShiftedMass rotation point shift must be an array of three floats.')

		rotationPointShift = (readFloat(rotationPointShiftData[0], "rotation point shift"),
			readFloat(rotationPointShiftData[1], "rotation point shift"),
			readFloat(rotationPointShiftData[2], "rotation point shift"))
		mass = readFloat(data.get('mass', -1), 'mass', 0)

		ShiftedMass.Start(builder)
		ShiftedMass.AddRotationPointShift(builder, CreateVector3f(builder, *rotationPointShift))
		ShiftedMass.AddMass(builder, mass)
		return CustomMassProperties.ShiftedMass, ShiftedMass.End(builder)
	elif centeredInertiaData:
		if not isinstance(centeredInertiaData, list) or len(centeredInertiaData) != 3:
			raise Exception('MassProperties centered inertia must be a 3x3 array of floats.')
		for arr in centeredInertiaData:
			if not isinstance(arr, list) or len(arr) != 3:
				raise Exception('MassProperties centered inertia must be a 3x3 array of floats.')

		centeredInertia = (readFloat(centeredInertiaData[0][0], 'centered inertia data'),
			readFloat(centeredInertiaData[0][1], 'centered inertia data'),
			readFloat(centeredInertiaData[0][2], 'centered inertia data'),
			readFloat(centeredInertiaData[1][0], 'centered inertia data'),
			readFloat(centeredInertiaData[1][1], 'centered inertia data'),
			readFloat(centeredInertiaData[1][2], 'centered inertia data'),
			readFloat(centeredInertiaData[2][0], 'centered inertia data'),
			readFloat(centeredInertiaData[2][1], 'centered inertia data'),
			readFloat(centeredInertiaData[2][2], 'centered inertia data'))

		centerOfMassData = data.get('centerOfMass')
		if centerOfMassData:
			if not isinstance(centerOfMassData, list) or len(centerOfMassData) != 3:
				raise Exception('MassProperties center of mass must be an array of three floats.')

			centerOfMass = (readFloat(centerOfMassData[0], 'center of mass'),
				readFloat(centerOfMassData[1], 'center of mass'),
				readFloat(centerOfMassData[2], 'center of mass'))
		else:
			centerOfMass = None

		mass = readFloat(data['mass'], 'mass', 0)

		inertiaTranslateData = data.get('inertiaTranslate')
		if inertiaTranslateData:
			if not isinstance(inertiaTranslateData, list) or len(inertiaTranslateData) != 3:
				raise Exception(
					'MassProperties inertia translate must be an array of three floats.')

			inertiaTranslate = (readFloat(inertiaTranslateData[0], 'inertia translate'),
				readFloat(inertiaTranslateData[1], 'inertia translate'),
				readFloat(inertiaTranslateData[2], 'inertia translate'))
		else:
			inertiaTranslate = None

		inertiaRotateData = data.get('inertiaRotate')
		if inertiaRotateData:
			if not isinstance(inertiaRotateData, list) or len(inertiaRotateData) != 3:
				raise Exception(
					'MassProperties inertia rotate must be an array of three floats.')

			inertiaRotate = eulerToQuaternion(readFloat(inertiaRotateData[0], 'inertia rotate'),
				readFloat(inertiaRotateData[1], 'inertia rotate'),
				readFloat(inertiaRotateData[2], 'inertia rotate'))
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
