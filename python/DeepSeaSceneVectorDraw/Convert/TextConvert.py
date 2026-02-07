# Copyright 2020-2026 Aaron Barany
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
import io
import sys
import xml.dom
from xml.dom import minidom

from ..Color import CreateColor
from .. import SceneText
from .. import SceneTextStyle
from .. import VectorResourceRef

from DeepSeaVectorDraw.Convert.SVG import readText, Materials
from DeepSeaVectorDraw.TextPosition import TextPosition

def createColor(colorMaterials, style, builder):
	color = colorMaterials.get(style.material)
	if not color:
		raise Exception('Only solid colors are allowed for text styles.')

	alpha = int(round(float(color[3])*style.opacity))
	if alpha < 0:
		alpha = 0
	elif alpha > 255:
		alpha = 255

	return CreateColor(builder, color[0], color[1], color[2], alpha)

def convertText(convertContext, data, inputDir, outputDir):
	"""
	Converts text for use in a scene. The data map is expected to contain the following elements:
	- resources: the name of the vector resources to get the font from.
	- font: the name of the font to use if not provided by the text XML element. Defaults to
	  'serif'.
	- text: the text. This can either be a path to a .xml file or embedded XML string. The XML
	  contents should be a single <text> SVG element with any number of <tspan> embedded elements.
	  (see https://www.w3.org/TR/SVG2/text.html#TextElement for details) Only solid colors are
	  allowed for stroke and fill. When a position is provided, only a relative offset for the
	  vertical position is supported.
	"""
	builder = flatbuffers.Builder(0)

	try:
		vectorResources = str(data['vectorResources'])
		defaultFont = str(data.get('font', 'serif'))
		textData = str(data['text']).strip()
	except KeyError as e:
		raise Exception('SceneText doesn\'t contain element ' + str(e) + '.')
	except (AttributeError, TypeError, ValueError):
		raise Exception('SceneText must be an object.')

	if textData.startswith('<'):
		if sys.version_info < (3, 0):
			textData = unicode(textData)
		textXml = minidom.parse(io.StringIO(textData))
	else:
		textXml = minidom.parse(textData)

	materials = Materials('text')
	font, text, ranges = readText(textXml.firstChild, defaultFont, [0.0, 0.0], 0.0, materials)
	if not text:
		raise Exception('Invalid SVG text data.')

	colorMaterials = dict()
	for i in range(0, len(materials.colors)):
		colorMaterials[materials.getColorName(i)] = materials.colors[i]

	vectorResourcesOffset = builder.CreateString(vectorResources)
	fontOffset = builder.CreateString(font.font)

	VectorResourceRef.Start(builder)
	VectorResourceRef.AddResources(builder, vectorResourcesOffset)
	VectorResourceRef.AddName(builder, fontOffset)
	vectorResourceRefOffset = VectorResourceRef.End(builder)

	textOffset = builder.CreateString(text)

	rangeOffsets = []
	for textRange in ranges:
		style = textRange.style

		SceneTextStyle.Start(builder)
		SceneTextStyle.AddStart(builder, textRange.start)
		SceneTextStyle.AddCount(builder, textRange.count)
		SceneTextStyle.AddSize(builder, style.font.size)
		SceneTextStyle.AddEmbolden(builder, style.font.embolden)
		SceneTextStyle.AddSlant(builder, style.font.slant)
		SceneTextStyle.AddOutlineWidth(builder, style.stroke.width if style.stroke else 0.0)
		SceneTextStyle.AddFuziness(builder, 1.0)
		SceneTextStyle.AddVerticalOffset(builder,
			textRange.position[1] if textRange.positionType == TextPosition.Offset else 0.0)

		if style.fill:
			colorOffset = createColor(colorMaterials, style.fill, builder)
		else:
			colorOffset = 0

		SceneTextStyle.AddColor(builder, colorOffset)

		if style.stroke:
			colorOffset = createColor(colorMaterials, style.stroke, builder)
		else:
			colorOffset = 0

		SceneTextStyle.AddOutlineColor(builder, colorOffset)
		rangeOffsets.append(SceneTextStyle.End(builder))

	SceneText.StartStylesVector(builder, len(rangeOffsets))
	for offset in reversed(rangeOffsets):
		builder.PrependUOffsetTRelative(offset)
	stylesOffset = builder.EndVector()

	SceneText.Start(builder)
	SceneText.AddFont(builder, vectorResourceRefOffset)
	SceneText.AddText(builder, textOffset)
	SceneText.AddStyles(builder, stylesOffset)
	builder.Finish(SceneText.End(builder))
	return builder.Output()
