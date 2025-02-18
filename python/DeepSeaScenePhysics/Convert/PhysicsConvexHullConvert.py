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

import os
import struct

import flatbuffers
from .Helpers import readFloat
from DeepSeaPhysics.ShapeUnion import ShapeUnion
from DeepSeaPhysics.Vector3f import CreateVector3f
from DeepSeaPhysics import ConvexHull
from DeepSeaPhysics import Shape
from DeepSeaScene.Convert.ModelConvert import VertexAttrib, loadAndConvertModelGeometry, \
	modelVertexAttribEnum

def convertPhysicsConvexHullOffset(convertContext, data, builder):
	try:
		vertexData = data.get('vertices')
		vertices = []
		if vertexData:
			if not isinstance(vertexData, list) or len(vertexData) % 3 != 0:
				raise Exception('PhysicsConvexHull "vertices" must be a list of floats with a '
					'length divisible by 3.')
			for f in vertexData:
				vertices.append(readFloat(f, 'vertex'))
		else:
			path = str(data['path'])
			modelType = str(data.get('modelType', ''))
			if not modelType:
				modelType = os.path.splitext(path)[1]
				if modelType:
					modelType = modelType[1:]
				if not modelType:
					raise Exception('PhysicsConvexHull geometry has no known model type.')
			component = str(data.get('component'))
			vertexBytes = bytearray()
			loadAndConvertModelGeometry(convertContext, modelType, path, [component],
				[[VertexAttrib(modelVertexAttribEnum['Position'], 'X32Y32Z32', 'Float')]], None,
				None, vertexBytes, pointsOnly = True)
			for i in range(len(vertexBytes)/12):
				vertices.extend(struct.unpack_from('fff', vertexBytes, i*12))

		convexRadius = readFloat(data.get('convexRadius', -1), 'convex radius')
		cacheName = str(data.get('cacheName', ''))
	except (TypeError, ValueError):
		raise Exception('PhysicsConvexHull data must be an object.')
	except KeyError as e:
		raise Exception('PhysicsConvexHull data doesn\'t contain element ' + str(e) + '.')

	ConvexHull.StartVerticesVector(builder, len(vertices))
	for f in reversed(vertices):
		builder.PrependFloat32(f)
	verticesOffset = builder.EndVector()

	if cacheName:
		cacheNameOffset = builder.CreateString(cacheName)
	else:
		cacheNameOffset = 0

	ConvexHull.Start(builder)
	ConvexHull.AddVertices(builder, verticesOffset)
	ConvexHull.AddConvexRadius(builder, convexRadius)
	ConvexHull.AddCacheName(builder, cacheNameOffset)
	convexHullOffset = ConvexHull.End(builder)

	Shape.Start(builder)
	Shape.AddShapeType(builder, ShapeUnion.ConvexHull)
	Shape.AddShape(builder, convexHullOffset)
	return Shape.End(builder)

def convertPhysicsConvexHull(convertContext, data, outputDir):
	"""
	Converts a PhysicsConvexHull. The data map is expected to contain the following elements:
	- The following elements are used when providing data from a model:
	  - modelType: the name of the model type, such as "obj" or "gltf". If omitted, the type is
	    inferred from the path extension.
	  - path: the path to the geometry.
	  - component: the name of the component of the model.
	- The following elements are used when providing data directly:
	  - vertices: array of floats for the raw vertex data. This must be divisible by 3, with each
	    vertex having three values.
	- convexRadius: the convex radius for collision checks. If unset or a value < 0 the physics
	  system's default will be used.
	- cacheName: name used for caching pre-computed data. If not set, the pre-computed data will
	  not be cached.
	"""
	builder = flatbuffers.Builder(0)
	builder.Finish(convertPhysicsConvexHullOffset(builder, data, builder))
	return builder.Output()
