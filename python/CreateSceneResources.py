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
import io
import json
import os
import sys
from importlib import import_module

from DeepSeaScene.Convert.ConvertContext import ConvertContext
from DeepSeaScene.Convert.SceneResourcesConvert import convertSceneResources

from DeepSeaSceneAnimation.Convert.AnimationJointTreeConvert import convertAnimationJointTree
from DeepSeaSceneAnimation.Convert.AnimationNodeConvert import convertAnimationNode
from DeepSeaSceneAnimation.Convert.AnimationTreeConvert import convertAnimationTree
from DeepSeaSceneAnimation.Convert.AnimationTransformNodeConvert import \
	convertAnimationTransformNode
from DeepSeaSceneAnimation.Convert.AnimationTreeNodeConvert import convertAnimationTreeNode
from DeepSeaSceneAnimation.Convert.DirectAnimationConvert import convertDirectAnimation
from DeepSeaSceneAnimation.Convert.GLTFAnimationJointTree import registerGLTFAnimationJointTreeType
from DeepSeaSceneAnimation.Convert.GLTFAnimationTree import registerGLTFAnimationTreeType
from DeepSeaSceneAnimation.Convert.GLTFKeyframeAnimation import registerGLTFKeyframeAnimationType
from DeepSeaSceneAnimation.Convert.KeyframeAnimationConvert import convertKeyframeAnimation
from DeepSeaSceneAnimation.Convert.NodeMapCacheConvert import convertNodeMapCache

from DeepSeaSceneLighting.Convert.LightNodeConvert import convertLightNode
from DeepSeaSceneLighting.Convert.LightSetConvert import convertLightSet
from DeepSeaSceneLighting.Convert.ShadowManagerConvert import convertShadowManager

from DeepSeaSceneParticle.Convert.ParticleNodeConvert import convertParticleNode
from DeepSeaSceneParticle.Convert.StandardParticleEmitterFactoryConvert import \
	convertStandardParticleEmitterFactory

from DeepSeaScenePhysics.Convert.DistancePhysicsConstraintConvert import \
	convertDistancePhysicsConstraint
from DeepSeaScenePhysics.Convert.FixedPhysicsConstraintConvert import convertFixedPhysicsConstraint
from DeepSeaScenePhysics.Convert.GearPhysicsConstraintConvert import convertGearPhysicsConstraint
from DeepSeaScenePhysics.Convert.GenericPhysicsConstraintConvert import \
	convertGenericPhysicsConstraint
from DeepSeaScenePhysics.Convert.PhysicsBoxConvert import convertPhysicsBox
from DeepSeaScenePhysics.Convert.PhysicsCapsuleConvert import convertPhysicsCapsule
from DeepSeaScenePhysics.Convert.PhysicsConeConvert import convertPhysicsCone
from DeepSeaScenePhysics.Convert.PhysicsConstraintNodeConvert import convertPhysicsConstraintNode
from DeepSeaScenePhysics.Convert.PhysicsConvexHullConvert import convertPhysicsConvexHull
from DeepSeaScenePhysics.Convert.PhysicsCylinderConvert import convertPhysicsCylinder
from DeepSeaScenePhysics.Convert.PhysicsMeshConvert import convertPhysicsMesh
from DeepSeaScenePhysics.Convert.PhysicsSphereConvert import convertPhysicsSphere
from DeepSeaScenePhysics.Convert.PhysicsShapeRefConvert import convertPhysicsShapeRef
from DeepSeaScenePhysics.Convert.PointPhysicsConstraintConvert import convertPointPhysicsConstraint
from DeepSeaScenePhysics.Convert.RigidBodyConvert import convertRigidBody
from DeepSeaScenePhysics.Convert.RigidBodyGroupNodeConvert import convertRigidBodyGroupNode
from DeepSeaScenePhysics.Convert.RigidBodyNodeConvert import convertRigidBodyNode
from DeepSeaScenePhysics.Convert.RigidBodyTemplateConvert import convertRigidBodyTemplate
from DeepSeaScenePhysics.Convert.RackAndPinionPhysicsConstraintConvert import \
	convertRackAndPinionPhysicsConstraint
from DeepSeaScenePhysics.Convert.RevolutePhysicsConstraintConvert import \
	convertRevolutePhysicsConstraint
from DeepSeaScenePhysics.Convert.SliderPhysicsConstraintConvert import \
	convertSliderPhysicsConstraint
from DeepSeaScenePhysics.Convert.SwingTwistPhysicsConstraintConvert import \
	convertSwingTwistPhysicsConstraint

from DeepSeaSceneVectorDraw.Convert.TextConvert import convertText
from DeepSeaSceneVectorDraw.Convert.TextNodeConvert import convertTextNode
from DeepSeaSceneVectorDraw.Convert.VectorImageConvert import convertVectorImage
from DeepSeaSceneVectorDraw.Convert.VectorImageNodeConvert import convertVectorImageNode
from DeepSeaSceneVectorDraw.Convert.VectorResourcesConvert import convertVectorResources
from DeepSeaSceneVectorDraw.Convert.VectorShadersConvert import convertVectorShaders

def createSceneResourcesConvertContext(cuttlefish='cuttlefish', vfc='vfc', multithread=True,
		customExtensions=None):
	"""
	Creates a ConvertContext for scene resources with the default set of extensions.

	:param cuttlefish: Path to the cuttlefish executable.
	:param vfc: Path to the vfc executable.
	:param multithread: Whether to multithread texture conversion.
	:param customExtensions: List of custom extensions to add, which will be loaded as modules.
	"""
	convertContext = ConvertContext(cuttlefish, vfc, multithread)

	# Animation scene types.
	convertContext.addCustomResourceType('AnimationJointTree', convertAnimationJointTree,
		'AnimationTree')
	convertContext.addCustomResourceType('AnimationTree', convertAnimationTree)
	convertContext.addCustomResourceType('DirectAnimation', convertDirectAnimation)
	convertContext.addCustomResourceType('KeyframeAnimation', convertKeyframeAnimation)
	convertContext.addCustomResourceType('AnimationNodeMapCache', convertNodeMapCache)
	convertContext.addNodeType('AnimationNode', convertAnimationNode)
	convertContext.addNodeType('AnimationTransformNode', convertAnimationTransformNode)
	convertContext.addNodeType('AnimationTreeNode', convertAnimationTreeNode)
	registerGLTFAnimationJointTreeType(convertContext)
	registerGLTFAnimationTreeType(convertContext)
	registerGLTFKeyframeAnimationType(convertContext)

	# Lighting scene types.
	convertContext.addCustomResourceType('LightSet', convertLightSet)
	convertContext.addCustomResourceType('ShadowManager', convertShadowManager)
	convertContext.addNodeType('LightNode', convertLightNode)

	# Particle scene types.
	convertContext.addCustomResourceType('StandardParticleEmitterFactory',
		convertStandardParticleEmitterFactory)
	convertContext.addNodeType('ParticleNode', convertParticleNode)

	# Physics scene types.
	#Shapes
	convertContext.addCustomResourceType("PhysicsBox", convertPhysicsBox, "PhysicsShape")
	convertContext.addCustomResourceType("PhysicsCapsule", convertPhysicsCapsule, "PhysicsShape")
	convertContext.addCustomResourceType("PhysicsCone", convertPhysicsCone, "PhysicsShape")
	convertContext.addCustomResourceType("PhysicsConvexHull", convertPhysicsConvexHull,
		"PhysicsShape")
	convertContext.addCustomResourceType("PhysicsCylinder", convertPhysicsCylinder, "PhysicsShape")
	convertContext.addCustomResourceType("PhysicsMesh", convertPhysicsMesh, "PhysicsShape")
	convertContext.addCustomResourceType("PhysicsSphere", convertPhysicsSphere, "PhysicsShape")
	convertContext.addCustomResourceType("PhysicsShapeRef", convertPhysicsShapeRef, "PhysicsShape")
	# Constraints
	convertContext.addCustomResourceType("FixedPhysicsConstraint", convertFixedPhysicsConstraint,
		"PhysicsConstraint")
	convertContext.addCustomResourceType("PointPhysicsConstraint", convertPointPhysicsConstraint,
		"PhysicsConstraint")
	convertContext.addCustomResourceType("SwingTwistPhysicsConstraint",
		convertSwingTwistPhysicsConstraint, "PhysicsConstraint")
	convertContext.addCustomResourceType("RevolutePhysicsConstraint",
		convertRevolutePhysicsConstraint, "PhysicsConstraint")
	convertContext.addCustomResourceType("DistancePhysicsConstraint",
		convertDistancePhysicsConstraint, "PhysicsConstraint")
	convertContext.addCustomResourceType("SliderPhysicsConstraint", convertSliderPhysicsConstraint,
		"PhysicsConstraint")
	convertContext.addCustomResourceType("GenericPhysicsConstraint",
		convertGenericPhysicsConstraint, "PhysicsConstraint")
	convertContext.addCustomResourceType("GearPhysicsConstraint", convertGearPhysicsConstraint,
		"PhysicsConstraint")
	convertContext.addCustomResourceType("RackAndPinionPhysicsConstraint",
		convertRackAndPinionPhysicsConstraint, "PhysicsConstraint")
	# Other
	convertContext.addCustomResourceType("RigidBody", convertRigidBody, "RigidBody")
	convertContext.addCustomResourceType("RigidBodyTemplate",
		convertRigidBodyTemplate, "RigidBodyTemplate")
	convertContext.addNodeType('PhysicsConstraintNode', convertPhysicsConstraintNode)
	convertContext.addNodeType('RigidBodyGroupNode', convertRigidBodyGroupNode)
	convertContext.addNodeType('RigidBodyNode', convertRigidBodyNode)
	convertContext.addNodeType('RigidBodyTemplateNode', convertRigidBodyNode)
	convertContext.addNodeType('UniqueRigidBodyNode', convertRigidBodyNode)

	# Vector draw scene types.
	convertContext.addCustomResourceType('Text', convertText)
	convertContext.addCustomResourceType('VectorImage', convertVectorImage)
	convertContext.addCustomResourceType('VectorResources', convertVectorResources)
	convertContext.addCustomResourceType('VectorShaders', convertVectorShaders)
	convertContext.addNodeType('TextNode', convertTextNode)
	convertContext.addNodeType('VectorImageNode', convertVectorImageNode)

	if customExtensions:
		for extension in customExtensions:
			import_module(extension).deepSeaSceneExtension(convertContext)

	return convertContext

if __name__ == '__main__':
	parser = argparse.ArgumentParser(description =
		'Create scene resources to be used by Deep Sea.')
	parser.add_argument('-i', '--input', required = True,
		help = 'input json description of the resources')
	parser.add_argument('-d', '--input-directory', help = 'explicit directory to use for relatve '
		"paths; defaults to the input file's directory")
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
	convertContext = createSceneResourcesConvertContext(args.cuttlefish, args.vfc, args.multithread,
		args.extensions)

	if args.input_directory:
		inputDir = args.input_directory
	else:
		inputDir = os.path.dirname(args.input)

	try:
		with io.open(args.input, encoding='utf-8') as f:
			data = json.load(f)

		with open(args.output, 'wb') as f:
			f.write(convertSceneResources(
				convertContext, data, inputDir, os.path.dirname(args.output)))
	except Exception as e:
		print(args.input + ': error: ' + str(e), file=sys.stderr)
		exit(1)
