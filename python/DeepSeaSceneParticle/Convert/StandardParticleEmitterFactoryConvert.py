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

from DeepSeaScene.Convert.TransformConvert import convertTransform
from DeepSeaScene.Matrix44f import CreateMatrix44f
from DeepSeaScene.Vector2f import CreateVector2f
from DeepSeaScene.Vector3f import CreateVector3f

from .. import ParticleBox
from .. import ParticleCylinder
from .. import ParticleEmitterParams
from .. import ParticleSphere
from .. import StandardParticleEmitterFactory
from ..ParticleVolume import ParticleVolume
from ..Vector2u import CreateVector2u

def convertStandardParticleEmitterFactory(convertContext, data):
	"""
	Converts a StandardParticleEmitterFactory. The data map is expected to contain the following
	elements:
	- maxParticles: the maximum number of particles displayed at once.
	- shader: the name of the shader to draw with.
	- material: the name of the material to draw with.
	- instanceValueCount: optional number of material instance values. The max between this and the
	  instance value count of material will be used, which can be used to allow swapping out for
	  different materials at runtime. Defaults to 0.
	- spawnVolume: the volume to spawn particles in. This is expected to contain the following
	  members:
	  - type: they type of the volume. May be Box, Sphere, or Cylinder. The following members
	    depend on the type.
	  Box:
	  - bounds: 2x3 array of float values for the minimum maximum values of the box.
	  Sphere:
	  - center: array of 3 floats for the center of the sphere.
	  - radius: float for the radius of the sphere.
	  Cylinder:
	  - center: array of 3 floats for the center of the cylinder.
	  - radius: float for the radius of the cylinder.
	  - height: float for the height of the cylinder along the Z axis.
	- spawnVolumeTransformList: optional array of transforms to apply to the volume. If unset the
	  transform will be the identiy matrix. When provided, the matrices for the transforms provided
	  are multiplied in reverse order given (i.e. in logical transform order), starting from the
	  identity matrix. Each member of the array has the following elements:
	  - type: the type of transform. May be Rotate, Scale, Translate, or Matrix.
	  - value: the value of transform based on the type:
	    - Rotate: array of 3 floats for the X, Y, and Z rotation in degrees.
	    - Scale: array of 3 floats for the scale value along the X, Y, and Z axes.
	    - Translate: array of 3 floats for the translation along X, Y, and Z.
	    - Matrix: a 4x4 array of floats for a matrix. Each inner array is a column of the matrix.
	- widthRange: array of 2 floats for the minimum and maximum width values.
	- heightRange: optional array of 2 floats for the minimum and maximum height values. If unsset
	  the width values will be used to keep the particles square.
	- baseDirection: array of 3 floats for the base direction the particles will move along.
	- directionSpread: spread along the base direction for particles to move along as an angle in
	  degrees.
	- spawnTimeRange: array of 2 floats for the minimum and maximum time in seconds between
	  spawning of particles.
	- activeTimeRange: array of 2 floats for the minimum and maximum time in seconds a particle is
	  active for.
	- speedRange: array of 2 floats for the minimum and maximum speed a particle travels at.
	- rotationSpeedRange: array of 2 floats for the minimum and maximum rotation speed of a particle
	  in degrees per second.
	- textureRange: optional array of 2 integers for the minimum and maximum texture index ranges.
	  Defaults to [0, 0] if unset.
	- colorHueRange: array of 2 floats for the minimum and maximum color hue n the range [0, 360].
	  The minimum may be larger than the maximum to wrap around at 360.
	- colorSaturationRange: array of 2 floats for the minimum and maximum color saturation in the
	  range [0, 1].
	- colorValueRange: array of 2 floats for the minimum and maximum color value in the range
	  [0, 1].
	- colorAlphaRange: array of 2 floats for the minimum and maximum color alpha value in the range
	  [0, 1]. Defaults to [1, 1] if unset.
	- intensityRange: array of 2 floats for the minimum and maximum intensity of a particle.
	  Defaults to [1, 1] if unset.
	- relativeNode: optional name of a node to transform the particles relative to. When set, the
	  particles will use the transform of relativeNode, while the volume boundary to spawn the
	  particles will be relative to the particle emitter's node. This must be an ancestor of the
	  node the particle emitter will be created with.
	- seed: optional random seed to create the factory with. If unset or 0 a random seed will be
	  generated.
	- enabled: optional bool for whether or not the particle emitter will be enabled on creation.
	  Defaults to true.
	- startTime: optional float for the time in seconds to advance the particle emitter on creation.
	  Defaults to 0.
	"""
	def invalidValueStr(value, name):
		return 'Invalid standard particle emitter ' + name + ' "' + str(value) + '".'

	def readInt(value, name, minValue = 0):
		try:
			intVal = int(value)
			if intVal < minValue:
				raise Exception() # Common error handling in except block.
			return intVal
		except:
			raise Exception(invalidValueStr(value, name + ' int value'))

	def readIntMinMax(value, name, minValue = 0):
		if not isinstance(value, list) or len(value) != 2:
			raise Exception(invalidValueStr(value, name))

		uintValues = [readInt(value[0], name, minValue), readInt(value[1], name, minValue)]
		if uintValues[0] > uintValues[1]:
			raise Exception(invalidValueStr(value, name))
		return uintValues

	def readFloat(value, name, minVal = None, maxVal = None):
		try:
			f = float(value)
			if (minVal is not None and f < minVal) or (maxVal is not None and f > maxVal):
				raise Exception()
			return f
		except:
			raise Exception(invalidValueStr(value, name + ' float value'))
	
	def readFloatMinMax(value, name, minVal = None, maxVal = None, validateOrder = True):
		if not isinstance(value, list) or len(value) != 2:
			raise Exception(invalidValueStr(value, name))

		floatValues = [readFloat(value[0], name, minVal, maxVal),
			readFloat(value[1], name, minVal, maxVal)]
		if validateOrder and floatValues[0] > floatValues[1]:
			raise Exception(invalidValueStr(value, name))
		return floatValues

	def readVector3f(value, name):
		if not isinstance(value, list) or len(value) != 3:
			raise Exception(invalidValueStr(value, name))

		return [readFloat(value[0], name), readFloat(value[1], name), readFloat(value[2], name)]

	def readBool(value, name):
		try:
			return bool(value)
		except:
			raise Exception(invalidValueStr(value, name))

	try:
		maxParticles = readInt(data['maxParticles'], 'maxParticles', 1)
		shader = str(data['shader'])
		material = str(data['material'])
		instanceValueCount = readInt(data.get('instanceValueCount', 0), 'instanceValueCount')

		spawnVolumeData = data['spawnVolume']
		try:
			volumeTypeStr = str(spawnVolumeData['type'])
			if volumeTypeStr == 'Box':
				volumeType = ParticleVolume.ParticleBox
				bounds = spawnVolumeData['bounds']
				if not isinstance(bounds, list) or len(bounds) != 2:
					raise Exception(invalidValueStr(bounds, 'bounds'))
				volumeBoxMin = readVector3f(bounds[0], 'bounds min')
				volumeBoxMax = readVector3f(bounds[1], 'bounds min')
			elif volumeTypeStr == 'Sphere':
				volumeType = ParticleVolume.ParticleSphere
				volumeCenter = readVector3f(spawnVolumeData['center'], 'center')
				volumeRadius = readFloat(spawnVolumeData['radius'], 'radius', 0.0)
			elif volumeTypeStr == 'Cylinder':
				volumeType = ParticleVolume.ParticleCylinder
				volumeCenter = readVector3f(spawnVolumeData['center'], 'center')
				volumeRadius = readFloat(spawnVolumeData['radius'], 'radius', 0.0)
				volumeHeight = readFloat(spawnVolumeData['height'], 'height', 0.0)
			else:
				raise Exception('Invalid volume type "' + volumeType + '".')
		except KeyError as e:
			raise Exception(
				'StandardParticleEmitterFactory spawnVolumeData doesn\'t contain element ' +
				str(e) + '.')
		except (TypeError, ValueError):
			raise Exception('StandardParticleEmitterFactory spawnVolumeData must be an object.')

		volumeMatrix = convertTransform(data.get('spawnVolumeTransformList', []),
			'StandardParticleEmitterFactory', 'spawnVolumeTransformList')
		widthRange = readFloatMinMax(data['widthRange'], 'widthRange', 0.0)

		heightRangeData = data.get('heightRange')
		if heightRangeData:
			heightRange = readFloatMinMax(heightRangeData, 'heightRange', 0.0)
		else:
			heightRange = None

		baseDirection = readVector3f(data['baseDirection'], 'baseDirection')
		directionSpread = readFloat(data['directionSpread'], 'directionSpread')
		spawnTimeRange = readFloatMinMax(data['spawnTimeRange'], 'spawnTimeRange', 0.0)
		activeTimeRange = readFloatMinMax(data['activeTimeRange'], 'activeTimeRange', 0.0)
		speedRange = readFloatMinMax(data['speedRange'], 'speedRange', 0.0)
		rotationSpeedRange = readFloatMinMax(data['rotationSpeedRange'], 'rotationSpeedRange')
		textureRange = readIntMinMax(data.get('textureRange', [0, 0]), 'textureRange', 0)
		colorHueRange = readFloatMinMax(data['colorHueRange'], 'colorHueRange', 0.0, 360.0, False)
		colorSaturationRange = readFloatMinMax(data['colorSaturationRange'], 'colorSaturationRange',
			0.0, 1.0)
		colorValueRange = readFloatMinMax(data['colorValueRange'], 'colorValueRange', 0.0, 1.0)
		colorAlphaRangeData = data.get('colorAlphaRange')
		if colorAlphaRangeData:
			colorAlphaRange = readFloatMinMax(colorAlphaRangeData, 'colorAlphaRange', 0.0, 1.0)
		else:
			colorAlphaRange = [1.0, 1.0]
		intensityRangeData = data.get('intensityRange')
		if intensityRangeData:
			intensityRange = readFloatMinMax(intensityRangeData, 'intensityRange', 0.0)
		else:
			intensityRange = [1.0, 1.0]
		relativeNode = str(data.get('relativeNode', ''))
		seed = readInt(data.get('seed', 0), 'seed')
		enabled = readBool(data.get('enabled', True), 'enabled')
		startTime = readFloat(data.get('startTime', 0.0), 'startTime', 0.0)
	except KeyError as e:
		raise Exception(
			'StandardParticleEmitterFactory data doesn\'t contain element ' + str(e) + '.')
	except (TypeError, ValueError):
		raise Exception('StandardParticleEmitterFactory data must be an object.')

	builder = flatbuffers.Builder(0)

	shaderOffset = builder.CreateString(shader)
	materialOffset = builder.CreateString(material)

	ParticleEmitterParams.Start(builder)
	ParticleEmitterParams.AddMaxParticles(builder, maxParticles)
	ParticleEmitterParams.AddShader(builder, shaderOffset)
	ParticleEmitterParams.AddMaterial(builder, materialOffset)
	ParticleEmitterParams.AddInstanceValueCount(builder, instanceValueCount)
	particleEmitterParamsOffset = ParticleEmitterParams.End(builder)

	if volumeType == ParticleVolume.ParticleBox:
		ParticleBox.Start(builder)
		ParticleBox.AddMin(builder, CreateVector3f(builder, *volumeBoxMin))
		ParticleBox.AddMax(builder, CreateVector3f(builder, *volumeBoxMax))
		volumeOffset = ParticleBox.End(builder)
	elif volumeType == ParticleVolume.ParticleSphere:
		ParticleSphere.Start(builder)
		ParticleSphere.AddCenter(builder, CreateVector3f(builder, *volumeCenter))
		ParticleSphere.AddRadius(builder, volumeRadius)
		volumeOffset = ParticleSphere.End(builder)
	elif volumeType == ParticleVolume.ParticleCylinder:
		ParticleCylinder.Start(builder)
		ParticleCylinder.AddCenter(builder, CreateVector3f(builder, *volumeCenter))
		ParticleCylinder.AddRadius(builder, volumeRadius)
		ParticleCylinder.AddHeight(builder, volumeHeight)
		volumeOffset = ParticleCylinder.End(builder)
	else:
		assert False

	if relativeNode:
		relativeNodeOffset = builder.CreateString(relativeNode)
	else:
		relativeNodeOffset = 0

	StandardParticleEmitterFactory.Start(builder)
	StandardParticleEmitterFactory.AddParams(builder, particleEmitterParamsOffset)
	StandardParticleEmitterFactory.AddSpawnVolumeType(builder, volumeType)
	StandardParticleEmitterFactory.AddSpawnVolume(builder, volumeOffset)
	StandardParticleEmitterFactory.AddSpawnVolumeMatrix(builder,
		CreateMatrix44f(builder, *volumeMatrix))
	StandardParticleEmitterFactory.AddWidthRange(builder, CreateVector2f(builder, *widthRange))
	StandardParticleEmitterFactory.AddHeightRange(builder,
		CreateVector2f(builder, *heightRange) if heightRange else 0)
	StandardParticleEmitterFactory.AddBaseDirection(builder,
		CreateVector3f(builder, *baseDirection))
	StandardParticleEmitterFactory.AddDirectionSpread(builder, directionSpread)
	StandardParticleEmitterFactory.AddSpawnTimeRange(builder,
		CreateVector2f(builder, *spawnTimeRange))
	StandardParticleEmitterFactory.AddActiveTimeRange(builder,
		CreateVector2f(builder, *activeTimeRange))
	StandardParticleEmitterFactory.AddSpeedRange(builder, CreateVector2f(builder, *speedRange))
	StandardParticleEmitterFactory.AddRotationSpeedRange(builder,
		CreateVector2f(builder, *rotationSpeedRange))
	StandardParticleEmitterFactory.AddTextureRange(builder, CreateVector2u(builder, *textureRange))
	StandardParticleEmitterFactory.AddColorHueRange(builder,
		CreateVector2f(builder, *colorHueRange))
	StandardParticleEmitterFactory.AddColorSaturationRange(builder,
		CreateVector2f(builder, *colorSaturationRange))
	StandardParticleEmitterFactory.AddColorValueRange(builder,
		CreateVector2f(builder, *colorValueRange))
	StandardParticleEmitterFactory.AddColorAlphaRange(builder,
		CreateVector2f(builder, *colorAlphaRange))
	StandardParticleEmitterFactory.AddIntensityRange(builder,
		CreateVector2f(builder, *intensityRange))
	StandardParticleEmitterFactory.AddRelativeNode(builder, relativeNodeOffset)
	StandardParticleEmitterFactory.AddSeed(builder, seed)
	StandardParticleEmitterFactory.AddEnabled(builder, enabled)
	StandardParticleEmitterFactory.AddStartTime(builder, startTime)
	builder.Finish(StandardParticleEmitterFactory.End(builder))
	return builder.Output()
