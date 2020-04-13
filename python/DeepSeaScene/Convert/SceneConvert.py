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
from ..FormatDecoration import *
from ..Scene import *
from ..TextureFormat import *

attachmentUsageEnum = {
	'Standard': 0,
	'Clear': 0x1,
	'KeepBefore': 0x2,
	'KeepAfter': 0x4,
	'UseLater': 0x8
}

pipelineStageEnum = {
	'CommandBuffer': 0x1,
	'DrawIndirect': 0x2,
	'VertexInput': 0x4,
	'VertexShader': 0x8,
	'TessellationControlShader': 0x10,
	'TessellationEvaluationShader': 0x20,
	'GeometryShader': 0x40,
	'FragmentShader': 0x80,
	'PreFragmentShaderTests': 0x100,
	'PostFragmentShaderTests': 0x200,
	'ColorOutput': 0x400,
	'ComputeShader': 0x800,
	'Copy': 0x1000,
	'HostAccess': 0x2000,
	'AllGraphics': 0x4000,
	'AllCommands': 0x8000,
}

pipelineAccessEnum = {
	'None': 0x0,
	'IndirectCommandRead': 0x1,
	'IndexRead': 0x2,
	'VertexAttributeRead': 0x4,
	'UniformBlockRead': 0x8,
	'UniformBufferRead': 0x10,
	'UniformBufferWrite': 0x20,
	'TextureRead': 0x40,
	'ImageRead': 0x80,
	'ImageWrite': 0x100,
	'InputAttachmentRead': 0x200,
	'ColorAttachmentRead': 0x400,
	'ColorAttachmentWrite': 0x800,
	'DepthStencilAttachmentRead': 0x1000,
	'DepthStencilAttachmentWrite': 0x2000,
	'CopyRead': 0x4000,
	'CopyWrite': 0x8000,
	'HostRead': 0x10000,
	'HostWrite': 0x20000,
	'MemoryRead': 0x40000,
	'MemoryWrite': 0x80000
}

def convertScene(convertContext, data):
	"""
	Converts a Scene. The data is expected to contain the following elements:
	- sharedItems: an optional array of shared item lists. Each element of the array is itself an
	  array with the following members:
	  - type: the name of the item list type.
	  - name: the name of the item list.
	  - data: the data for the item list. What this member contains (e.g. a string or a dict with
	    other members) depends on the instance data type.
	- pipeline: array of stages to define the pipeline to process when drawing the scene. Each
	  element of the array has one of the the following set members.
	  (For an item list):
	  - type: the name of the item list type.
	  - name: the name of the item list.
	  - data: the data for the item list. What this member contains (e.g. a string or a dict with
	    other members) depends on the instance data type.
	  (For a render pass):
	  - name: the name of the render pass.
	  - framebuffer: the name of the framebuffer to use when rendering.
	  - attachments: array of attachments to use during the render pass. Each element of the array
	    has the following members:
	    - usage: array of usage flags. See the dsAttachmentUsage enum for values, removing the type
	      prefix. Defaults to ['Standard'].
	    - format: the attachment format. See the dsGfxFormat enum for values, removing the type
		  prefix. The decorator values may not be used.
	    - decoration: the decoration for the format. See the dsGfxFormat enum for values, removing
	      the type prefix. Only the decorator values may be used.
	    - samples: the number of anti-alias samples. When ommitted, this uses the number of samples
	      set on the renderer for window surfaces.
	    - clearValue: a dict with one of the following members:
	      - floatValues: array of 4 float values.
	      - intValues: array of 4 signed int values.
	      - uintValues: array of 4 unsigned int values.
	      - depth: float depth value. This may be paired with "stencil".
	      - stencil: unsigned int stencil value. This may be paired with "depth".
	    - subpasses: array of subpasses in the render pass. Each element of the array has the
	      following members:
	      - name: the name of the subpass.
	      - inputAttachments: array of indices into the attachment arrays for the attachments to use
	        as subpass inputs.
	      - colorAttachments: array of attachments to write to for color. Each element of the array
	        has the following members:
	        - index: the index into the attachment array.
	        - resolve: whether or not to resolve multisampled results. Defaults to true.
	      - depthStencilAttachment: if set, the attachment to write to for depth/stencil. Each
	        element has the following members:
	        - index: the index into the attachment array.
	        - resolve: whether or not to resolve multisampled results. Defaults to true.
	      - drawLists: array of item lists to draw within the subpass. Each element of the array has
	        the following members:
	        - type: the name of the item list type.
	        - name: the name of the item list.
	        - data: the data for the item list. What this member contains (e.g. a string or a dict
	          with other members) depends on the instance data type.
	    - dependencies: optionall array of dependencies between subpasses. If ommitted, default
	      dependencies will be used, which should be sufficient for all but very specialized use
	      cases. Each element of the array has the following members:
	      - srcSubpass: the index to the source subpass. If ommitted, the dependency will be before
	        the render pass.
	      - srcStages: array of stages for the source dependency. See the dsGfxPipelineStage enum
	        for values, removing the type prefix.
	      - srcAccess: array of access types for the source dependency. See the dsGfxAccess enum for
	        values, removing the type prefix.
	      - dstSubpass: the index to the destination subpass. If ommitted, the dependency will be
	        after the render pass.
	      - dstStages: array of stages for the destination dependency. See the dsGfxPipelineStage
	        enum for values, removing the type prefix.
	      - dstAccess: array of access types for the destination dependency. See the dsGfxAccess
	        enum for values, removing the type prefix.
	      - regionDependency: true to have the dependency can be applied separately for different
	        regions of the attachment, false if it must be applied for the entire subpass.
	- globalData: optional array of items to store data globally in the scene. Each element of the
	  array has the following members:
	  - type: the name of the global data type.
	  - data: the data for the global data. What this member contains (e.g. a string or a dict with
	    other members) depends on the instance data type.
	"""
	unsetValue = 0xFFFFFFFF

	def readFloat(value, name):
		try:
			return float(value)
		except:
			raise Exception('Invalid ' + name + ' float value "' + str(value) + '".')

	def readInt(value, name):
		try:
			return int(value)
		except:
			raise Exception('Invalid ' + name + ' int value "' + str(value) + '".')

	def readUInt(value, name, defaultValue = None):
		if defaultValue is not None and value is None:
			return defaultValue

		try:
			intVal = int(value)
			if intVal < 0:
				raise Exception()
			return intVal
		except:
			raise Exception('Invalid ' + name + ' unsigned int value "' + str(value) + '".')

	def readBool(value, name):
		try:
			return bool(value)
		except:
			raise Exception('Invalid ' + name + ' bool value "' + str(value) + '".')

	def readRenderPass(info, item):
		item.name = str(info['name'])
		item.framebuffer = str(info['framebuffer'])
		attachmentInfos = info.get('attachments', [])

		item.attachments = []
		try:
			for attachmentInfo in attachmentInfos:
				attachment = object()
				attachment.usage = 0
				hasClear = False
				try:
					for usageEnum in attachmentInfo.get('usage', []):
						try:
							attachment.usage |= attachmentUsageEnum[usageEnum]
							if usageEnum == 'Clear':
								hasClear = True
						except:
							raise Exception('Invalid attachment usage value "' +
								str(usageEnum) + '".')
				except (TypeError, ValueError):
					raise Exception('Attachment usage must be an array of valid '
						'dsAttachmentUsage values.')

				formatStr = attachmentInfo['format']
				try:
					attachment.format = getattr(TextureFormat, formatStr)
				except AttributeError:
					raise Exception('Invalid attachment format "' + formatStr + '".')

				decorationStr = attachmentInfo['decoration']
				try:
					attachment.decoration = getattr(FormatDecoration, decorationStr)
				except AttributeError:
					raise Exception(
						'Invalid attachment format decoration "' + decorationStr + '".')

				attachment.samples = readUInt(attachmentInfo.get('samples'),
					'attachment samples', unsetValue)

				clearValueInfo = attachmentInfo.get('clearValue')
				if clearValueInfo:
					try:
						if 'floatValues' in clearValueInfo:
							floatValues = clearValueInfo['floatValues']
							try:
								if len(floatValues) != 4:
									raise Exception()
							except:
								raise Exception('Attachment clear values "floatValues" '
									'must be an array of 4 floats.')

							attachment.clearValueFloat = []
							for f in floatValues:
								attachment.clearValueFloat.append(readFloat(f,
									'clear value'))
						elif 'intValues' in clearValueInfo:
							intValues = clearValueInfo['intValues']
							try:
								if len(intValues) != 4:
									raise Exception()
							except:
								raise Exception('Attachment clear values "intValues" '
									'must be an array of 4 ints.')

							attachment.clearValueInt = []
							for i in intValues:
								attachment.clearValueInt.append(readInt(i,
									'clear value'))
						elif 'uintValues' in clearValueInfo:
							uintValues = clearValueInfo['uintValues']
							try:
								if len(uintValues) != 4:
									raise Exception()
							except:
								raise Exception('Attachment clear values "uintValues" '
									'must be an array of 4 unsigned ints.')

							attachment.clearValueUInt = []
							for i in intValues:
								attachment.clearValueUInt.append(readUInt(i,
									'clear value'))
						elif 'depth' in clearValueInfo or 'stencil' in clearValueInfo:
							attachment.clearValueDepthStencil = (
								readFloat(clearValueInfo.get('depth', 0.0),
									'clear depth'),
								readUInt(clearValueInfo.get('stencil', 0),
									'clear stencil')
							)
						else:
							raise Exception('Attachment clear values has no value.')
					except (AttributeError, TypeError, ValueError):
						raise Exception('Attachment clear values must be an object.')
		except KeyError as e:
			raise Exception('Attachments doesn\'t contain element "' + str(e) + '".')
		except (TypeError, ValueError):
			raise Exception('Attachments must be an array of objects.')

		subpassInfos = pipelineInfo['subpasses']
		item.subpasses = []
		try:
			for info in subpassInfos:
				subpass = object()
				subpass.name = str(info['name'])

				inputAttachmentInfos = info.get('inputAttachments', [])
				subpass.inputAttachments = []
				try:
					for inputAttachment in inputAttachmentInfos:
						index = readUInt(inputAttachment, 'input attachment')
						if index >= len(item.attachments):
							raise Exception(
								'Input attachment "' + str(index) + '" is out of range.')
						subpass.inputAttachments.append(index)
				except (TypeError, ValueError):
					raise Exception('Input attachments must be an array of indices.')

				colorAttachmentRefs = info.get('colorAttachments', [])
				subpass.colorAttachments = []
				try:
					for colorRef in colorAttachmentRefs:
						index = readUInt(colorRef['index'], 'color attachment')
						if index >= len(item.attachments):
							raise Exception(
								'Color attachment "' + str(index) + '" is out of range.')
						subpass.colorAttachments.append((index,
							readBool(colorRef.get('resolve', True), 'color attachment resolve')))
				except KeyError as e:
					raise Exception('Color attachment doesn\'t contain element "' + str(e) + '".')
				except (TypeError, ValueError):
					raise Exception('Color attachments must be an array of objects.')

				depthStencilRef = info.get('depthStencilAttachment')
				if depthStencilRef:
					try:
						index = readUInt(depthStencilRef['index'], 'depth/stencil attachment')
						if index >= len(item.attachments):
							raise Exception(
								'Depth/stencil attachment "' + str(index) + '" is out of range.')
						subpass.depthStencilAttachment = (index,
							readBool(depthStencilRef.get('resolve', True),
								'depth/stencil attachment resolve'))
					except KeyError as e:
						raise Exception(
							'Depth/stencil attachment doesn\'t contain element "' + str(e) + '".')
					except (TypeError, ValueError):
						raise Exception('Depth/stencil attachments must be an array of objects.')
				else:
					subpass.depthStencilAttachment = None

				drawListInfos = info['drawLists']
				subpass.drawLists = []
				try:
					for listInfo in drawListInfos:
						drawList = object()
						drawList.type = str(info['type'])
						drawList.name = str(info['name'])
						# Some item lists don't have data.
						drawList.data = info.get('data')
						subpass.drawLists.append(drawList)
				except KeyError as e:
					raise Exception('Draw list doesn\'t contain element "' + str(e) + '".')
				except (TypeError, ValueError):
					raise Exception('Draw lists must be an array of objects.')

				if not subpass.drawLists:
					raise Exception('Render subpass contains no draw lists.')

				item.subpasses.append(subpass)
		except KeyError as e:
			raise Exception('Subpasses doesn\'t contain element "' + str(e) + '".')
		except (TypeError, ValueError):
			raise Exception('Subpasses must be an array of objects.')

		if not item.subpasses:
			raise Exception('Render pass contains no subpasses.')

		dependencyInfos = info.get('dependencies')
		if dependencyInfos:
			item.dependencies = []
			try:
				for dependencyInfo in dependencyInfos:
					dependency = object()

					dependency.srcSubpass = readUInt(dependencyInfo.get('srcSubpass'),
						'source subpass', unsetValue)
					if dependency.srcSubpass != unsetValue and \
							dependency.srcSubpass >= len(item.subpasses):
						raise Exception('Source subpass "' + str(dependency.srcSubpass) +
							'" is out of range.')

					dependency.srcStages = 0
					try:
						for stageEnum in attachmentInfo.get('srcStages', []):
							try:
								dependency.srcStages |= pipelineStageEnum[stageEnum]
							except:
								raise Exception(
									'Invalid source stage value "' + str(stageEnum) + '".')
					except (TypeError, ValueError):
						raise Exception(
							'Source stages must be an array of valid dsGfxPipelineStage values.')
					if not dependency.srcStages:
						raise Exception('Dependency has no source stages.')

					dependency.srcAccess = 0
					try:
						for accessEnum in attachmentInfo.get('srcAccess', []):
							try:
								dependency.srcAccess |= pipelineAccessEnum[accessEnum]
							except:
								raise Exception(
									'Invalid source access value "' + str(accessEnum) + '".')
					except (TypeError, ValueError):
						raise Exception(
							'Source access must be an array of valid dsGfxAccess values.')
					if not dependency.srcStages:
						raise Exception('Dependency has no source stages.')

					dependency.dstStages = 0
					try:
						for stageEnum in attachmentInfo.get('dstStages', []):
							try:
								dependency.dstStages |= pipelineStageEnum[stageEnum]
							except:
								raise Exception(
									'Invalid destination stage value "' + str(stageEnum) + '".')
					except (TypeError, ValueError):
						raise Exception('Destination stages must be an array of valid '
							'dsGfxPipelineStage values.')
					if not dependency.dstStages:
						raise Exception('Dependency has no destination stages.')

					dependency.dstAccess = 0
					try:
						for accessEnum in attachmentInfo.get('dstAccess', []):
							try:
								dependency.dstAccess |= pipelineAccessEnum[accessEnum]
							except:
								raise Exception(
									'Invalid destination access value "' + str(accessEnum) + '".')
					except (TypeError, ValueError):
						raise Exception(
							'Destination access must be an array of valid dsGfxAccess values.')
					if not dependency.dstStages:
						raise Exception('Dependency has no destination stages.')

					dependency.regionDependency = readBool(attachmentInfo['regionDependency'],
						'region dependency')

					item.dependencies.append(dependency)
			except KeyError as e:
				raise Exception('Dependencies doesn\'t contain element "' + str(e) + '".')
			except (AttributeError, TypeError, ValueError):
				raise Exception('Dependencies must be an array of objects.')
		else:
			item.dependencies = None

	try:
		sharedItemInfo = data.get('sharedItems', [])
		sharedItems = []
		try:
			for infoArray in sharedItemInfo:
				itemArray = []
				for info in infoArray:
					sharedItem = object()
					sharedItem.type = str(info['type'])
					sharedItem.name = str(info['name'])
					# Some item lists don't have data.
					sharedItem.data = info.get('data')
					itemArray.append(sharedItem)
				sharedItems.append(itemArray)
		except KeyError as e:
			raise Exception('Scene "sharedItems" doesn\'t contain element "' + str(e) + '".')
		except (TypeError, ValueError):
			raise Exception('Scene "sharedItems" must be an array of array of objects.')

		pipelineInfo = data['pipeline']
		pipeline = []
		try:
			for info in pipelineInfo:
				item = object()
				if 'type' in info:
					item.type = str(info['type'])
					item.name = str(info['name'])
					# Some item lists don't have data.
					item.data = info.get('data')
				else:
					readRenderPass(info, item)
				pipeline.append(item)
		except KeyError as e:
			raise Exception('Scene "pipeline" doesn\'t contain element "' + str(e) + '".')
		except (TypeError, ValueError):
			raise Exception('Scene "pipeline" must be an array of objects.')

		globalDataInfo = data.get('globalData', [])
		globalData = []
		try:
			for info in globalDataInfo:
				item = object()
				item.type = str(info['type'])
				# Some item lists don't have data.
				item.data = info.get('data')
				globalData.append(globalData)
		except KeyError as e:
			raise Exception('Scene "globalData" doesn\'t contain element "' + str(e) + '".')
		except (TypeError, ValueError):
			raise Exception('Scene "globalData" must be an array of array of objects.')

	except KeyError as e:
		raise Exception('Scene doesn\'t contain element "' + str(e) + '".')
	except (AttributeError, TypeError, ValueError):
		raise Exception('Scene must be an object.')
