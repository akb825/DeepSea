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
	* `mainLight`: the name of the main light. If omitted no light will be considered the main light.
	* `srgb`: true to treat all color values as sRGB values to be converted to linear space. Defaults to `false`.
* `"ShadowManager"`
	* `lightSet`: the name of the light set to query the light from. If set, this will be the default for elements in the shadows array.
	* `shadows`: array of objects for the shadows the shadow manager will manage. Each element is
	  expected to have the following members:
		* `name`: name of the shadows.
		* `lightSet`: name of the light set to query the light from.
		* `lightType`: type of the light to shadow. See `dsSceneLightType` enum for values, removing the type prefix.
		* `light`: name of the light to shadow. May be unset to disable initially until set at runtime.
		* `transformGroupDesc`: name of the shader variable group description for the transform group.
		* `transformGroupName`: name of the transform group to set as view global data. This may be omitted if not used as global data on a view.
		* `maxCascades`: the maximum number of cascades for cascaded directional light shadows. Defaults to 4.
		* `maxFirstSplitDistance`: maximum distance for the first split for cascaded shadows. Defaults to 100.
		* `cascadeExpFactor`: exponential factor for cascaded shadows in the range \[0, 1\], where 0 uses linear distances between the splits and 1 is fully exponential. Defaults to 0.5.
		* `fadeStartDistance`: the distance to start fading out shadows. Defaults to 1000000, which is a large distance less likely to break GPUs that use limited precision floats.
		* `maxDistance`: the maximum distance to display shadows. Defaults to 1000000, which is a large distance less likely to break GPUs that use limited precision floats.

## Global Data

The following global data types are provided with the members that are expected:
* `"LightSetPrepare"`:
	* `lightSets`: array of light set names to prepare.
	* `intensityThreshold`: the threshold below which the light is considered out of view. If unset this will use the default.
* `"ShadowManagerPrepare"`:
	* `shadowManager`: name of the shadow manager to prepare.

## Item Lists

The following item list types are provided with the members that are expected:
* `"DeferredLightResolve"`:
	* `lightSet`: name of the light set to draw the lights from.
	* `shadowManager`: name of the shadow manager when drawing shadowed lights.
	* `ambient`: object containing info for the ambient light. If omitted, the ambient light won't be drawn. It is expected to contain the following elements:
		* `shader`: the name of the shader to draw the light.
		* `material`: the name of the material to use with the light shader.
	* `directional`: object containing info for non-shadowed directional lights. If omitted, non-shadowed directional lights won't be drawn. It is expected to contain the following elements:
		* `shader`: the name of the shader to draw the light.
		* `material`: the name of the material to use with the light shader.
	* `point`: object containing info for non-shadowed point lights. If omitted, non-shadowed spot lights won't be drawn. It is expected to contain the following elements:
		* `shader`: the name of the shader to draw the light.
		* `material`: the name of the material to use with the light shader.
	* `spot`: object containing info for non-shadowed spot lights. If omitted, non-shadowed spot lights won't be drawn. It is expected to contain the following elements:
		* `shader`: the name of the shader to draw the light.
		* `material`: the name of the material to use with the light shader.
	* `shadowDirectional`: object containing info for shadowed directional lights. If omitted, shadowedDirectional: lights won't be drawn. It is expected to contain the following elements:
		* `shader`: the name of the shader to draw the light.
		* `material`: the name of the material to use with the light shader.
		* `transformGroup`: name of the shader variable group containing the shadow transform.
	* `shadowPoint`: object containing info for shadowed point lights. If omitted, shadowed spot lights won't be drawn. It is expected to contain the following elements:
		* `shader`: the name of the shader to draw the light.
		* `material`: the name of the material to use with the light shader.
		* `transformGroup`: name of the shader variable group containing the shadow transform.
	* `spot`: object containing info for shadowed spot lights. If omitted, shadowed spot lights won't be drawn. It is expected to contain the following elements:
		* `shader`: the name of the shader to draw the light.
		* `material`: the name of the material to use with the light shader.
		* `transformGroup`: name of the shader variable group containing the shadow transform.
	* `intensityThreshold`: the threshold below which the light is considered out of view. If unset this will use the default.
* `"ShadowCullList"`
	* `shadowManager`: name of the shadow manager that contains the shadows being culled for.
	* `shadows`: name of the shadows within the shadow manager to cull for.
	* `surface`: index of the surface within the light shadows.

## Instance Data

The following instance data types are provided with the members that are expected:

* `"InstanceForwardLightData"`:
	* `variableGroupDesc`: string name for the shader variable group to use.
	* `lightSet`: string name of the light set to use.
* `"ShadowInstanceTransformData"`:
	* `shadowManager`: name of the shadow manager that contains the shadows to get the transform from.
	* `shadows`: name of the shadows within the shadow manager to get the transform from.
	* `surface`: index of the surface within the shadows to get the transform from.
	* `variableGroupDesc`:  name for the shader variable group to use.
