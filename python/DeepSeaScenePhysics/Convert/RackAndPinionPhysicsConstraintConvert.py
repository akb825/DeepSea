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

import math

import flatbuffers
from .Helpers import eulerToQuaternion, readFloat
from DeepSeaPhysics.ConstraintUnion import ConstraintUnion
from DeepSeaPhysics.Vector3f import CreateVector3f
from DeepSeaPhysics import Constraint
from DeepSeaPhysics import RackAndPinionConstraint

def convertRackAndPinionPhysicsConstraint(convertContext, data, outputDir):
	"""
	Converts a RackAndPinionPhysicsConstraint. The data map is expected to contain the following
	elements:
	- rackActor: the name of the rack actor used in the constraint. This may be unset if the
	  actor will be provided later.
	- rackAxis: array of 3 floats for the axis of translation of the rack actor.
	- rackConstraint: the name of the slider constraint for the rack actor. This may be unset if
	  the constraint won't be set or will be provided later.
	- rackToothCount: the number of teeth for the rack actorr to compute the gear ratio. This may be
	  negative if it is flipped.
	- rackLength: the length of the rack to compute the gear ratio.
	- pinionActor: the name of the pinion actor used in the constraint. This may be unset if the
	  actor will be provided later.
	- pinionAxis: array of 3 floats for the axis of rotation of the pinion actor.
	- pinionConstraint: the name of the revolute constraint for the pinion actor. This may be unset
	  if the constraint won't be set or will be provided later.
	- pinionToothCount: the number of teeth for the pinion actor to compute the gear ratio. This may
	  be negative if it is flipped.
	"""
	try:
		rackActor = str(data.get('rackActor', ''))

		rackAxisData = data['rackAxis']
		if not isinstance(rackAxisData, list) or len(rackAxisData) != 3:
			raise Exception(
				'RackAndPinionPhysicsConstraint "rackAxis" must be an array of three floats.')
		rackAxis = (readFloat(value, 'rack position') for value in rackAxisData)

		rackConstraint = str(data.get('rackConstraint', ''))
		rackToothCount = readFloat(data['rackToothCount'], 'rack tooth count')
		rackLength = readFloat(data['rackLengtht'], 'rack length', 0)

		pinionActor = str(data.get('pinionActor', ''))

		pinionAxisData = data['pinionAxis']
		if not isinstance(pinionAxisData, list) or len(pinionAxisData) != 3:
			raise Exception(
				'RackAndPinionPhysicsConstraint "pinionAxis" must be an array of three floats.')
		pinionAxis = (readFloat(value, 'pinion position') for value in pinionAxisData)

		pinionConstraint = str(data.get('pinionConstraint', ''))
		pinionToothCount = readFloat(data['pinionToothCount'], 'pinion tooth count')
	except (TypeError, ValueError):
		raise Exception('RackAndPinionPhysicsConstraint data must be an object.')
	except KeyError as e:
		raise Exception(
			'RackAndPinionPhysicsConstraint data doesn\'t contain element ' + str(e) + '.')

	builder = flatbuffers.Builder(0)

	rackActorOffset = builder.CreateString(rackActor) if rackActor else 0
	rackConstraintOffset = builder.CreateString(rackConstraint) if rackConstraint else 0
	pinionActorOffset = builder.CreateString(pinionActor) if pinionActor else 0
	pinionConstraintOffset = builder.CreateString(pinionConstraint) if pinionConstraint else 0

	RackAndPinionConstraint.Start(builder)
	RackAndPinionConstraint.AddRackActor(builder, rackActorOffset)
	RackAndPinionConstraint.AddRackAxis(builder, CreateVector3f(builder, *rackAxis))
	RackAndPinionConstraint.AddRackConstraint(builder, rackConstraintOffset)
	RackAndPinionConstraint.AddPinionActor(builder, pinionActorOffset)
	RackAndPinionConstraint.AddPinionAxis(builder, CreateVector3f(builder, *pinionAxis))
	RackAndPinionConstraint.AddPinionConstraint(builder, pinionConstraintOffset)
	RackAndPinionConstraint.AddRatio(builder,
		2*math.pi*rackToothCount/(rackLength*pinionToothCount))
	constraintOffset = RackAndPinionConstraint.End(builder)

	Constraint.Start(builder)
	Constraint.AddConstraintType(builder, ConstraintUnion.RackAndPinionConstraint)
	Constraint.AddConstraint(builder, constraintOffset)
	builder.Finish(Constraint.End(builder))
	return builder.Output()
