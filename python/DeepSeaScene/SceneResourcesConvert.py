# Copyright 2020 Aaron Barany
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

from .SceneResources import *

def convertSceneResources(convertContext, data):
	"""
	Converts SceneResources. The data map is expected to contain the following optional element,
	where empty arrays at the top level may be ommitted:
	- buffers: array of buffers to embed. Each element of the array has the following members:
	  - name: string name of the buffer.
	  - usage: array of usage flags. See the dsGfxBufferUsage enum for values, removing the type
	    prefix. At least one must be provided.
	  - memoryHints: array of memory hints. See the dsGfxMemory enum for values, removing the type
	    prefix. At least one must be provided.
	  - size: the size of the buffer. This can be ommitted if data is provided.
	  - data: path to the buffer data or base64 encoded data prefixed with "base64:".
	- texture: array of textures to include. Each element of the array has the following members:
	  - name: the name of the texture.
	  - usage: array of usage flags. See the dsGfxBufferUsage enum for values, removing the type
	    prefix. At least one must be provided.
	  - memoryHints: array of memory hints. See the dsGfxMemory enum for values, removing the type
	    prefix. At least one must be provided.
	  - path: the path to the image. This may be ommitted if no initial texture data is used.
	    This may be an array of paths if converting a texture array or cubemap.
	  - output: the path to the output the texture. This can be ommitted if no input path is
	    provided, or if the texture is embedded. When converting textures, the extension should
	    match the desired output container format.
	  - outputRelativeDir: the directory relative to output path. This will be removed from the path
	    before adding the reference.
	  - resourceType: the resource type. See the dsFileResourceType for values, removing the type
	    prefix. Defaults to "Embedded".
	  - textureInfo: the info to describe the texture. This must be provided if no image path is
	    provided or if the image from the input path is to be converted. This is expected to have
	    the following elements:
	    - format: the texture format. See the dsGfxFormat enum for values, removing the type prefix.
	      The decorator values may not be used.
	    - decoration: the decoration for the format. See the dsGfxFormat enum for values, removing
	      the type prefix. Only the decorator values may be used.
	    - dimension: the dimension of the texture. See the dsTextureDim enum for values, removing
	      the type prefix.
	    - width: the width of the texture in pixels. When converting, may also be the string
	      nextpo2 or nearestpo2.
	    - height: the height of the texture in pixels. When converting, may also be the string
	      nextpo2 or nearestpo2.
	    - depth: the depth or array layers of the textures. If 0 or ommitted, this is not a texture
	      array.
	    (the following elements are only used for texture conversion)
	    - mipLevels: the number of mipmap levels.
	    - quality: the quality to use during conversion. May be one of lowest, low, normal, high,
	      or highest. Defaults to normal.
	    - normalmap: float value for a height to use to convert to a normalmap.
	    - swizzle: string of R, G, B, A, or X values to swizzle the color channels.
	    - rotate: angle to rotate. Must be a multile of 90 degrees.
	    - alpha: the alpha mode to use. Must be: none, standard, pre-multiplied, encoded. Default
	      value is standard.
	    - transforms: array of transforms to apply. Valid values are: flipx, flipy, srgb,
	      and grayscale. Note that srgb is implied if the decorator is srgb.
	- shaderVariableGroupDescs: array of shader variable group descriptions to include. Each
      element of the array has the following members:
	  - name: the name of the shader variable group description.
	  - elements: array of elements for the shader variable group. Each element of the array has
	    the following members:
	    - name: the name of the element.
	    - type: the type of the element. See dsMaterialType enum for values, removing the type
	      prefix.
	    - count: the number of array elements. If 0 or ommitted, this is not an array.
	- shaderVariableGroups: array of shader variable groups to include. Each element of the array
	  has the following members:
	  - name: the name of the element.
	  - description: the name of the description defined in shaderVariableGroupDescs. The
	    description may be in a different scene resources package.
	  - data: array of data elements to set. Each element of the array has the following members:
	    - name: the name of the data element.
	    - type: the type of the element. See the dsMaterialType enum for values, removing the type
	      prefix.
	    - first: the index of the first element to set when it's an array. Defaults to 0 if not set.
	    - data: the data to set, with the contents depending on the "type" that was set. Vector
	      types are arrays, while matrix types are arrays of column vector arrays. Texture, image,
	      buffer, and shader variable group types are string names.
	    - dataArray: this may be set in place of the data member to provide an array of data
	      elements rather than a single one.
	- materialDescs: array of material descriptions to include. Each element of the array has the
	  following members:
	  - name: the name of the material description.
	  - elements: array of elements for the material. Each element of the array has the following
	    members:
	    - name: the name of the element.
	    - type: the type of the element. See dsMaterialType enum for values, removing the type
	      prefix.
	    - count: the number of array elements. If 0 or ommitted, this is not an array.
		- binding: the binding type for the element. See the dsMaaterialBinding enum for values,
	      removing the type prefix. This is only used for texture, image, buffer, and shader
		  variable group types.
	    - shaderVariableGroupDesc: the name of the shader variable group description when the type
	      is a shader variable group. The description may be in a different scene resources package.
	- materials: array of materials to include. See shaderVariableGroups for a description of the
	  array members, except the "description" element is for a material description rather than a
	  shader variable group description.
	- shaderModules: array of shader modules to include. Each element of the array has the following
	  members:
	  - name: the name of the shader module.
	  - path: the path to the shader module file compiled with Modular Shader Language (MSL).
	  - output: the path to the location to copy the shader module to. This can be ommitted to embed
	    the shader module directly.
	  - outputRelativeDir: the directory relative to output path. This will be removed from the path
	    before adding the reference.
	  - resourceType: the resource type. See the dsFileResourceType for values, removing the type
	    prefix. Defaults to "Embedded".
	- shaders: array of shaders to include. Each element of the array has the following members:
	  - name: the name of the shader.
	  - shaderModule: the name of the shader module the shader resides in. The shader module may be
	    in a different scene resources package.
	  - pipelineName: the name of the shader pipeline within the shader module.
	  - materialDesc: The name of the material description for materials that will be used with the
	    shader. The material may be in a different scene resources package.
	- drawGeometries: array of draw geometries to include. Each element of the array has the
	  following members:
	  - name: the name of the draw geometry.
	  - vertexBuffers: array of vertex buffers. This can have up to 4 elements with the following
	    members:
	    - name: the name of the buffer to use. The buffer may be in a different scene resources
	      package.
	    - offset: the offset in bytes into the buffer to the first vertex.
	    - count: the number of vertices in the buffer.
	    - format: the vertex format. This is a dict with the following members:
	      - attributes: array of attributes for the format. Each element has the following members:
	        - format: the attribute format. See the dsGfxFormat enum for values, removing the type
	          prefix. Only the "standard" formats may be used.
	        - decoration: the decoration for the format. See the dsGfxFormat enum for values,
	          removing the type prefix. Only the decorator values may be used.
	      - instanced: true if the vertex data is instanced. Defaults to false.
	  - indexBuffer: the index buffer. If not set, the draw geometry isn't indexed. This is a dict
	    with the following members:
	    - name: the name of the buffer to use. The buffer may be in a different scene resources
	      package.
	    - offset: the offset in bytes into the buffer to the first index.
	    - count: the number of indices in the buffer.
	    - indexSize: the size of the index in bytes. This must be either 2 or 4.
	- sceneNodes: array of scene nodes to include. Each element of the array has the following
	  members:
	  - name: the name of the node.
	  - type: the name of the node type.
	  - data: the data for the node. What this member contains (e.g. a string or a dict with other
	    members) depends on the node type.
	"""
	pass
