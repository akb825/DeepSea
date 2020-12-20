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
from ..Attachment import *
from ..AttachmentRef import *
from ..ClearValue import *
from ..ClearColorFloat import *
from ..ClearColorInt import *
from ..ClearColorUInt import *
from ..ClearDepthStencil import *
from ..FormatDecoration import *
from ..RenderPass import *
from ..RenderSubpass import *
from ..Scene import *
from ..SceneItemLists import *
from ..ScenePipelineItem import *
from ..ScenePipelineItemUnion import *
from ..SubpassDependency import *
from ..TextureFormat import *

class Object:
	pass

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
	  - Remaining members depend on the value of type.
	- pipeline: array of stages to define the pipeline to process when drawing the scene. Each
	  element of the array has one of the the following set members.
	  (For an item list):
	  - type: the name of the item list type.
	  - name: the name of the item list.
	  - Remaining members depend on the value of type.
	  (For a render pass):
	  - framebuffer: the name of the framebuffer to use when rendering.
	  - attachments: array of attachments to use during the render pass. Each element of the array
	    has the following members:
	    - usage: array of usage flags. See the dsAttachmentUsage enum for values, removing the type
	      prefix. Defaults to ["Standard"].
	    - format: the attachment format. See the dsGfxFormat enum for values, removing the type
		  prefix. The decorator values may not be used. May also be "SurfaceColor" or
	      "SurfaceDepthStencil" to use the color or depth/stencil format for render surfaces.
	    - decoration: the decoration for the format. See the dsGfxFormat enum for values, removing
	      the type prefix. Only the decorator values may be used. May also be "Unset" in cases where
	      a decorator isn't valid.
	    - samples: the number of anti-alias samples. When omitted, this uses the number of samples
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
	      - resolve: whether or not to resolve multisampled results.
	    - depthStencilAttachment: if set, the attachment to write to for depth/stencil. Each
	      element has the following members:
	      - index: the index into the attachment array.
	      - resolve: whether or not to resolve multisampled results.
	    - drawLists: array of item lists to draw within the subpass. Each element of the array has
	      the following members:
	      - type: the name of the item list type.
	      - name: the name of the item list.
	      - Remaining members depend on the value of type.
	  - dependencies: optionall array of dependencies between subpasses. If omitted, default
	    dependencies will be used, which should be sufficient for all but very specialized use
	    cases. Each element of the array has the following members:
	    - srcSubpass: the index to the source subpass. If omitted, the dependency will be before
	      the render pass.
	    - srcStages: array of stages for the source dependency. See the dsGfxPipelineStage enum for
	      values, removing the type prefix.
	    - srcAccess: array of access types for the source dependency. See the dsGfxAccess enum for
	      values, removing the type prefix.
	    - dstSubpass: the index to the destination subpass. If omitted, the dependency will be
	      after the render pass.
	    - dstStages: array of stages for the destination dependency. See the dsGfxPipelineStage enum
	      for values, removing the type prefix.
	    - dstAccess: array of access types for the destination dependency. See the dsGfxAccess enum
	      for values, removing the type prefix.
	    - regionDependency: true to have the dependency can be applied separately for different
	      regions of the attachment, false if it must be applied for the entire subpass.
	- globalData: optional array of items to store data globally in the scene. Each element of the
	  array has the following members:
	  - type: the name of the global data type.
	  - Remaining members depend on the value of type.
	- nodes: array of string node names to set on the scene.
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

	def readAttachment(info):
		attachment = Object()
		attachment.usage = 0
		hasClear = False
		try:
			for usageEnum in info.get('usage', []):
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

		formatStr = info['format']
		try:
			attachment.format = getattr(TextureFormat, formatStr)
		except AttributeError:
			raise Exception('Invalid attachment format "' + formatStr + '".')

		decorationStr = info['decoration']
		try:
			attachment.decoration = getattr(FormatDecoration, decorationStr)
		except AttributeError:
			raise Exception(
				'Invalid attachment format decoration "' + decorationStr + '".')

		attachment.samples = readUInt(info.get('samples'), 'attachment samples', unsetValue)

		clearValueInfo = info.get('clearValue')
		if clearValueInfo:
			try:
				if 'floatValues' in clearValueInfo:
					floatValues = clearValueInfo['floatValues']
					try:
						if len(floatValues) != 4:
							raise Exception()
					except:
						raise Exception(
							'Attachment clear values "floatValues" must be an array of 4 floats.')

					attachment.clearColorFloat = []
					for f in floatValues:
						attachment.clearColorFloat.append(readFloat(f, 'clear value'))
				elif 'intValues' in clearValueInfo:
					intValues = clearValueInfo['intValues']
					try:
						if len(intValues) != 4:
							raise Exception()
					except:
						raise Exception(
							'Attachment clear values "intValues" must be an array of 4 ints.')

					attachment.clearColorInt = []
					for i in intValues:
						attachment.clearColorInt.append(readInt(i, 'clear value'))
				elif 'uintValues' in clearValueInfo:
					uintValues = clearValueInfo['uintValues']
					try:
						if len(uintValues) != 4:
							raise Exception()
					except:
						raise Exception('Attachment clear values "uintValues" must be an array of '
							'4 unsigned ints.')

					attachment.clearColorUInt = []
					for i in intValues:
						attachment.clearColorUInt.append(readUInt(i, 'clear value'))
				elif 'depth' in clearValueInfo or 'stencil' in clearValueInfo:
					attachment.clearDepthStencil = (
						readFloat(clearValueInfo.get('depth', 0.0), 'clear depth'),
						readUInt(clearValueInfo.get('stencil', 0), 'clear stencil')
					)
				else:
					raise Exception('Attachment clear values has no value.')
			except (AttributeError, TypeError, ValueError):
				raise Exception('Attachment clear values must be an object.')
		elif hasClear:
			raise Exception('Attachment is set to clear but has no clear value.')

		return attachment

	def readSubpass(info, renderPass):
		subpass = Object()
		subpass.name = str(info['name'])

		inputAttachmentInfos = info.get('inputAttachments', [])
		subpass.inputAttachments = []
		try:
			for inputAttachment in inputAttachmentInfos:
				index = readUInt(inputAttachment, 'input attachment')
				if index >= len(renderPass.attachments):
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
				if index >= len(renderPass.attachments):
					raise Exception(
						'Color attachment "' + str(index) + '" is out of range.')
				subpass.colorAttachments.append((index,
					readBool(colorRef['resolve'], 'color attachment resolve')))
		except KeyError as e:
			raise Exception('Color attachment doesn\'t contain element ' + str(e) + '.')
		except (TypeError, ValueError):
			raise Exception('Color attachments must be an array of objects.')

		depthStencilRef = info.get('depthStencilAttachment')
		if depthStencilRef:
			try:
				index = readUInt(depthStencilRef['index'], 'depth/stencil attachment')
				if index >= len(renderPass.attachments):
					raise Exception(
						'Depth/stencil attachment "' + str(index) + '" is out of range.')
				subpass.depthStencilAttachment = (index,
					readBool(depthStencilRef['resolve'], 'depth/stencil attachment resolve'))
			except KeyError as e:
				raise Exception(
					'Depth/stencil attachment doesn\'t contain element ' + str(e) + '.')
			except (TypeError, ValueError):
				raise Exception('Depth/stencil attachments must be an array of objects.')
		else:
			subpass.depthStencilAttachment = None

		drawListInfos = info['drawLists']
		subpass.drawLists = []
		try:
			for listInfo in drawListInfos:
				drawList = Object()
				drawList.type = str(listInfo['type'])
				drawList.name = str(listInfo['name'])
				drawList.data = listInfo
				subpass.drawLists.append(drawList)
		except KeyError as e:
			raise Exception('Draw list doesn\'t contain element ' + str(e) + '.')
		except (TypeError, ValueError):
			raise Exception('Draw lists must be an array of objects.')

		if not subpass.drawLists:
			raise Exception('Render subpass contains no draw lists.')

		return subpass

	def readDependency(info, renderPass):
		dependency = Object()
		dependency.srcSubpass = readUInt(info.get('srcSubpass'),
			'source subpass', unsetValue)
		if dependency.srcSubpass != unsetValue and \
				dependency.srcSubpass >= len(renderPass.subpasses):
			raise Exception('Source subpass "' + str(dependency.srcSubpass) + '" is out of range.')

		dependency.srcStages = 0
		try:
			for stageEnum in info.get('srcStages', []):
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
			for accessEnum in info.get('srcAccess', []):
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
			for stageEnum in info.get('dstStages', []):
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
			for accessEnum in info.get('dstAccess', []):
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

		dependency.regionDependency = readBool(info['regionDependency'],
			'region dependency')
		return dependency

	def readRenderPass(info, item):
		item.framebuffer = str(info['framebuffer'])
		attachmentInfos = info.get('attachments', [])

		item.attachments = []
		try:
			for attachmentInfo in attachmentInfos:
				item.attachments.append(readAttachment(attachmentInfo))
		except KeyError as e:
			raise Exception('Attachments doesn\'t contain element ' + str(e) + '.')
		except (TypeError, ValueError):
			raise Exception('Attachments must be an array of objects.')

		subpassInfos = info['subpasses']
		item.subpasses = []
		try:
			for subpassInfo in subpassInfos:
				item.subpasses.append(readSubpass(subpassInfo, item))
		except KeyError as e:
			raise Exception('Subpasses doesn\'t contain element ' + str(e) + '.')
		except (TypeError, ValueError):
			raise Exception('Subpasses must be an array of objects.')

		if not item.subpasses:
			raise Exception('Render pass contains no subpasses.')

		dependencyInfos = info.get('dependencies')
		if dependencyInfos:
			item.dependencies = []
			try:
				for dependencyInfo in dependencyInfos:
					item.dependencies.append(readDependency(dependencyInfo, item))
			except KeyError as e:
				raise Exception('Dependencies doesn\'t contain element ' + str(e) + '.')
			except (AttributeError, TypeError, ValueError):
				raise Exception('Dependencies must be an array of objects.')
		else:
			item.dependencies = None

		return

	try:
		sharedItemInfo = data.get('sharedItems', [])
		sharedItems = []
		try:
			for infoArray in sharedItemInfo:
				itemArray = []
				for info in infoArray:
					sharedItem = Object()
					sharedItem.type = str(info['type'])
					sharedItem.name = str(info['name'])
					# Some item lists don't have data.
					sharedItem.data = info
					itemArray.append(sharedItem)
				sharedItems.append(itemArray)
		except KeyError as e:
			raise Exception('Scene "sharedItems" doesn\'t contain element ' + str(e) + '.')
		except (TypeError, ValueError):
			raise Exception('Scene "sharedItems" must be an array of array of objects.')

		pipelineInfo = data['pipeline']
		pipeline = []
		try:
			for info in pipelineInfo:
				item = Object()
				if 'type' in info:
					item.type = str(info['type'])
					item.name = str(info['name'])
					# Some item lists don't have data.
					item.data = info
				else:
					readRenderPass(info, item)
				pipeline.append(item)
		except KeyError as e:
			raise Exception('Scene "pipeline" doesn\'t contain element ' + str(e) + '.')
		except (TypeError, ValueError):
			raise Exception('Scene "pipeline" must be an array of objects.')

		if not pipeline:
			raise Exception('Scene pipeline is empty.')

		globalDataInfo = data.get('globalData', [])
		globalData = []
		try:
			for info in globalDataInfo:
				item = Object()
				item.type = str(info['type'])
				# Some item lists don't have data.
				item.data = info
				globalData.append(item)
		except KeyError as e:
			raise Exception('Scene "globalData" doesn\'t contain element ' + str(e) + '.')
		except (TypeError, ValueError):
			raise Exception('Scene "globalData" must be an array of objects.')

		nodeInfos = data.get('nodes', [])
		nodes = []
		try:
			for node in nodeInfos:
				nodes.append(str(node))
		except (TypeError, ValueError):
			raise Exception('Scene "nodes" must be an array of strings.')

	except KeyError as e:
		raise Exception('Scene doesn\'t contain element ' + str(e) + '.')
	except (AttributeError, TypeError, ValueError):
		raise Exception('Scene must be an object.')

	builder = flatbuffers.Builder(0)

	sharedItemsOffsets = []
	for itemLists in sharedItems:
		itemListOffsets = []
		for item in itemLists:
			itemListOffsets.append(convertContext.convertItemList(builder, item.type, item.name,
				item.data))

		SceneItemListsStartItemListsVector(builder, len(itemListOffsets))
		for offset in reversed(itemListOffsets):
			builder.PrependUOffsetTRelative(offset)
		itemListOffset = builder.EndVector(len(itemListOffsets))

		SceneItemListsStart(builder)
		SceneItemListsAddItemLists(builder, itemListOffset)
		sharedItemsOffsets.append(SceneItemListsEnd(builder))

	if sharedItemsOffsets:
		SceneStartSharedItemsVector(builder, len(sharedItemsOffsets))
		for offset in reversed(sharedItemsOffsets):
			builder.PrependUOffsetTRelative(offset)
		sharedItemsOffset = builder.EndVector(len(sharedItemsOffsets))
	else:
		sharedItemsOffset = 0

	pipelineOffsets = []
	for item in pipeline:
		if hasattr(item, 'type'):
			itemType = ScenePipelineItemUnion.SceneItemList
			itemOffset = convertContext.convertSharedItemList(builder, item.type, item.name,
				item.data)
		else:
			itemType = ScenePipelineItemUnion.RenderPass
			framebufferOffset = builder.CreateString(item.framebuffer)

			attachmentOffsets = []
			for attachment in item.attachments:
				if hasattr(attachment, 'clearColorFloat'):
					clearValueType = ClearValue.ClearColorFloat
					ClearColorFloatStart(builder)
					ClearColorFloatAddRed(builder, attachment.clearColorFloat[0])
					ClearColorFloatAddGreen(builder, attachment.clearColorFloat[1])
					ClearColorFloatAddBlue(builder, attachment.clearColorFloat[2])
					ClearColorFloatAddAlpha(builder, attachment.clearColorFloat[3])
					clearValueOffset = ClearColorFloatEnd(builder)
				elif hasattr(attachment, 'clearColorInt'):
					clearValueType = ClearValue.ClearColorInt
					ClearColorIntStart(builder)
					ClearColorIntAddRed(builder, attachment.clearColorInt[0])
					ClearColorIntAddGreen(builder, attachment.clearColorInt[1])
					ClearColorIntAddBlue(builder, attachment.clearColorInt[2])
					ClearColorIntAddAlpha(builder, attachment.clearColorInt[3])
					clearValueOffset = ClearColorIntEnd(builder)
				elif hasattr(attachment, 'clearColorUInt'):
					clearValueType = ClearValue.ClearColorUInt
					ClearColorUIntStart(builder)
					ClearColorUIntAddRed(builder, attachment.clearColorUInt[0])
					ClearColorUIntAddGreen(builder, attachment.clearColorUInt[1])
					ClearColorUIntAddBlue(builder, attachment.clearColorUInt[2])
					ClearColorUIntAddAlpha(builder, attachment.clearColorUInt[3])
					clearValueOffset = ClearColorUIntEnd(builder)
				elif hasattr(attachment, 'clearDepthStencil'):
					clearValueType = ClearValue.ClearDepthStencil
					ClearDepthStencilStart(builder)
					ClearDepthStencilAddDepth(builder, attachment.clearDepthStencil[0])
					ClearDepthStencilAddStencil(builder, attachment.clearDepthStencil[1])
					clearValueOffset = ClearDepthStencilEnd(builder)
				else:
					clearValueType = ClearValue.NONE
					clearValueOffset = 0

				AttachmentStart(builder)
				AttachmentAddUsage(builder, attachment.usage)
				AttachmentAddFormat(builder, attachment.format)
				AttachmentAddDecoration(builder, attachment.decoration)
				AttachmentAddSamples(builder, attachment.samples)
				AttachmentAddClearValueType(builder, clearValueType)
				AttachmentAddClearValue(builder, clearValueOffset)
				attachmentOffsets.append(AttachmentEnd(builder))

			if attachmentOffsets:
				RenderPassStartAttachmentsVector(builder, len(attachmentOffsets))
				for offset in reversed(attachmentOffsets):
					builder.PrependUOffsetTRelative(offset)
				attachmentsOffset = builder.EndVector(len(attachmentOffsets))
			else:
				attachmentsOffset = 0

			subpassOffsets = []
			for subpass in item.subpasses:
				subpassNameOffset = builder.CreateString(subpass.name)

				if subpass.inputAttachments:
					RenderSubpassStartInputAttachmentsVector(builder, len(subpass.inputAttachments))
					for attachment in reversed(subpass.inputAttachments):
						builder.PrependUint32(attachment)
					inputAttachmentsOffset = builder.EndVector(len(subpass.inputAttachments))
				else:
					inputAttachmentsOffset = 0

				if subpass.colorAttachments:
					RenderSubpassStartColorAttachmentsVector(builder, len(subpass.colorAttachments))
					for attachment in reversed(subpass.colorAttachments):
						CreateAttachmentRef(builder, *attachment)
					colorAttachmentsOffset = builder.EndVector(len(subpass.colorAttachments))
				else:
					colorAttachmentsOffset = 0

				drawListOffsets = []
				for drawList in subpass.drawLists:
					drawListOffsets.append(convertContext.convertItemList(builder, drawList.type,
						drawList.name, drawList.data))

				RenderSubpassStartDrawListsVector(builder, len(drawListOffsets))
				for offset in reversed(drawListOffsets):
					builder.PrependUOffsetTRelative(offset)
				drawListsOffset = builder.EndVector(len(drawListOffsets))

				RenderSubpassStart(builder)
				RenderSubpassAddName(builder, subpassNameOffset)
				RenderSubpassAddInputAttachments(builder, inputAttachmentsOffset)
				RenderSubpassAddColorAttachments(builder, colorAttachmentsOffset)

				if subpass.depthStencilAttachment:
					depthStencilAttachmentOffset = CreateAttachmentRef(builder,
						*subpass.depthStencilAttachment)
				else:
					depthStencilAttachmentOffset = 0
				RenderSubpassAddDepthStencilAttachment(builder, depthStencilAttachmentOffset)

				RenderSubpassAddDrawLists(builder, drawListsOffset)
				subpassOffsets.append(RenderSubpassEnd(builder))

			RenderPassStartSubpassesVector(builder, len(subpassOffsets))
			for offset in reversed(subpassOffsets):
				builder.PrependUOffsetTRelative(offset)
			subpassesOffset = builder.EndVector(len(subpassOffsets))

			if item.dependencies:
				RenderPassStartDependenciesVector(builder, len(item.dependencies))
				for dependency in reversed(item.dependencies):
					CreateSubpassDependency(builder, dependency.srcSubpass, dependency.srcStages,
						dependency.srcAccess, dependency.dstSubpass, dependency.dstStages,
						dependency.dstAccess, dependency.regionDependency)
				dependenciesOffset = builder.EndVector(len(item.dependencies))
			else:
				dependenciesOffset = 0

			RenderPassStart(builder)
			RenderPassAddFramebuffer(builder, framebufferOffset)
			RenderPassAddAttachments(builder, attachmentsOffset)
			RenderPassAddSubpasses(builder, subpassesOffset)
			RenderPassAddDependencies(builder, dependenciesOffset)
			itemOffset = RenderPassEnd(builder)

		ScenePipelineItemStart(builder)
		ScenePipelineItemAddItemType(builder, itemType)
		ScenePipelineItemAddItem(builder, itemOffset)
		pipelineOffsets.append(ScenePipelineItemEnd(builder))

	SceneStartPipelineVector(builder, len(pipelineOffsets))
	for offset in reversed(pipelineOffsets):
		builder.PrependUOffsetTRelative(offset)
	pipelineOffset = builder.EndVector(len(pipelineOffsets))

	globalDataOffsets = []
	for item in globalData:
		globalDataOffsets.append(convertContext.convertGlobalData(builder, item.type, item.data))

	if globalDataOffsets:
		SceneStartGlobalDataVector(builder, len(globalDataOffsets))
		for offset in reversed(globalDataOffsets):
			builder.PrependUOffsetTRelative(offset)
		globalDataOffset = builder.EndVector(len(globalDataOffsets))
	else:
		globalDataOffset = 0

	if nodes:
		nodeOffsets = []
		for node in nodes:
			nodeOffsets.append(builder.CreateString(node))

		SceneStartNodesVector(builder, len(nodeOffsets))
		for offset in reversed(nodeOffsets):
			builder.PrependUOffsetTRelative(offset)
		nodesOffset = builder.EndVector(len(nodeOffsets))
	else:
		nodesOffset = 0

	SceneStart(builder)
	SceneAddSharedItems(builder, sharedItemsOffset)
	SceneAddPipeline(builder, pipelineOffset)
	SceneAddGlobalData(builder, globalDataOffset)
	SceneAddNodes(builder, nodesOffset)
	builder.Finish(SceneEnd(builder))
	return builder.Output()
