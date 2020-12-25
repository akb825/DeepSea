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
from .SceneResourcesConvert import memoryHintsEnum
from ..AlignedBox3f import *
from ..CubeFace import *
from ..FormatDecoration import *
from ..Framebuffer import *
from ..FramebufferSurface import *
from ..Surface import *
from ..SurfaceType import *
from ..TextureDim import *
from ..TextureFormat import *
from ..View import *

class Object:
	pass

renderbufferUsageEnum = {
	'Standard': 0x0,
	'BlitFrom': 0x1,
	'BlitTo': 0x2,
	'Continue': 0x4
}

textureUsageEnum = {
	'Texture': 0x1,
	'Image': 0x2,
	'SubpassInput': 0x4,
	'CopyFrom': 0x8,
	'CopyTo': 0x10,
	'OffscreenContinue': 0x20
}

def convertView(convertContext, data):
	"""
	Converts a View. The data is expected to contain the following elements:
	- surfaces: array of surfaces to use within the view. Each element of the array contains the
	  following members:
	  - name: the name of the surface.
	  - type: the type of the surface. May either be "Renderbuffer" or "Offscreen".
	  - usage: array of usage flags. See the dsRenderbufferUsage enum (for renderbuffers) and 
	    dsTextureUsage enum (for offscreens) for values, removing the type prefix. Defaults to
	    ["Standard"] for renderbuffers or ["Texture"] for offscreens.
	  - memoryHints: array of memory hints. See the dsGfxMemory enum for values, removing the type
	    prefix. Defaults to ["GPUOnly"].
	  - dimension: dimension for an offscreen. See the dsTextureDim enum for values, removing the
	    type prefix. Defaults to Dim2D.
	  - format: the texture format. See the dsGfxFormat enum for values, removing the type prefix.
	    The decorator values may not be used. May also be "SurfaceColor" or "SurfaceDepthStencil"
	    to use the color or depth/stencil format for render surfaces.
	  - decoration: the decoration for the format. See the dsGfxFormat enum for values, removing the
	    type prefix. Only the decorator values may be used. May also be "Unset" in cases where a
	    decorator isn't valid.
	  - width: the explicit width of the surface.
	  - widthRatio: the ratio of the width relative to the view width. "width" should be omitted
	    when this is used. Defaults to 1.0 if neither width nor widthRatio is set.
	  - height: the explicit height of the surface.
	  - heightRatio: the ratio of the height relative to the view height. "height" should be omitted
	    when this is used. Defaults to 1.0 if neither height nor heightRatio is set.
	  - depth: the depth or array layers of the surface if an offscreen. If 0 or omitted, this is
	    not a texture array.
	  - mipLevels: the number of mipmap levels for an offscreen. Defaults to 1.
	  - samples: the number of anti-alias samples. When omitted, this uses the number of samples
	    set on the renderer for window surfaces.
	  - resolve: whether or not to resolve multisampled results.
	  - windowFramebuffer: Whether or not the surface is used in the same framebuffer as the window
	    surface. When true, the surface will follow the rotation of the view and window surface.
	    Defaults to true.
	- framebuffers: array of framebuffers to use in the view. Each element of the array has the
	  following members:
	  - name: the name of the framebuffer.
	  - surfaces: array of surfaces within the framebuffer. Each element of the array has the
	    following members:
	    - name: the name of the surface to use. This should either be present in the surfaces array
	      or provided to the View when loaded.
	    - face: cube face for when the surface is a cubemap. See the dsCubeFace enum for values,
	      removing the type prefix. Defaults to PosX.
	    - layer: the texture array or 3D texture level to use for an offscreen. Defaults to 0.
	    - mipLevel: the mip level to use for an offscreen. Defaults to 0.
	  - width: the explicit width of the framebuffer.
	  - widthRatio: the ratio of the width relative to the view width. "width" should be omitted
	    when this is used. Defaults to 1.0 if neither width nor widthRatio is set.
	  - height: the explicit height of the framebuffer.
	  - heightRatio: the ratio of the height relative to the view height. "height" should be
	    omitted when this is used. Defaults to 1.0 if neither height nor heightRatio is set.
	  - layers: the number of layers in the framebuffer. Defaults to 1.
	  - viewport: the viewport to use. This is a dict with the following elements.
	    - minX: the minimum X value for the upper-left position as a fraction of the width. Defaults 
	      to 0.
	    - minY: the minimum Y value for the upper-left position as a fraction of the height.
	      Defaults to 0.
	    - maxX: the maximum X value for the lower-right position as a fraction of the width.
	      Defaults to 1.
	    - maxY: the maximum Y value for the lower-right position as a fraction of the hieght.
	      Defaults to 1.
	    - minDepth: the minimum depth value. Defaults to 0.
	    - maxDepth: the maximum depth value. Defaults to 1.
	"""
	unsetValue = 0xFFFFFFFF

	def readFloat(value, name, minVal = None, maxVal = None):
		try:
			f = float(value)
			if (minVal is not None and f < minVal) or (maxVal is not None and f > maxVal):
				raise Exception()
			return f
		except:
			raise Exception('Invalid ' + name + ' float value "' + str(value) + '".')

	def readInt(value, name, minVal = None, maxVal = None):
		try:
			i = int(value)
			if (minVal is not None and i < minVal) or (maxVal is not None and i > maxVal):
				raise Exception()
			return i
		except:
			raise Exception('Invalid ' + name + ' int value "' + str(value) + '".')

	def readBool(value, name):
		try:
			return bool(value)
		except:
			raise Exception('Invalid ' + name + ' bool value "' + str(value) + '".')

	def readSurface(info):
		surface = Object()
		surface.name = str(info['name'])

		typeStr = str(info['type'])
		try:
			surface.type = getattr(SurfaceType, typeStr)
		except AttributeError:
			raise Exception('Invalid surface type "' + typeStr + '".')

		surface.usage = 0
		if surface.type == SurfaceType.Renderbuffer:
			try:
				for usageEnum in info.get('usage', []):
					try:
						surface.usage |= renderbufferUsageEnum[usageEnum]
					except:
						raise Exception('Invalid renderbuffer usage value "' +
							str(usageEnum) + '".')
			except (TypeError, ValueError):
				raise Exception('Attachment usage must be an array of valid '
					'dsRenderbufferUsage values.')
		else:
			try:
				for usageEnum in info.get('usage', ['Texture']):
					try:
						surface.usage |= textureUsageEnum[usageEnum]
					except:
						raise Exception('Invalid renderbuffer usage value "' +
							str(usageEnum) + '".')
			except (TypeError, ValueError):
				raise Exception('Attachment usage must be an array of valid '
					'dsTextureUsage values.')
			if not surface.usage:
				raise Exception('View surface "usage" must not be empty for offscreens.')

		if 'memoryHints' in info:
			surface.memoryHints = 0
			try:
				for memoryEnum in info['memoryHints']:
					try:
						surface.memoryHints |= memoryHintsEnum[memoryEnum]
					except KeyError as e:
						raise Exception('Invalid dsMemoryUsage enum value ' + str(e) + '.')
				if surface.memoryHints == 0:
					raise Exception('View surface "memoryHints" must not be empty.')
			except (ValueError, KeyError):
				raise Exception('SceneResources buffer "memoryHints" must be an array of valid '
					'dsGfxMemory enum values.')
		else:
			surface.memoryHints = memoryHintsEnum['GPUOnly']

		dimensionStr = info.get('dimension', 'Dim2D')
		try:
			surface.dimension = getattr(TextureDim, dimensionStr)
		except AttributeError:
			raise Exception('Invalid surface dimension "' + dimensionStr + '".')

		formatStr = info['format']
		try:
			surface.format = getattr(TextureFormat, formatStr)
		except AttributeError:
			raise Exception('Invalid surface format "' + formatStr + '".')

		decorationStr = info['decoration']
		try:
			surface.decoration = getattr(FormatDecoration, decorationStr)
		except AttributeError:
			raise Exception(
				'Invalid surface format decoration "' + decorationStr + '".')

		if 'width' in info:
			surface.width = readInt(info['width'], 'surface width', 0)
		else:
			surface.width = 0

		if 'widthRatio' in info:
			surface.width = readFloat(info['widthRatio'], 'surface width ratio', 0.0)
		else:
			surface.widthRatio = 0.0

		if not surface.width and not surface.widthRatio:
			surface.widthRatio = -1.0

		if 'height' in info:
			surface.height = readInt(info['height'], 'surface height', 0)
		else:
			surface.height = 0

		if 'heightRatio' in info:
			surface.height = readFloat(info['heightRatio'], 'surface height ratio', 0.0)
		else:
			surface.heightRatio = 0.0

		if not surface.height and not surface.heightRatio:
			surface.heightRatio = -1.0

		surface.depth = readInt(info.get('depth', 0), 'surface depth', 0)
		surface.mipLevels = readInt(info.get('mipLevels', 1), 'surface mip levels', 1)
		surface.samples = readInt(info.get('samples', unsetValue), 'surface samples', 1)
		surface.resolve = readBool(info['resolve'], 'surface resolve')
		print(surface.resolve)
		surface.windowFramebuffer = readBool(info.get('windowFramebuffer', True),
			'window framebuffer')
		return surface

	def readFramebufferSurface(info):
		surface = Object()
		surface.name = str(info['name'])

		faceStr = str(info.get('face', 'PosX'))
		try:
			surface.face = getattr(CubeFace, faceStr)
		except AttributeError:
			raise Exception('Invalid framebuffer surface face "' + faceStr + '".')

		surface.layer = readInt(info.get('layer', 0), 'framebuffer surface layer', 0)
		surface.mipLevel = readInt(info.get('mipLevel', 0), 'framebuffer surface mip level', 0)
		return surface

	def readFramebuffer(info):
		framebuffer = Object()
		framebuffer.name = str(info['name'])

		framebufferSurfaceInfos = info.get('surfaces', [])
		framebuffer.surfaces = []
		try:
			for surfaceInfo in framebufferSurfaceInfos:
				framebuffer.surfaces.append(readFramebufferSurface(surfaceInfo))
		except KeyError as e:
			raise Exception('Framebuffer surface doesn\'t contain element ' + str(e) + '.')
		except (TypeError, ValueError):
			raise Exception('Framebuffer surfaces must be an array of objects.')

		if 'widthRatio' in info:
			framebuffer.width = -readFloat(info['widthRatio'], 'framebuffer width ratio', 0.0)
		elif 'width' in info:
			framebuffer.width = float(readInt(info['width'], 'framebuffer width', 1))
		else:
			framebuffer.width = -1.0

		if 'heightRatio' in info:
			framebuffer.height = -readFloat(info['heightRatio'], 'framebuffer height ratio', 0.0)
		elif 'height' in info:
			framebuffer.height = float(readInt(info['height'], 'framebuffer height', 1))
		else:
			framebuffer.height = -1.0
			
		framebuffer.layers = readInt(info.get('layers', 1), 'framebuffer layers', 1)

		viewportInfo = info.get('viewport')
		if viewportInfo:
			try:
				framebuffer.viewport = (
					readFloat(viewportInfo.get('minX', 0.0),
						'framebuffer viewport min X', 0.0, 1.0),
					readFloat(viewportInfo.get('minY', 0.0),
						'framebuffer viewport min Y', 0.0, 1.0),
					readFloat(viewportInfo.get('minDepth', 0.0),
						'framebuffer viewport min depth', 0.0, 1.0),
					readFloat(viewportInfo.get('maxX', 1.0),
						'framebuffer viewport max X', 0.0, 1.0),
					readFloat(viewportInfo.get('maxY', 1.0),
						'framebuffer viewport max Y', 0.0, 1.0),
					readFloat(viewportInfo.get('maxDepth', 1.0),
						'framebuffer viewport max depth', 0.0, 1.0),
				)
			except AttributeError:
				raise Exception('View Framebuffer viewport must be an object.')
		else:
			framebuffer.viewport = None

		return framebuffer

	try:
		surfaceInfos = data.get('surfaces', [])
		surfaces = []
		try:
			for surfaceInfo in surfaceInfos:
				surfaces.append(readSurface(surfaceInfo))
		except KeyError as e:
			raise Exception('View "surfaces" doesn\'t contain element ' + str(e) + '.')
		except (TypeError, ValueError):
			raise Exception('View "surfaces" must be an array of array of objects.')

		framebufferInfos = data.get('framebuffers', [])
		framebuffers = []
		try:
			for framebufferInfo in framebufferInfos:
				framebuffers.append(readFramebuffer(framebufferInfo))
		except KeyError as e:
			raise Exception('View "framebuffers" doesn\'t contain element ' + str(e) + '.')
		except (TypeError, ValueError):
			raise Exception('View "framebuffers" must be an array of array of objects.')
	except KeyError as e:
		raise Exception('View doesn\'t contain element ' + str(e) + '.')
	except (AttributeError, TypeError, ValueError):
		raise Exception('View must be an object.')

	builder = flatbuffers.Builder(0)

	surfaceOffsets = []
	for surface in surfaces:
		nameOffset = builder.CreateString(surface.name)

		SurfaceStart(builder)
		SurfaceAddName(builder, nameOffset)
		SurfaceAddType(builder, surface.type)
		SurfaceAddUsage(builder, surface.usage)
		SurfaceAddMemoryHints(builder, surface.memoryHints)
		SurfaceAddFormat(builder, surface.format)
		SurfaceAddDecoration(builder, surface.decoration)
		SurfaceAddDimension(builder, surface.dimension)
		SurfaceAddWidth(builder, surface.width)
		SurfaceAddWidthRatio(builder, surface.widthRatio)
		SurfaceAddHeight(builder, surface.height)
		SurfaceAddHeightRatio(builder, surface.heightRatio)
		SurfaceAddDepth(builder, surface.depth)
		SurfaceAddMipLevels(builder, surface.mipLevels)
		SurfaceAddSamples(builder, surface.samples)
		SurfaceAddResolve(builder, surface.resolve)
		SurfaceAddWindowFramebuffer(builder, surface.windowFramebuffer)
		surfaceOffsets.append(SurfaceEnd(builder))

	if surfaceOffsets:
		ViewStartSurfacesVector(builder, len(surfaceOffsets))
		for offset in reversed(surfaceOffsets):
			builder.PrependUOffsetTRelative(offset)
		surfacesOffset = builder.EndVector(len(surfaceOffsets))
	else:
		surfacesOffset = 0

	framebufferOffsets = []
	for framebuffer in framebuffers:
		nameOffset = builder.CreateString(framebuffer.name)

		framebufferSurfaceOffsets = []
		for surface in framebuffer.surfaces:
			surfaceNameOffset = builder.CreateString(surface.name)

			FramebufferSurfaceStart(builder)
			FramebufferSurfaceAddName(builder, surfaceNameOffset)
			FramebufferSurfaceAddFace(builder, surface.face)
			FramebufferSurfaceAddLayer(builder, surface.layer)
			FramebufferSurfaceAddMipLevel(builder, surface.mipLevel)
			framebufferSurfaceOffsets.append(FramebufferSurfaceEnd(builder))

		if framebufferSurfaceOffsets:
			FramebufferStartSurfacesVector(builder, len(framebufferSurfaceOffsets))
			for offset in reversed(framebufferSurfaceOffsets):
				builder.PrependUOffsetTRelative(offset)
			framebufferSurfacesOffset = builder.EndVector(len(framebufferSurfaceOffsets))
		else:
			framebufferSurfacesOffset = 0

		FramebufferStart(builder)
		FramebufferAddName(builder, nameOffset)
		FramebufferAddSurfaces(builder, framebufferSurfacesOffset)
		FramebufferAddWidth(builder, framebuffer.width)
		FramebufferAddHeight(builder, framebuffer.height)
		FramebufferAddLayers(builder, framebuffer.layers)

		if framebuffer.viewport:
			viewportOffset = CreateAlignedBox3f(builder, *framebuffer.viewport)
		else:
			viewportOffset = 0
		FramebufferAddViewport(builder, viewportOffset)

		framebufferOffsets.append(FramebufferEnd(builder))

	ViewStartFramebuffersVector(builder, len(framebufferOffsets))
	for offset in reversed(framebufferOffsets):
		builder.PrependUOffsetTRelative(offset)
	framebuffersOffset = builder.EndVector(len(framebufferOffsets))

	ViewStart(builder)
	ViewAddSurfaces(builder, surfacesOffset)
	ViewAddFramebuffers(builder, framebuffersOffset)
	builder.Finish(ViewEnd(builder))
	return builder.Output()
