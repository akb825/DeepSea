# Scene Lighting

The DeepSea Scene Lighting library provides various structures, routines, and shaders to apply lighting to a scene. This includes classical forward lighting, deferred lighting, shadows, and ambient occlusion.

# Additional Conversion Types

The following JSON formats are added to extend scene conversion.

## Scene Resources

The following custom scene resource types are provided with the members that are expected:

* `"LightSet"`
	* `lights`: array of lights to initially populate the light set with. Each member of the array has the following members:
		* `name`: the name of the light.
		* `color`: the color of the light as an array of three float values, typically in the range `[0, 1]`.
		* `intensity`: the intensity of the light, which multiplies the color.
		* `type`: the type of the light. The following types are supported with the members they expect:
			* `"Directional"`
				* `direction`: direction of the light as an array of three float values.
			* `"Point"`
				* `position`: position of the light as an array of three float values.
				* `linearFalloff`: amount the light falls off based on distance.
				* `quadraticFalloff`: amount the light falls off based on squared distance.
			* `"Spot"`
				* `position`: position of the light as an array of three float values.
				* `direction`: direction of the light as an array of three float values.
				* `linearFalloff`: amount the light falls off based on distance.
				* `quadraticFalloff`: amount the light falls off based on squared distance.
				* `innerSpotAngle`: the angle in degrees of the spot light where it starts to fade out.
				* `outerSpotAngle`: the angle in degrees of the spot light where it finishes fade out.
	* `maxLights`: the maximum number of lights that can be stored. If unset, the number of elements in lights will be used.
	* `ambientColor`: the color of the ambient light as an array of three floats, typically in the range `[0,1]`. Defaults to all 0.
	* `ambientIntensity`: the intensity of the ambient light, which multiplies the color. Defaults to 0.
	* `srgb`: true to treat all color values as sRGB values to be converted to linear space. Defaults to `false`.

## Item Lists

The following item list types are provided with the members that are expected:
* `"LightSetPrepare"`:
	* `lightSets`: array of light set names to prepare.
	* `intensityThreshold`: the threshold below which the light is considered out of view. If unset this will use the default.

## Instance Data

The following instance data types are provided with the members that are expected:

* `"InstanceForwardLightData"`:
	* `variableGroupDesc`: string name for the shader variable group to use.
	* `lightSet`: string name of the light set to use.
