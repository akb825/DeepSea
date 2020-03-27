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

import flatbuffers
from .ModelNode import *

attribEnum = {
	'Position': 0,
	'Position0': 0,
	'Position1': 1,
	'Normal': 2,
	'Color': 3,
	'Color0': 3,
	'Color1': 4,
	'FogCoord': 5,
	'Tangent': 6,
	'Bitangent': 7,
	'TexCoord0': 8,
	'TexCoord1': 9,
	'TexCoord2': 10,
	'TexCoord3': 11,
	'TexCoord4': 12,
	'TexCoord5': 13,
	'TexCoord6': 14,
	'TexCoord7': 15,
	'BlendIndices': 14,
	'BlendWeights': 15
}

def convertModelNode(convertContext, data):
	"""
	Converts a ModelNode. The data map is expected to contain the following elements:
	- embeddedResources: optional set of resources to embed with the node. This is a map containing
	  the elements as expected by SceneResourcesConvert.convertSceneResources().
	- modelGeometry: array of model geometry. Each element of the array has the following members:
	  - name: the name of the geometry.
	  - type: the name of the geometry type, such as "obj" or "gltf".
	  - path: the path to the geometry.
	  - vertexFormat: the vertex format. This is a dict with the following members:
	    - attributes: array of attributes for the format. Each element of the array has the
	      following members:
		  - attrib: the attribute. This can either be an enum value from dsVertexAttrib, removing
		    the type prefix, or the integer for the attribute.
	      - format: the attribute format. See the dsGfxFormat enum for values, removing the type
	        prefix. Only the "standard" formats may be used.
	      - decoration: the decoration for the format. See the dsGfxFormat enum for values,
	        removing the type prefix. Only the decorator values may be used.
	    - instanced: true if the vertex data is instanced. Defaults to false.
	  - indexSize: the size of the index in bytes. This must be either 2 or 4. If not set, no
	    indices will be produced.
	  - primitiveType: the type of primitives. See the dsPrimitiveType enum for values, removing the
	    type prefix. Defaults to "TriangleList".
	  - patchPoints: the number of points when primitiveType is "PatchList".
	  - transforms: optional array of transforms to perform on the vertex values. Each element of
	    the array has the following members:
	    - attrib: the attribute, matching one of the attributes in vertexFormat.
	    - transform: transform to apply on the attribute. Valid values are:
	      - Identity: leaves the values un-transformed.
	      - Bounds: normalizes the values based on the original value's bounds
	      - UNormToSNorm: converts UNorm values to SNorm values.
	      - SNormToUNorm: converts SNorm values to UNorm values.
	  - drawInfo: array of definitions for drawing components of the geometry. Each element of the
	    array has the following members:
	    - name: the name of the model component.
	    - shader: te name of the shader to draw with.
	    - material: the name of the material to draw with.
	    - distanceRange: array of two floats for the minimum and maximum distance to draw at.
	      Defaults to [0, 3.402823466e38].
	    - listName The name of the item list to draw the model with.
	- models: array of models to draw with manually provided geometry. (i.e. not converted from
	  the modelGeometry array) Each element of the array has the following members:
	  - shader: the name of the shader to draw with.
	  - material: the name of the material to draw with.
	  - geometry: the name of the geometry to draw.
	  - distanceRange: array of two floats for the minimum and maximum distance to draw at. Defaults
	    to [0, 3.402823466e38].
	  - drawRange: the range of the geometry to draw. This is an object with the following members,
	    depending on if the geometry is indexed or not:
	    Indexed geometry:
	    - indexCount: the number of indices to draw.
	    - instanceCount: the number of instances to draw. Defaults to 1.
	    - firstIndex: the first index to draw. Defaults to 0.
	    - vertexOffset: the offset to apply to each index value. Defaults to 0.
	    - firstInstance: the first instance to draw. Defaults to 0.
	    Non-indexed geometry:
	    - vertexCount: the number of vertices to draw.
	    - instanceCount: the number of instances to draw. Defaults to 1.
	    - firstVertex: the first vertex to draw. Defaults to 0.
	    - firstIstance: the first instance to draw. Defaults to 0.
	  - primitiveType: the primitive type to draw with. See the dsPrimitiveType enum for values,
	    removing the type prefix. Defaults to "TriangleList".
	  - listName The name of the item list to draw the model with.
	- extraItemLists: array of extra item list names to add the node to.
	- bounds: 2x3 array of float values for the minimum and maximum values for the positions. This
	  will be automatically calculated from geometry in modelGeometry if unset. Otherwise if unset
	  the model will have no explicit bounds for culling.
	"""
