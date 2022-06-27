# Scene Lighting

The DeepSea Scene Lighting library provides various structures, routines, and shaders to apply lighting to a scene. This includes classical forward lighting, deferred lighting, shadows, and ambient occlusion.

# Additional Conversion Types

The following JSON formats are added to extend scene conversion.

## Scene Resources

The following custom scene resource types are provided with the members that are expected:

* `"LightSet"`: set of lights to use within a scene.
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
* `"ShadowManager"`: object to manage shadow buffers within a scene.
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
		* `minDepthRanges`: minimim distance between the near and far planes for each cascade. Spot and point light shadows only use the first value. Can either be an array to set the cascade values or a float to set all 4 possible cascade values.
		* `fadeStartDistance`: the distance to start fading out shadows. Defaults to 1000000, which is a large distance less likely to break GPUs that use limited precision floats.
		* `maxDistance`: the maximum distance to display shadows. Defaults to 1000000, which is a large distance less likely to break GPUs that use limited precision floats.

## Global Data

The following global data types are provided with the members that are expected:

* `"LightSetPrepare"`: prepares a light set to be used in a scene before drawing.
	* `lightSets`: array of light set names to prepare.
	* `intensityThreshold`: the threshold below which the light is considered out of view. If unset this will use the default.
* `"ShadowManagerPrepare"`: prepares a shadow manager to be used in a scene before drawing.
	* `shadowManager`: name of the shadow manager to prepare.

## Item Lists

The following item list types are provided with the members that are expected:

* `"DeferredLightResolve"`: resolves the results of deferred lighting to the screen.
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
	* `shadowDirectional`: object containing info for shadowed directional lights. If omitted, shadowed directional lights won't be drawn. It is expected to contain the following elements:
		* `shader`: the name of the shader to draw the light.
		* `material`: the name of the material to use with the light shader.
		* `transformGroup`: name of the shader variable group containing the shadow transform.
		* `shadowTexture`: name of the shader variable for the the shadow texture.
	* `shadowPoint`: object containing info for shadowed point lights. If omitted, shadowed spot lights won't be drawn. It is expected to contain the following elements:
		* `shader`: the name of the shader to draw the light.
		* `material`: the name of the material to use with the light shader.
		* `transformGroup`: name of the shader variable group containing the shadow transform.
		* `shadowTexture`: name of the shader variable for the the shadow texture.
	* `spot`: object containing info for shadowed spot lights. If omitted, shadowed spot lights won't be drawn. It is expected to contain the following elements:
		* `shader`: the name of the shader to draw the light.
		* `material`: the name of the material to use with the light shader.
		* `transformGroup`: name of the shader variable group containing the shadow transform.
		* `shadowTexture`: name of the shader variable for the the shadow texture.
	* `intensityThreshold`: the threshold below which the light is considered out of view. If unset this will use the default.
* `"ShadowCullList"`: culls nodes that derive from `dsSceneCullNode` with a shadow surface.
	* `shadowManager`: name of the shadow manager that contains the shadows being culled for.
	* `shadows`: name of the shadows within the shadow manager to cull for.
	* `surface`: index of the surface within the light shadows.
* `"SSAO"`: calculates screen-space ambient occlusion with traditional pixel shaders.
	* `shader`: the shader to calculate the ambient occlusion with.
	* `material`: the material to use with the shader.
* `"ComputeSSAO"`: calculates screen-space ambient occlusion with a compute shader.
	* `shader`: the compute shader to calculate the ambient occlusion with.
	* `material`: the material to use with the shader.

## Instance Data

The following instance data types are provided with the members that are expected:

* `"InstanceForwardLightData"`: sets the brightest lights for the instance to shader variables for use in drawing.
	* `variableGroupDesc`: string name for the shader variable group to use.
	* `lightSet`: string name of the light set to use.
* `"ShadowInstanceTransformData"`: sets instance transforms for shadow mapping.
	* `shadowManager`: name of the shadow manager that contains the shadows to get the transform from.
	* `shadows`: name of the shadows within the shadow manager to get the transform from.
	* `surface`: index of the surface within the shadows to get the transform from.
	* `variableGroupDesc`:  name for the shader variable group to use.


# Advanced lighting techniques

The TestLighitng tester demonstrates the main lighting types that can be used with this library. This includes:

* Standard forward lighting. This is achieved by using `dsLightSetPrepare` to prepare the lights at the start of the scene, then utilizing the `dsInstanceForwardLightData` instance data object in the `dsSceneModelList` instances for the models to compute which lights to use for each model.
* Deferred lighting. This uses a render pass with two subpasses, first to draw the gbuffers and second to draw the lights. The gbuffer rendering use standard `dsSceneModelList` objects to draw to multiple render targets in the shader, then uses `dsDeferredLightResolve` to draw the lights. The shader code for each light type can be found under the `DeepSea/SceneLighting/Shaders` include directory. (e.g. `DeepSea/SceneLighting/Shaders/DeferredPointLight.mslh`)
* Deferred lighting with screen-space ambient occlusion (SSAO). This adds a pre-pass to write the depth and simplified normal without normal map. The `dsSceneSSAO` object is used to calculate the SSAO with a shader based on `DeepSea/SceneLighting/Shaders/SSAO.mslh`. After the ambient-occlusion is computed, the deferred lighting is computed similarly to before, except the ambient shader queries the SSAO value with `DeepSea/SceneLighting/Shaders/QuerySSAO.mslh`.
* All testers use shadows to some extent. A `dsShadowManger` object in the scene resources is used in conjunction with `dsShadowManagerPrepare` to make shadows available within the scene. `dsShadowCullList` instances are used for each shadow surface to perform the cull checks. In the case of forward lighting, the shadow map is set on the shader with Global material binding and the transform data is set when drawing the models with `dsShadowInstanceTransformData`. In the case of deferred lighting, the `dsDeferredLightResolve` instance will check if the light being drawn has shadows associated with it, and if so will use the shadow light shader to draw the light with the appropriate uniforms bound.
