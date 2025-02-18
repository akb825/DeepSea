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
from .. import ActorResourceReference
from .. import ConstraintNode
from .. import ConstraintNodeReference
from .. import ConstraintResourceReference
from .. import InstanceReference 
from ..ActorReference import ActorReference
from ..ConstraintReference import ConstraintReference

def convertPhysicsConstraintNode(convertContext, data, outputDir):
	"""
	Converts a PhysicsConstraintNode. The data map is expected to contain the following elements:
	- constraint: the name of the base constraint.
	- firstActor: the first actor for the constraint. This may be unset if using the existing first
	  actor of the constraint, or an object with the following elements depending on how the actor
	  is referenced:
	  For referencing an instance as part of a rigid body group node:
	  - rootNode: the name of a distinct root node. This may be unset to use the rigid body group
	    node.
	  - rigidBodyGroupNode: the name of the rigid body group node.
	  - instance: the name of the instance within the rigid body group node.
	  For referencing an actor (such as a rigid body) resource:
	  - actor: the name of the actor.
	- firstConnectedConstraint: the first connected constraint. This may be unset if using the
	  existing first connected constraint or if the first connected constraint is unused. Otherwise
	  it is an object with the following elements depending on how the constraint is referenced:
	  For referencing an instance as part of a rigid body group node:
	  - rootNode: the name of a distinct root node. This may be unset to use the rigid body group
	    node.
	  - rigidBodyGroupNode: the name of the rigid body group node.
	  - instance: the name of the instance within the rigid body group node.
	  For referencing a constraint resource:
	  - constraint: the name of the constraint.
	  For referencing another constraint node:
	  - constraintNode: the name of the constraint node.
	- secondActor: the second actor for the constraint. This may be unset if using the existing
	  second actor of the constraint, or an object with the following elements depending on how the
	  actor is referenced:
	  For referencing an instance as part of a rigid body group node:
	  - rootNode: the name of a distinct root node. This may be unset to use the rigid body group
	    node.
	  - rigidBodyGroupNode: the name of the rigid body group node.
	  - instance: the name of the instance within the rigid body group node.
	  For referencing an actor (such as a rigid body) resource:
	  - actor: the name of the actor.
	- secondConnectedConstraint: the second connected constraint. This may be unset if using the
	  existing second connected constraint or if the second connected constraint is unused.
	  Otherwise it is an object with the following elements depending on how the constraint is
	  referenced:
	  For referencing an instance as part of a rigid body group node:
	  - rootNode: the name of a distinct root node. This may be unset to use the rigid body group
	    node.
	  - rigidBodyGroupNode: the name of the rigid body group node.
	  - instance: the name of the instance within the rigid body group node.
	  For referencing a constraint resource:
	  - constraint: the name of the constraint.
	  For referencing another constraint node:
	  - constraintNode: the name of the constraint node.
	- itemLists: array of item list names to add the node to.
	"""
	try:
		constraint = str(data['constraint'])
		firstActorData = data.get('firstActor')
		firstConnectedConstraintData = data.get('firstConnectedConstraint')
		secondActorData = data.get('secondActor')
		secondConnectedConstraintData = data.get('secondConnectedConstraint')
		itemLists = data.get('itemLists')
	except (TypeError, ValueError):
		raise Exception('PhysicsConstraintNode data must be an object.')
	except KeyError as e:
		raise Exception('PhysicsConstraintNode data doesn\'t contain element ' + str(e) + '.')

	builder = flatbuffers.Builder(0)

	constraintOffset = builder.CreateString(constraint)

	if firstActorData:
		try:
			actorStr = str(firstActorData.get('actor', ''))
			if actorStr:
				actorOffset = builder.CreateString(actorStr)

				ActorResourceReference.Start(builder)
				ActorResourceReference.AddActor(builder, actorOffset)
				firstActorOffset = ActorResourceReference.End(builder)
				firstActorType = ActorReference.ActorResourceReference
			else:
				rootNodeStr = str(firstActorData.get('rootNode', ''))
				rigidBodyGroupNodeStr = str(firstActorData['rigidBodyGroupNode'])
				instanceStr = str(firstActorData['instance'])

				rootNodeOffset = builder.CreateString(rootNodeStr) if rootNodeStr else 0
				rigidBodyGroupNodeOffset = builder.CreateString(rigidBodyGroupNodeStr)
				instanceOffset = builder.CreateString(instanceStr)

				InstanceReference.Start(builder)
				InstanceReference.AddRootNode(builder, rootNodeOffset)
				InstanceReference.AddRigidBodyGroupNode(builder, rigidBodyGroupNodeOffset)
				InstanceReference.AddInstance(builder, instanceOffset)
				firstActorOffset = InstanceReference.End(builder)
				firstActorType = ActorReference.InstanceReference
		except (TypeError, ValueError):
			raise Exception('PhysicsConstraintNode "firstActor" must be an object.')
		except KeyError as e:
			raise Exception(
				'PhysicsConstraintNode "firstActor" doesn\'t contain element ' + str(e) + '.')
	else:
		firstActorOffset = 0
		firstActorType = ActorReference.NONE

	if firstConnectedConstraintData:
		try:
			constraintStr = str(firstConnectedConstraintData.get('constraint', ''))
			constraintNodeStr = str(firstConnectedConstraintData.get('constraintNode', ''))
			if constraintStr:
				constraintOffset = builder.CreateString(constraintStr)

				ConstraintResourceReference.Start(builder)
				ConstraintResourceReference.AddConstraint(builder, constraintOffset)
				firstConnectedConstraintOffset = ConstraintResourceReference.End(builder)
				firstConnectedConstraintType = ConstraintReference.ConstraintResourceReference
			elif constraintNodeStr:
				constraintNodeOffset = builder.CreateString(constraintNodeStr)

				ConstraintNodeReference.Start(builder)
				ConstraintNodeReference.AddConstraintNode(builder, constraintNodeOffset)
				firstConnectedConstraintOffset = ConstraintNodeReference.End(builder)
				firstConnectedConstraintType = ConstraintReference.ConstraintResourceReference
			else:
				rootNodeStr = str(firstConnectedConstraintData.get('rootNode', ''))
				rigidBodyGroupNodeStr = str(firstConnectedConstraintData['rigidBodyGroupNode'])
				instanceStr = str(firstConnectedConstraintData['instance'])

				rootNodeOffset = builder.CreateString(rootNodeStr) if rootNodeStr else 0
				rigidBodyGroupNodeOffset = builder.CreateString(rigidBodyGroupNodeStr)
				instanceOffset = builder.CreateString(instanceStr)

				InstanceReference.Start(builder)
				InstanceReference.AddRootNode(builder, rootNodeOffset)
				InstanceReference.AddRigidBodyGroupNode(builder, rigidBodyGroupNodeOffset)
				InstanceReference.AddInstance(builder, instanceOffset)
				firstConnectedConstraintOffset = InstanceReference.End(builder)
				firstConnectedConstraintType = ConstraintReference.InstanceReference
		except (TypeError, ValueError):
			raise Exception('PhysicsConstraintNode "firstConnectedConstraint" must be an object.')
		except KeyError as e:
			raise Exception(
				'PhysicsConstraintNode "firstConnectedConstraint" doesn\'t contain element ' +
				str(e) + '.')
	else:
		firstConnectedConstraintOffset = 0
		firstConnectedConstraintType = ConstraintReference.NONE

	if secondActorData:
		try:
			actorStr = str(secondActorData.get('actor', ''))
			if actorStr:
				actorOffset = builder.CreateString(actorStr)

				ActorResourceReference.Start(builder)
				ActorResourceReference.AddActor(builder, actorOffset)
				secondActorOffset = ActorResourceReference.End(builder)
				secondActorType = ActorReference.ActorResourceReference
			else:
				rootNodeStr = str(secondActorData.get('rootNode', ''))
				rigidBodyGroupNodeStr = str(secondActorData['rigidBodyGroupNode'])
				instanceStr = str(secondActorData['instance'])

				rootNodeOffset = builder.CreateString(rootNodeStr) if rootNodeStr else 0
				rigidBodyGroupNodeOffset = builder.CreateString(rigidBodyGroupNodeStr)
				instanceOffset = builder.CreateString(instanceStr)

				InstanceReference.Start(builder)
				InstanceReference.AddRootNode(builder, rootNodeOffset)
				InstanceReference.AddRigidBodyGroupNode(builder, rigidBodyGroupNodeOffset)
				InstanceReference.AddInstance(builder, instanceOffset)
				secondActorOffset = InstanceReference.End(builder)
				secondActorType = ActorReference.InstanceReference
		except (TypeError, ValueError):
			raise Exception('PhysicsConstraintNode "secondActor" must be an object.')
		except KeyError as e:
			raise Exception('PhysicsConstraintNode "secondActor" doesn\'t contain element ' +
				str(e) + '.')
	else:
		secondActorOffset = 0
		secondActorType = ActorReference.NONE

	if secondConnectedConstraintData:
		try:
			constraintStr = str(secondConnectedConstraintData.get('constraint', ''))
			constraintNodeStr = str(secondConnectedConstraintData.get('constraintNode', ''))
			if constraintStr:
				constraintOffset = builder.CreateString(constraintStr)

				ConstraintResourceReference.Start(builder)
				ConstraintResourceReference.AddConstraint(builder, constraintOffset)
				secondConnectedConstraintOffset = ConstraintResourceReference.End(builder)
				secondConnectedConstraintType = ConstraintReference.ConstraintResourceReference
			elif constraintNodeStr:
				constraintNodeOffset = builder.CreateString(constraintNodeStr)

				ConstraintNodeReference.Start(builder)
				ConstraintNodeReference.AddConstraintNode(builder, constraintNodeOffset)
				secondConnectedConstraintOffset = ConstraintNodeReference.End(builder)
				secondConnectedConstraintType = ConstraintReference.ConstraintResourceReference
			else:
				rootNodeStr = str(secondConnectedConstraintData.get('rootNode', ''))
				rigidBodyGroupNodeStr = str(secondConnectedConstraintData['rigidBodyGroupNode'])
				instanceStr = str(secondConnectedConstraintData['instance'])

				rootNodeOffset = builder.CreateString(rootNodeStr) if rootNodeStr else 0
				rigidBodyGroupNodeOffset = builder.CreateString(rigidBodyGroupNodeStr)
				instanceOffset = builder.CreateString(instanceStr)

				InstanceReference.Start(builder)
				InstanceReference.AddRootNode(builder, rootNodeOffset)
				InstanceReference.AddRigidBodyGroupNode(builder, rigidBodyGroupNodeOffset)
				InstanceReference.AddInstance(builder, instanceOffset)
				secondConnectedConstraintOffset = InstanceReference.End(builder)
				secondConnectedConstraintType = ConstraintReference.InstanceReference
		except (TypeError, ValueError):
			raise Exception('PhysicsConstraintNode "secondConnectedConstraint" must be an object.')
		except KeyError as e:
			raise Exception(
				'PhysicsConstraintNode "secondConnectedConstraint" doesn\'t contain element ' +
				str(e) + '.')
	else:
		secondConnectedConstraintOffset = 0
		secondConnectedConstraintType = ConstraintReference.NONE

	if itemLists:
		itemListOffsets = []
		try:
			for item in itemLists:
				itemListOffsets.append(builder.CreateString(str(item)))
		except (TypeError, ValueError):
			raise Exception('PhysicsConstraintNode "itemLists" must be an array of strings.')

		ConstraintNode.StartItemListsVector(builder, len(itemListOffsets))
		for offset in reversed(itemListOffsets):
			builder.PrependUOffsetTRelative(offset)
		itemListsOffset = builder.EndVector()
	else:
		itemListsOffset = 0

	ConstraintNode.Start(builder)
	ConstraintNode.AddConstraint(builder, constraintOffset)
	ConstraintNode.AddFirstActorType(builder, firstActorType)
	ConstraintNode.AddFirstActor(builder, firstActorOffset)
	ConstraintNode.AddFirstConnectedConstraintType(builder, firstConnectedConstraintType)
	ConstraintNode.AddFirstConnectedConstraint(builder, firstConnectedConstraintOffset)
	ConstraintNode.AddSecondActorType(builder, secondActorType)
	ConstraintNode.AddSecondActor(builder, secondActorOffset)
	ConstraintNode.AddSecondConnectedConstraintType(builder, secondConnectedConstraintType)
	ConstraintNode.AddSecondConnectedConstraint(builder, secondConnectedConstraintOffset)
	ConstraintNode.AddItemLists(builder, itemListsOffset)
	builder.Finish(ConstraintNode.End(builder))
	return builder.Output()
