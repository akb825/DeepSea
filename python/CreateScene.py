#!/usr/bin/env python
# Copyright 2020-2023 Aaron Barany
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
import sys
from importlib import import_module

from DeepSeaScene.Convert.ConvertContext import ConvertContext
from DeepSeaScene.Convert.SceneConvert import convertScene

from DeepSeaSceneAnimation.Convert.AnimationListConvert import convertAnimationList

from DeepSeaSceneLighting.Convert.InstanceForwardLightDataConvert \
	import convertInstanceForwardLightData
from DeepSeaSceneLighting.Convert.DeferredLightResolveConvert import convertDeferredLightResolve
from DeepSeaSceneLighting.Convert.LightSetPrepareConvert import convertLightSetPrepare
from DeepSeaSceneLighting.Convert.ShadowCullListConvert import convertShadowCullList
from DeepSeaSceneLighting.Convert.ShadowInstanceTransformDataConvert \
	import convertShadowInstanceTransformData
from DeepSeaSceneLighting.Convert.ShadowManagerPrepareConvert import convertShadowManagerPrepare
from DeepSeaSceneLighting.Convert.SSAOConvert import convertSSAO

from DeepSeaSceneParticle.Convert.ParticleDrawListConvert import convertParticleDrawList
from DeepSeaSceneParticle.Convert.ParticlePrepareConvert import convertParticlePrepare
from DeepSeaSceneParticle.Convert.ParticleTransformDataConvert import convertParticleTransformData

from DeepSeaSceneVectorDraw.Convert.VectorDrawPrepareConvert import convertVectorDrawPrepare
from DeepSeaSceneVectorDraw.Convert.VectorItemListConvert import convertVectorItemList

if __name__ == '__main__':
	parser = argparse.ArgumentParser(description =
		'Create a scene to be used by Deep Sea.')
	parser.add_argument('-i', '--input', required = True,
		help = 'input json description of the scene')
	parser.add_argument('-o', '--output', required = True,
		help = 'output file name, typically with the extension ".dss"')
	parser.add_argument('-e', '--extensions', nargs = '*', default = [],
		help = 'list of module names for extensions. Eeach extension should have a '
			'deepSeaSceneExtension(convertContext) function to register the custom types with the'
			'convert context.')

	args = parser.parse_args()
	convertContext = ConvertContext()

	# Animation scene types.
	convertContext.addItemListType('AnimationList', convertAnimationList)

	# Lighting scene types.
	convertContext.addItemListType('ComputeSSAO', convertSSAO) # Same type as normal SSAO.
	convertContext.addItemListType('DeferredLightResolve', convertDeferredLightResolve)
	convertContext.addItemListType('LightSetPrepare', convertLightSetPrepare)
	convertContext.addItemListType('ShadowCullList', convertShadowCullList)
	convertContext.addItemListType('ShadowManagerPrepare', convertShadowManagerPrepare)
	convertContext.addItemListType('SSAO', convertSSAO)
	convertContext.addInstanceDataType('InstanceForwardLightData', convertInstanceForwardLightData)
	convertContext.addInstanceDataType('ShadowInstanceTransformData',
		convertShadowInstanceTransformData)

	# Particle scene types.
	convertContext.addItemListType('ParticleDrawList', convertParticleDrawList)
	convertContext.addItemListType('ParticlePrepare', convertParticlePrepare)
	convertContext.addInstanceDataType('ParticleTransformData', convertParticleTransformData)

	# Vector draw scene types.
	convertContext.addItemListType('VectorDrawPrepare', convertVectorDrawPrepare)
	convertContext.addItemListType('VectorItemList', convertVectorItemList)

	for extension in args.extensions:
		import_module(extension).deepSeaSceneExtension(convertContext)

	try:
		with open(args.input) as f:
			data = json.load(f)

		with open(args.output, 'wb') as f:
			f.write(convertScene(convertContext, data))
	except Exception as e:
		print(args.input + ': error: ' + str(e), file=sys.stderr)
		exit(1)
