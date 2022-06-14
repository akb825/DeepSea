# Scene Particle

The DeepSea Scene Particle library integrates the Particle and Scene libraries to draw particles within a scene.

# Additional Conversion Types

The following JSON formats are added to extend scene conversion.

## Scene Resources

The following custom scene resource types are provided with the members that are expected:

* `"StandardParticleEmitterFactory"`:
	* `maxParticles`: the maximum number of particles displayed at once.
	* `shader`: the name of the shader to draw with.
	* `material`: the name of the material to draw with.
	* `instanceValueCount`: optional number of material instance values. The max between this and the instance value count of material will be used, which can be used to allow swapping out for different materials at runtime. Defaults to 0.
	* `spawnVolume`: the volume to spawn particles in. This is expected to contain the following members:
		* `type`: they type of the volume. May be Box, Sphere, or Cylinder. The following members depend on the type.
		* Box:
			* `bounds`: 2x3 array of float values for the minimum maximum values of the box.
		* Sphere:
			* `center`: array of 3 floats for the center of the sphere.
			* `radius`: float for the radius of the sphere.
		* Cylinder:
			* `center`: array of 3 floats for the center of the cylinder.
			* `radius`: float for the radius of the cylinder.
			* `height`: float for the height of the cylinder along the Z axis.
	* `spawnVolumeTransformList`: optional array of transforms to apply to the volume. If unset the transform will be the identiy matrix. When provided, the matrices for the transforms provided are multiplied in reverse order given (i.e. in logical transform order), starting from the identity matrix. Each member of the array has the following elements:
		* `type`: the type of transform. May be Rotate, Scale, Translate, or Matrix.
		* `value`: the value of transform based on the type:
			* Rotate: array of 3 floats for the X, Y, and Z rotation in degrees.
			* Scale: array of 3 floats for the scale value along the X, Y, and Z axes.
			* Translate: array of 3 floats for the translation along X, Y, and Z.
			* Matrix: a 4x4 array of floats for a matrix. Each inner array is a column of the matrix.
	* `widthRange`: array of 2 floats for the minimum and maximum width values.
	* `heightRange`: optional array of 2 floats for the minimum and maximum height values. If unsset the width values will be used to keep the particles square.
	* `baseDirection`: array of 3 floats for the base direction the particles will move along.
	* `directionSpread`: spread along the base direction for particles to move along as an angle in degrees.
	* `spawnTimeRange`: array of 2 floats for the minimum and maximum time in seconds between spawning of particles.
	* `activeTimeRange`: array of 2 floats for the minimum and maximum time in seconds a particle is active for.
	* `speedRange`: array of 2 floats for the minimum and maximum speed a particle travels at.
	* `rotationSpeedRange`: array of 2 floats for the minimum and maximum rotation speed of a particle in degrees per second.
	* `textureRange`: optional array of 2 integers for the minimum and maximum texture index ranges. Defaults to [0, 0] if unset.
	* `colorHueRange`: array of 2 floats for the minimum and maximum color hue n the range [0, 360]. The minimum may be larger than the maximum to wrap around at 360.
	* `colorSaturationRange`: array of 2 floats for the minimum and maximum color saturation in the range [0, 1].
	* `colorValueRange`: array of 2 floats for the minimum and maximum color value in the range [0, 1].
	* `intensityRange`: array of 2 floats for the minimum and maximum intensity of a particle.
	* `relativeNode`: optional name of a node to transform the particles relative to. When set, the particles will use the transform of relativeNode, while the volume boundary to spawn the particles will be relative to the particle emitter's node. This must be an ancestor of the node the particle emitter will be created with.
	* `seed`: optional random seed to create the factory with. If unset or 0 a random seed will be generated.
	* `enabled`: optional bool for whether or not the particle emitter will be enabled on creation. Defaults to `true`.
	* `startTime`: optional float for the time in seconds to advance the particle emitter on creation. Defaults to `0`.

## Scene Nodes

The following scene node types are provided with the members that are expected:

* `"ParticleNode"`
	* `particleEmitterFactory`: the name of the factory to create particle emitters with.
	* `itemLists`: array of item list names to add the node to.

> **Note:** When creating a `ParticleNode`, it should have a `ParticlePrepareList` followed by at least one `ParticleDrawList`. The `ParticlePrepareList` *must* be before any `ParticleDrawList` entries to ensure the particle emitter is available.

## Scene Item Lists

The following scene item lists are provided with the expected members:

* `"ParticlePrepareList"` (no additional members)
* `ParticleDrawList"`
	* `instanceData`: optional list of instance data to include with the particle draw list. Each element of the array has the following members:
		* `type`: the name of the instance data type.
		* Remaining members depend on the value of `type`.