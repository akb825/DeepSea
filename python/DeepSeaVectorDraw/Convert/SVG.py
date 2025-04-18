# Copyright 2020-2025 Aaron Barany
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

from __future__ import print_function
import math
import os
import re
import sys
import xml.dom
from xml.dom import minidom

import flatbuffers
from .Colors import colorFromString
from .. import ArcCommand
from .. import BezierCommand
from .. import ClosePathCommand
from ..Color import CreateColor
from .. import ColorMaterial
from ..DashArray import CreateDashArray
from .. import EllipseCommand
from .. import FillPathCommand
from ..FillRule import FillRule
from ..GradientEdge import GradientEdge
from ..GradientStop import CreateGradientStop
from .. import ImageCommand
from .. import LinearGradient
from ..LineCap import LineCap
from .. import LineCommand
from ..LineJoin import LineJoin
from ..MaterialSpace import MaterialSpace
from ..Matrix33f import CreateMatrix33f
from .. import MoveCommand
from .. import QuadraticCommand
from .. import RadialGradient
from .. import RectangleCommand
from .. import StartPathCommand
from .. import StrokePathCommand
from .. import TextCommand
from ..TextAlign import TextAlign
from ..TextPosition import TextPosition
from .. import TextRangeCommand
from ..Vector2f import CreateVector2f
from ..Vector3f import CreateVector3f
from .. import VectorCommand
from ..VectorCommandUnion import VectorCommandUnion
from .. import VectorImage

lineJoinMap = {'miter': LineJoin.Miter, 'bevel': LineJoin.Bevel, 'round': LineJoin.Round}
lineCapMap = {'butt': LineCap.Butt, 'round': LineCap.Round, 'square': LineCap.Square}
textAlignMap = {'start': TextAlign.Start, 'end': TextAlign.End, 'left': TextAlign.Left,
	'right': TextAlign.Right, 'center': TextAlign.Center}
textAnchorMap = {'start': TextAlign.Start, 'end': TextAlign.End, 'middle': TextAlign.Center}
textAttributes = ['font-family', 'font-size', 'font-style', 'font-weight', 'text-align',
	'text-anchor', 'line-height', 'textLength', 'inline-size']

def sizeFromString(sizeStr, relativeSize):
	"""
	Converts from a size string to a float size.
	sizeStr: The string representation of the size.
	relativeSize: The size to use in case of percentages.
	"""
	if not sizeStr:
		raise Exception("Size not specified")

	dpi = 96.0
	cm = 2.54
	if len(sizeStr) > 2 and sizeStr[-2:] == 'cm':
		return float(sizeStr[:-2])*dpi/cm
	elif len(sizeStr) > 2 and sizeStr[-2:] == 'mm':
		return float(sizeStr[:-2])*dpi/(cm*10.0)
	elif len(sizeStr) > 1 and sizeStr[-1:] == 'Q':
		return float(sizeStr[:-1])*dpi/(cm*40.0)
	elif len(sizeStr) > 2 and sizeStr[-2:] == 'in':
		return float(sizeStr[:-2])*dpi
	elif len(sizeStr) > 2 and sizeStr[-2:] == 'pc':
		return float(sizeStr[:-2])*dpi/6.0
	elif len(sizeStr) > 2 and sizeStr[-2:] == 'pt':
		return float(sizeStr[:-2])*dpi/72.0
	elif len(sizeStr) > 2 and sizeStr[-2:] == 'em':
		return float(sizeStr[:-2])*16.0
	elif len(sizeStr) > 2 and sizeStr[-2:] == 'px':
		return float(sizeStr[:-2])
	elif len(sizeStr) > 1 and sizeStr[-1:] == '%':
		return float(sizeStr[:-1])/100.0*relativeSize
	return float(sizeStr)

def angleFromString(angleStr):
	"""Converts from an angle string to a float angle in radians."""
	if len(angleStr) > 3 and angleStr[-3:] == 'deg':
		return math.radians(float(angleStr[:-3]))
	elif len(angleStr) > 4 and angleStr[-4:] == 'grad':
		return float(angleStr[:-4])*math.pi/200.0
	elif len(angleStr) > 3 and angleStr[-3:] == 'rad':
		return float(angleStr[:-3])
	elif len(angleStr) > 4 and angleStr[-4:] == 'turn':
		return float(angleStr[:-4])*2.0*math.pi
	return math.radians(float(angleStr))

def extractAttributes(node):
	attributes = {}
	for attrName, attrValue in node.attributes.items():
		if attrName == 'style':
			elementStrings = attrValue.split(';')
			for elementStr in elementStrings:
				if len(elementStr.strip()) == 0:
					continue;
				elementPair = elementStr.split(':')
				if len(elementPair) != 2:
					raise Exception('Invalid style strimg "' + attrValue + '"')
				attributes[elementPair[0].strip()] = elementPair[1].strip()
		else:
			attributes[attrName.strip()] = attrValue.strip()
	return attributes

class Transform:
	"""3x3 transform matrix for a transform."""
	def __init__(self, matrix = None):
		if matrix:
			self.matrix = matrix
		else:
			self.matrix = ((1.0, 0.0, 0.0), (0.0, 1.0, 0.0), (0.0, 0.0, 1.0))

	@staticmethod
	def fromNode(node, transformName = "transform"):
		"""Extracts the transform from a node as a 3x3 matrix tuple in column-major order."""
		if not node.hasAttribute(transformName):
			return Transform()

		try:
			# Support a single matrix or translate element.
			transformStr = node.getAttribute(transformName)
			if transformStr[:7] == 'matrix(':
				values = re.findall(r"[-+0-9.eE]+", transformStr[7:-1])
				if len(values) != 6:
					raise Exception()
				return Transform(((float(values[0].strip()), float(values[1].strip()), 0.0),
					(float(values[2].strip()), float(values[3].strip()), 0.0),
					(float(values[4].strip()), float(values[5].strip()), 1.0)))
			elif transformStr[:10] == 'translate(':
				values = re.findall(r"[-+0-9.eE]+", transformStr[10:-1])
				if len(values) != 2:
					raise Exception()
				return Transform(((1.0, 0.0, 0.0), (0.0, 1.0, 0.0),
					(float(values[0].strip()), float(values[1].strip()), 1.0)))
			else:
				raise Exception()
		except:
			raise Exception('Invalid transform value "' + transformStr + '"')

	def createMatrix33f(self, builder):
		return CreateMatrix33f(builder, self.matrix[0][0], self.matrix[0][1], self.matrix[0][2],
			self.matrix[1][0], self.matrix[1][1], self.matrix[1][2], self.matrix[2][0],
			self.matrix[2][1], self.matrix[2][2])

	def __mul__(self, other):
		return Transform(
			(
				(
					self.matrix[0][0]*other.matrix[0][0] + self.matrix[1][0]*other.matrix[0][1] +
						self.matrix[2][0]*other.matrix[0][2],
					self.matrix[0][1]*other.matrix[0][0] + self.matrix[1][1]*other.matrix[0][1] +
						self.matrix[2][1]*other.matrix[0][2],
					self.matrix[0][2]*other.matrix[0][0] + self.matrix[1][2]*other.matrix[0][1] +
						self.matrix[2][2]*other.matrix[0][2]
				),
				(
					self.matrix[0][0]*other.matrix[1][0] + self.matrix[1][0]*other.matrix[1][1] +
						self.matrix[2][0]*other.matrix[1][2],
					self.matrix[0][1]*other.matrix[1][0] + self.matrix[1][1]*other.matrix[1][1] +
						self.matrix[2][1]*other.matrix[1][2],
					self.matrix[0][2]*other.matrix[1][0] + self.matrix[1][2]*other.matrix[1][1] +
						self.matrix[2][2]*other.matrix[1][2]
				),
				(
					self.matrix[0][0]*other.matrix[2][0] + self.matrix[1][0]*other.matrix[2][1] +
						self.matrix[2][0]*other.matrix[2][2],
					self.matrix[0][1]*other.matrix[2][0] + self.matrix[1][1]*other.matrix[2][1] +
						self.matrix[2][1]*other.matrix[2][2],
					self.matrix[0][2]*other.matrix[2][0] + self.matrix[1][2]*other.matrix[2][1] +
						self.matrix[2][2]*other.matrix[2][2]
				)
			))

class Gradient:
	"""Base class for a gradient."""
	def __init__(self, node, materials):
		self.name = node.getAttribute('id')
		self.transform = Transform.fromNode(node, 'gradientTransform')
		if node.hasAttribute('gradientUnits'):
			units = node.getAttribute('gradientUnits')
			if units == 'userSpaceOnUse':
				self.coordinateSpace = MaterialSpace.Local
			else:
				self.coordinateSpace = MaterialSpace.Bounds
		else:
			self.coordinateSpace = MaterialSpace.Bounds
		if node.hasAttribute('spreadMethod'):
			spread = node.getAttribute('spreadMethod')
			if spread == 'reflect':
				self.edge = GradientEdge.Mirror
			elif spread == 'repeat':
				self.edge = GradientEdge.Repeat
			else:
				self.edge = GradientEdge.Clamp
		else:
			self.edge = GradientEdge.Clamp

		if node.hasAttribute('xlink:href'):
			# Strip leading # from reference.
			self.stops = materials.findGradientStops(node.getAttribute('xlink:href')[1:])
		else:
			self.stops = []
			for stop in node.childNodes:
				if stop.nodeType != xml.dom.Node.ELEMENT_NODE:
					continue

				elements = extractAttributes(stop)
				position = sizeFromString(elements['offset'], 1.0)
				color = colorFromString(elements['stop-color'])
				opacity = 1.0
				if 'stop-opacity' in elements:
					opacity = sizeFromString(elements['stop-opacity'], 1.0)
				color = (color[0], color[1], color[2], int(round(float(color[3])*opacity)))
				self.stops.append((position, color))

class LinearGradientMaterial(Gradient):
	"""Class describing a linear gradient."""
	def __init__(self, node, size, materials):
		Gradient.__init__(self, node, materials)
		if self.coordinateSpace == MaterialSpace.Bounds:
			size = (1.0, 1.0)

		if node.hasAttribute('x1'):
			x1 = sizeFromString(node.getAttribute('x1'), size[0])
		else:
			x1 = 0.0;
		if node.hasAttribute('y1'):
			y1 = sizeFromString(node.getAttribute('y1'), size[1])
		else:
			y1 = 0.0;
		if node.hasAttribute('x2'):
			x2 = sizeFromString(node.getAttribute('x2'), size[0])
		else:
			x2 = size[0];
		if node.hasAttribute('y2'):
			y2 = sizeFromString(node.getAttribute('y2'), size[1])
		else:
			y2 = 0.0;

		self.start = (x1, y1)
		self.end = (x2, y2)

	def write(self, builder):
		nameOffset = builder.CreateString(self.name)
		LinearGradient.StartGradientVector(builder, len(self.stops))
		for position, color in reversed(self.stops):
			CreateGradientStop(builder, position, color[0], color[1], color[2], color[3])
		gradientOffset = builder.EndVector()

		LinearGradient.Start(builder)
		LinearGradient.AddName(builder, nameOffset)
		LinearGradient.AddGradient(builder, gradientOffset)
		LinearGradient.AddStart(builder, CreateVector2f(builder, self.start[0], self.start[1]))
		LinearGradient.AddEnd(builder, CreateVector2f(builder, self.end[0], self.end[1]))
		LinearGradient.AddEdge(builder, self.edge)
		LinearGradient.AddCoordinateSpace(builder, self.coordinateSpace)
		LinearGradient.AddTransform(builder, self.transform.createMatrix33f(builder))
		return LinearGradient.End(builder)
	
class RadialGradientMaterial(Gradient):
	"""Class describing a radial gradient."""
	def __init__(self, node, size, diagonalSize, materials):
		Gradient.__init__(self, node, materials)
		if self.coordinateSpace == MaterialSpace.Bounds:
			size = (1.0, 1.0)
			diagonalSize = 1.0

		if node.hasAttribute('cx'):
			cx = sizeFromString(node.getAttribute('cx'), size[0])
		else:
			cx = size[0]/2.0;
		if node.hasAttribute('cy'):
			cy = sizeFromString(node.getAttribute('cy'), size[1])
		else:
			cy = size[1]/2.0;
		self.center = (cx, cy)
		if node.hasAttribute('r'):
			self.radius = sizeFromString(node.getAttribute('r'), diagonalSize)
		else:
			self.radius = diagonalSize/2.0
		self.focus = [cx, cy]
		self.focusRadius = 0.0
		if node.hasAttribute('fx'):
			self.focus[0] = sizeFromString(node.getAttribute('fx'), size[0])
		if node.hasAttribute('fy'):
			self.focus[1] = sizeFromString(node.getAttribute('fy'), size[1])
		if node.hasAttribute('fr'):
			self.focusRadius = sizeFromString(node.getAttribute('fr'), diagonalSize)

	def write(self, builder):
		nameOffset = builder.CreateString(self.name)
		RadialGradient.StartGradientVector(builder, len(self.stops))
		for position, color in reversed(self.stops):
			CreateGradientStop(builder, position, color[0], color[1], color[2], color[3])
		gradientOffset = builder.EndVector()

		RadialGradient.Start(builder)
		RadialGradient.AddName(builder, nameOffset)
		RadialGradient.AddGradient(builder, gradientOffset)
		RadialGradient.AddCenter(builder, CreateVector2f(builder, self.center[0], self.center[1]))
		RadialGradient.AddRadius(builder, self.radius)
		RadialGradient.AddFocus(builder, CreateVector2f(builder, self.focus[0], self.focus[1]))
		RadialGradient.AddFocusRadius(builder, self.focusRadius)
		RadialGradient.AddEdge(builder, self.edge)
		RadialGradient.AddCoordinateSpace(builder, self.coordinateSpace)
		RadialGradient.AddTransform(builder, self.transform.createMatrix33f(builder))
		return RadialGradient.End(builder)

class Materials:
	"""Class to hold the materials of an image."""
	def __init__(self, name):
		self.name = name
		self.colors = []
		self.linearGradients = {}
		self.radialGradients = {}

	def getColorName(self, index):
		return self.name + '-' + str(index)

	def addColor(self, color):
		for i in range(len(self.colors)):
			if self.colors[i] == color:
				return self.getColorName(i)

		name = self.getColorName(len(self.colors))
		self.colors.append(color)
		return name

	def addLinearGradient(self, linearGradient):
		self.linearGradients[linearGradient.name] = linearGradient

	def addRadialGradient(self, radialGradient):
		self.radialGradients[radialGradient.name] = radialGradient

	def findGradientStops(self, name):
		if name in self.linearGradients:
			return self.linearGradients[name].stops
		if name in self.radialGradients:
			return self.radialGradients[name].stops
		return None

	def write(self, builder):
		colorOffsets = []
		for i in range(len(self.colors)):
			nameOffset = builder.CreateString(self.getColorName(i))
			ColorMaterial.Start(builder)
			ColorMaterial.AddName(builder, nameOffset)
			color = self.colors[i]
			ColorMaterial.AddColor(builder,
				CreateColor(builder, color[0], color[1], color[2], color[3]))
			colorOffsets.append(ColorMaterial.End(builder))

		linearGradientOffsets = []
		for gradient in self.linearGradients.values():
			linearGradientOffsets.append(gradient.write(builder))

		radialGradientOffsets = []
		for gradient in self.radialGradients.values():
			radialGradientOffsets.append(gradient.write(builder))

		VectorImage.StartColorMaterialsVector(builder, len(colorOffsets))
		for offset in reversed(colorOffsets):
			builder.PrependUOffsetTRelative(offset)
		self.colorsOffset = builder.EndVector()

		VectorImage.StartLinearGradientsVector(builder, len(linearGradientOffsets))
		for offset in reversed(linearGradientOffsets):
			builder.PrependUOffsetTRelative(offset)
		self.linearGradientsOffset = builder.EndVector()

		VectorImage.StartRadialGradientsVector(builder, len(radialGradientOffsets))
		for offset in reversed(radialGradientOffsets):
			builder.PrependUOffsetTRelative(offset)
		self.radialGradientsOffset =  builder.EndVector()

	def writeToVectorImage(self, builder):
		VectorImage.AddColorMaterials(builder, self.colorsOffset)
		VectorImage.AddLinearGradients(builder, self.linearGradientsOffset)
		VectorImage.AddRadialGradients(builder, self.radialGradientsOffset)

class Stroke:
	"""Class that describes a stroke."""
	def __init__(self, material):
		self.material = material
		self.opacity = 1.0
		self.join = LineJoin.Miter
		self.cap = LineCap.Butt
		self.width = 1.0
		self.miterLimit = 4.0
		self.dashArray = [0.0, 0.0, 0.0, 0.0]

class Fill:
	"""Class that describes a fill."""
	def __init__(self, material):
		self.material = material
		self.opacity = 1.0
		self.fillRule = FillRule.EvenOdd

class Font:
	"""Class that describes font properties."""
	def __init__(self, font):
		self.font = font
		self.size = 16.0
		self.embolden = 0.0
		self.slant = 0.0
		self.alignment = TextAlign.Start
		self.lineHeight = 1.2
		self.maxLength = None

class Style:
	"""Style used within a vector element."""
	def __init__(self, fill, stroke, font, opacity):
		self.fill = fill
		self.stroke = stroke
		self.font = font
		self.opacity = opacity

	@staticmethod
	def create(node, materials, relativeSize, parentStyle = None, group = False,
		defaultFont = None, width = None, text = False):
		"""Constructs the style with the encoded style."""
		fill = None
		stroke = None
		font = None
		opacity = 1.0

		hasAny = False
		if parentStyle:
			if parentStyle.fill:
				hasAny = True
				fill = Fill(parentStyle.fill.material)
				fill.opacity = parentStyle.fill.opacity
			if parentStyle.stroke:
				hasAny = True
				stroke = Stroke(parentStyle.stroke.material)
				stroke.opacity = parentStyle.stroke.opacity
				stroke.join = parentStyle.stroke.join
				stroke.cap = parentStyle.stroke.cap
				stroke.width = parentStyle.stroke.width
				stroke.miterLimit = parentStyle.stroke.miterLimit
				stroke.dashArray = parentStyle.stroke.dashArray
			if parentStyle.font:
				hasAny = True
				font = Font(parentStyle.font.font)
				font.size = parentStyle.font.size
				font.embolden = parentStyle.font.embolden
				font.slant = parentStyle.font.slant
				font.alignment = parentStyle.font.alignment
				font.lineHeight = parentStyle.font.lineHeight
				font.maxLength = parentStyle.font.maxLength
			opacity = parentStyle.opacity

		elements = extractAttributes(node)
		if 'fill' in elements:
			hasAny = True
			value = elements['fill']
			if value == 'none':
				fill = None
			else:
				if value[:4] == 'url(':
					# Also skip starting #
					material = value[5:-1]
				else:
					material = materials.addColor(colorFromString(value))
				fill = Fill(material)

				if 'fill-rule' in elements:
					if elements['fill-rule'] == 'nonzero':
						fill.fillRule = FillRule.NonZero
					else:
						fill.fillRule = FillRule.EvenOdd
		elif not parentStyle:
			fill = Fill(materials.addColor((0, 0, 0, 255)))
		if fill and 'fill-opacity' in elements:
			fill.opacity = float(elements['fill-opacity'])

		if 'stroke' in elements:
			hasAny = True
			value = elements['stroke']
			if value == 'none':
				stroke = None
			else:
				if value[:4] == 'url(':
					# Also skip starting #
					material = value[5:-1]
				else:
					material = materials.addColor(colorFromString(value))
				stroke = Stroke(material)

				if 'stroke-opacity' in elements:
					stroke.opacity = float(elements['stroke-opacity'])
				if 'stroke-linejoin' in elements:
					stroke.join = lineJoinMap[elements['stroke-linejoin']]
				if 'stroke-linecap' in elements:
					stroke.cap = lineCapMap[elements['stroke-linecap']]
				if 'stroke-width' in elements:
					stroke.width = sizeFromString(elements['stroke-width'], relativeSize)
				if 'stroke-miterlimit' in elements:
					stroke.miterLimit = float(elements['stroke-miterlimit'])
				if 'stroke-dasharray' in elements:
					value = elements['stroke-dasharray']
					if value != 'none':
						dashArray = value.split(',')
						if len(dashArray) > len(stroke.dashArray):
							raise Exception('Dash array may have a maximum of 4 elements.')
						for i in range(len(dashArray)):
							stroke.dashArray[i] = sizeFromString(dashArray[i].strip(),
								relativeSize)

		hasFontElement = text
		if not hasFontElement and defaultFont:
			for textAttr in textAttributes:
				if textAttr in elements:
					hasFontElement = True
					break

		if hasFontElement:
			hasAny = True
			if text and 'color' in elements:
				value = elements['color']
				if value[:4] == 'url(':
					# Also skip starting #
					material = value[5:-1]
				else:
					material = materials.addColor(colorFromString(value))
				fill = Fill(material)

			if not font:
				font = Font(defaultFont)
			if 'font' in elements:
				raise Exception("Combined 'font' elelement not supported, use separate elements "
					"such as 'font-family' instead.")
			if 'font-family' in elements:
				font.font = elements['font-family']
			if 'font-size' in elements:
				font.size = sizeFromString(elements['font-size'], 16.0)
			if 'font-style' in elements:
				fontStyle = elements['font-style']
				if fontStyle == 'italic' or fontStyle == 'oblique':
					font.slant = 0.2
				elif fontStyle == 'normal':
					font.slant = 0.0
			if 'font-weight' in elements:
				fontWeight = elements['font-weight']
				if fontWeight == 'normal':
					weight = 400
				elif fontWeight == 'bold':
					weight = 700
				elif fontWeight == 'bolder':
					weight = 900
				elif fontWeight == 'lighter':
					weight = 200
				else:
					weight = int(fontWeight)
				font.embolden = (float(weight) - 400.0)/2666.0
			if 'text-align' in elements:
				font.alignment = textAlignMap[elements['text-align']]
			if 'text-anchor' in elements:
				font.alignment = textAnchorMap[elements['text-anchor']]
			if 'line-height' in elements:
				try:
					font.lineHeight = float(elements['line-height'])
				except:
					font.lineHeight = sizeFromString(elements['line-height'], font.size)/font.size
			if 'textLength' in elements:
				font.maxLength = sizeFromString(elements['textLength'], width)
			if 'inline-size' in elements:
				font.maxLength = sizeFromString(elements['inline-size'], width)

		if 'opacity' in elements:
			opacity = float(elements['opacity'])

		if not group and not fill and not stroke:
			raise Exception("Shape doesn't have a stroke or a fill.")

		if hasAny or opacity != 1.0:
			return Style(fill, stroke, font, opacity)
		return None

	def write(self, builder):
		offsets = []
		if self.fill:
			materialOffset = builder.CreateString(self.fill.material)

			FillPathCommand.Start(builder)
			FillPathCommand.AddMaterial(builder, materialOffset)
			FillPathCommand.AddOpacity(builder, self.fill.opacity*self.opacity)
			FillPathCommand.AddFillRule(builder, self.fill.fillRule)
			commandOffset = FillPathCommand.End(builder)

			VectorCommand.Start(builder)
			VectorCommand.AddCommandType(builder, VectorCommandUnion.FillPathCommand)
			VectorCommand.AddCommand(builder, commandOffset)
			offsets.append(VectorCommand.End(builder))
		if self.stroke:
			materialOffset = builder.CreateString(self.stroke.material)

			StrokePathCommand.Start(builder)
			StrokePathCommand.AddMaterial(builder, materialOffset)
			StrokePathCommand.AddOpacity(builder, self.stroke.opacity*self.opacity)
			StrokePathCommand.AddJoinType(builder, self.stroke.join)
			StrokePathCommand.AddCapType(builder, self.stroke.cap)
			StrokePathCommand.AddWidth(builder, self.stroke.width)
			StrokePathCommand.AddMiterLimit(builder, self.stroke.miterLimit)
			StrokePathCommand.AddDashArray(builder, CreateDashArray(builder,
				self.stroke.dashArray[0], self.stroke.dashArray[1], self.stroke.dashArray[2],
				self.stroke.dashArray[3]))
			commandOffset = StrokePathCommand.End(builder)

			VectorCommand.Start(builder)
			VectorCommand.AddCommandType(builder, VectorCommandUnion.StrokePathCommand)
			VectorCommand.AddCommand(builder, commandOffset)
			offsets.append(VectorCommand.End(builder))
		return offsets

class TextRange:
	"""Class containing a range of text to draw."""
	def __init__(self, start, count, position, positionType, style):
		self.start = start
		self.count = count
		self.position = position
		self.positionType = positionType
		self.style = style

def writeStartPath(builder, transform, simple):
	StartPathCommand.Start(builder)
	StartPathCommand.AddTransform(builder, transform.createMatrix33f(builder))
	StartPathCommand.AddSimple(builder, simple)
	commandOffset = StartPathCommand.End(builder)

	VectorCommand.Start(builder)
	VectorCommand.AddCommandType(builder, VectorCommandUnion.StartPathCommand)
	VectorCommand.AddCommand(builder, commandOffset)
	return [VectorCommand.End(builder)]

def writeEllipse(builder, transform, style, center, radius):
	offsets = writeStartPath(builder, transform, True)

	EllipseCommand.Start(builder)
	EllipseCommand.AddCenter(builder, CreateVector2f(builder, center[0], center[1]))
	EllipseCommand.AddRadius(builder, CreateVector2f(builder, radius[0], radius[1]))
	commandOffset = EllipseCommand.End(builder)

	VectorCommand.Start(builder)
	VectorCommand.AddCommandType(builder, VectorCommandUnion.EllipseCommand)
	VectorCommand.AddCommand(builder, commandOffset)
	offsets.append(VectorCommand.End(builder))

	offsets.extend(style.write(builder))
	return offsets

def writeImage(builder, transform, style, upperLeft, size, location):
	name = os.path.splitext(os.path.basename(location))[0]
	nameOffset = builder.CreateString(name)

	ImageCommand.Start(builder)
	ImageCommand.AddImage(builder, nameOffset)
	ImageCommand.AddUpperLeft(builder, CreateVector2f(builder, upperLeft[0], upperLeft[1]))
	ImageCommand.AddLowerRight(builder, CreateVector2f(builder, upperLeft[0] + size[0],
		upperLeft[1] + size[1]))
	ImageCommand.AddOpacity(builder, 1.0 if not style else style.opacity)
	ImageCommand.AddTransform(builder, transform.createMatrix33f(builder))
	commandOffset = ImageCommand.End(builder)

	VectorCommand.Start(builder)
	VectorCommand.AddCommandType(builder, VectorCommandUnion.ImageCommand)
	VectorCommand.AddCommand(builder, commandOffset)
	return [VectorCommand.End(builder)]

def writeLines(builder, transform, style, points, closePath = False):
	if not points:
		raise Exception("No points available.")

	offsets = writeStartPath(builder, transform, False)

	MoveCommand.Start(builder)
	MoveCommand.AddPosition(builder, CreateVector2f(builder, points[0][0], points[0][1]))
	commandOffset = MoveCommand.End(builder)

	VectorCommand.Start(builder)
	VectorCommand.AddCommandType(builder, VectorCommandUnion.MoveCommand)
	VectorCommand.AddCommand(builder, commandOffset)
	offsets.append(VectorCommand.End(builder))

	for point in points[1:]:
		LineCommand.Start(builder)
		LineCommand.AddEnd(builder, CreateVector2f(builder, point[0], point[1]))
		commandOffset = LineCommand.End(builder)

		VectorCommand.Start(builder)
		VectorCommand.AddCommandType(builder, VectorCommandUnion.LineCommand)
		VectorCommand.AddCommand(builder, commandOffset)
		offsets.append(VectorCommand.End(builder))

	if closePath:
		ClosePathCommand.Start(builder)
		commandOffset = ClosePathCommand.End(builder)

		VectorCommand.Start(builder)
		VectorCommand.AddCommandType(builder, VectorCommandUnion.ClosePathCommand)
		VectorCommand.AddCommand(builder, commandOffset)
		offsets.append(VectorCommand.End(builder))

	offsets.extend(style.write(builder))
	return offsets

def parsePointList(pointStr, size):
	tokens = re.findall(r"[-+0-9.e]+(?:[eE][-+]?[0-9]+)?(?:cm|mm|Q|in|pc|pt|em|px|%)?", pointStr)
	points = []
	for i in range(int(len(tokens)/2)):
		points.append((sizeFromString(tokens[i*2], size[0]),
			sizeFromString(tokens[i*2 + 1], size[1])))
	return points

def writePolygon(builder, transform, style, pointStr, size):
	points = parsePointList(pointStr, size)
	return writeLines(builder, transform, style, points, True)

def writePolyline(builder, transform, style, pointStr, size):
	points = parsePointList(pointStr, size)
	return writeLines(builder, transform, style, points)

def writePath(builder, transform, style, path, size, diagonalSize):
	offsets = writeStartPath(builder, transform, False)

	tokens = re.findall(
		r"[mMzZlLhHvVcCsSqQtTaAbB]|[-+]?[0-9.]+(?:[eE][-+]?[0-9]+)?"
		"(?:cm|mm|Q|in|pc|pt|em|px|deg|grad|rad|turn|%)?", path)
	pos = (0.0, 0.0)
	lastControlPos = None
	lastQuadraticPos = None
	index = 0
	command = ''
	while index < len(tokens):
		if (tokens[index][0] == '.' or tokens[index][0] == '-' or tokens[index][0] == '+' or
				(ord(tokens[index][0]) >= ord('0') and ord(tokens[index][0]) <= ord('9'))):
			x = sizeFromString(tokens[index], size[0])
			index += 1
			if (command != 'b' and command != 'B' and command != 'h' and command != 'H' and
					command != 'v' and command != 'V'):
				y = sizeFromString(tokens[index], size[1])
				index += 1
			if command == 'm' or command == 'M':
				if command == 'm':
					pos = (pos[0] + x, pos[1] + y)
					command = 'l'
				else:
					pos = (x, y)
					command = 'L'

				MoveCommand.Start(builder)
				MoveCommand.AddPosition(builder, CreateVector2f(builder, pos[0], pos[1]))
				commandOffset = MoveCommand.End(builder)

				VectorCommand.Start(builder)
				VectorCommand.AddCommandType(builder, VectorCommandUnion.MoveCommand)
				VectorCommand.AddCommand(builder, commandOffset)
				offsets.append(VectorCommand.End(builder))
			elif (command == 'l' or command == 'L' or command == 'h' or command == 'H' or
					command == 'v' or command == 'V'):
				if command == 'l':
					pos = (pos[0] + x, pos[1] + y)
				elif command == 'L':
					pos = (x, y)
				elif command == 'h':
					pos = (pos[0] + x, pos[1])
				elif command == 'H':
					pos = (x, pos[1])
				elif command == 'v':
					pos = (pos[0], pos[1] + x)
				elif command == 'V':
					pos = (pos[0], x)

				LineCommand.Start(builder)
				LineCommand.AddEnd(builder, CreateVector2f(builder, pos[0], pos[1]))
				commandOffset = LineCommand.End(builder)

				VectorCommand.Start(builder)
				VectorCommand.AddCommandType(builder, VectorCommandUnion.LineCommand)
				VectorCommand.AddCommand(builder, commandOffset)
				offsets.append(VectorCommand.End(builder))
			elif command == 'c' or command == 'C' or command == 's' or command == 'S':
				if command == 's' or command == 'S':
					if lastControlPos:
						diff = (lastControlPos[0] - pos[0], lastControlPos[1] - pos[1])
						control1 = (pos[0] - diff[0], pos[1] - diff[1])
					else:
						control1 = pos
					if command == 's':
						control2 = (pos[0] + x, pos[1] + y)
					else:
						control2 = (x, y)
				elif command == 'c':
					control1 = (pos[0] + x, pos[1] + y)
					x = sizeFromString(tokens[index], size[0])
					index += 1
					y = sizeFromString(tokens[index], size[1])
					index += 1
					control2 = (pos[0] + x, pos[1] + y)
				elif command == 'C':
					control1 = (x, y)
					x = sizeFromString(tokens[index], size[0])
					index += 1
					y = sizeFromString(tokens[index], size[1])
					index += 1
					control2 = (x, y)

				x = sizeFromString(tokens[index], size[0])
				index += 1
				y = sizeFromString(tokens[index], size[1])
				index += 1
				if command == 'c' or command == 's':
					end = (pos[0] + x, pos[1] + y)
				else:
					end = (x, y)

				BezierCommand.Start(builder)
				BezierCommand.AddControl1(builder, CreateVector2f(builder, control1[0],
					control1[1]))
				BezierCommand.AddControl2(builder, CreateVector2f(builder, control2[0],
					control2[1]))
				BezierCommand.AddEnd(builder, CreateVector2f(builder, end[0], end[1]))
				commandOffset = BezierCommand.End(builder)

				VectorCommand.Start(builder)
				VectorCommand.AddCommandType(builder, VectorCommandUnion.BezierCommand)
				VectorCommand.AddCommand(builder, commandOffset)
				offsets.append(VectorCommand.End(builder))

				pos = end
				lastControlPos = control2
			elif command == 'q' or command == 'Q' or command == 't' or command == 'T':
				if command == 't' or command == 'T':
					if lastQuadraticPos:
						diff = (lastQuadraticPos[0] - pos[0], lastQuadraticPos[1] - pos[1])
						control = (pos[0] - diff[0], pos[1] - diff[1])
					else:
						control = pos
				else:
					if command == 'q':
						control = (pos[0] + x, pos[1] + y)
					elif command == 'Q':
						control = (x, y)
					x = sizeFromString(tokens[index], size[0])
					index += 1
					y = sizeFromString(tokens[index], size[1])
					index += 1

				if command == 'q' or command == 't':
					end = (pos[0] + x, pos[1] + y)
				else:
					end = (x, y)

				QuadraticCommand.Start(builder)
				QuadraticCommand.AddControl(builder, CreateVector2f(builder, control[0],
					control[1]))
				QuadraticCommand.AddEnd(builder, CreateVector2f(builder, end[0], end[1]))
				commandOffset = QuadraticCommand.End(builder)

				VectorCommand.Start(builder)
				VectorCommand.AddCommandType(builder, VectorCommandUnion.QuadraticCommand)
				VectorCommand.AddCommand(builder, commandOffset)
				offsets.append(VectorCommand.End(builder))

				pos = end
				lastQuadraticPos = control
			elif command == 'a' or command == 'A':
				radius = (x, y)
				rotation = angleFromString(tokens[index])
				index += 1
				largeArc = int(tokens[index]) != 0
				index += 1
				sweep = int(tokens[index]) != 0
				index += 1
				x = sizeFromString(tokens[index], size[0])
				index += 1
				y = sizeFromString(tokens[index], size[1])
				index += 1
				if command == 'a':
					end = (pos[0] + x, pos[1] + y)
				else:
					end = (x, y)

				ArcCommand.Start(builder)
				ArcCommand.AddRadius(builder, CreateVector2f(builder, radius[0], radius[1]))
				ArcCommand.AddRotation(builder, rotation)
				ArcCommand.AddLargeArc(builder, largeArc)
				ArcCommand.AddClockwise(builder, sweep)
				ArcCommand.AddEnd(builder, CreateVector2f(builder, end[0], end[1]))
				commandOffset = ArcCommand.End(builder)

				VectorCommand.Start(builder)
				VectorCommand.AddCommandType(builder, VectorCommandUnion.ArcCommand)
				VectorCommand.AddCommand(builder, commandOffset)
				offsets.append(VectorCommand.End(builder))

				pos = end
			elif command == 'b' or command == 'B':
				raise Exception('Bearing currently not implemented. '
					'It is generally not implemented by other SVG renderers either.')
		elif tokens[index] == 'z' or tokens[index] == 'Z':
			ClosePathCommand.Start(builder)
			commandOffset = ClosePathCommand.End(builder)

			VectorCommand.Start(builder)
			VectorCommand.AddCommandType(builder, VectorCommandUnion.ClosePathCommand)
			VectorCommand.AddCommand(builder, commandOffset)
			offsets.append(VectorCommand.End(builder))

			lastControlPos = None
			lastQuadraticPos = None
			index += 1
		else:
			# Reset the last control pos if not a curve command.
			if command != 'c' and command != 'C' and command != 's' and command != 'S':
				lastControlPos = None
			if command != 'q' and command != 'Q' and command != 't' and command != 'T':
				lastQuadraticPos = None
			command = tokens[index]
			index += 1

	offsets.extend(style.write(builder))
	return offsets

def writeRectangle(builder, transform, style, upperLeft, rectSize, radius):
	offsets = writeStartPath(builder, transform, True)

	RectangleCommand.Start(builder)
	RectangleCommand.AddUpperLeft(builder, CreateVector2f(builder, upperLeft[0], upperLeft[1]))
	RectangleCommand.AddLowerRight(builder, CreateVector2f(builder, upperLeft[0] + rectSize[0],
		upperLeft[1] + rectSize[1]))
	RectangleCommand.AddCornerRadius(builder, CreateVector2f(builder, radius[0], radius[1]))
	commandOffset = RectangleCommand.End(builder)

	VectorCommand.Start(builder)
	VectorCommand.AddCommandType(builder, VectorCommandUnion.RectangleCommand)
	VectorCommand.AddCommand(builder, commandOffset)
	offsets.append(VectorCommand.End(builder))
	
	offsets.extend(style.write(builder))
	return offsets

def writeText(builder, transform, font, text, rangeCount):
	textOffset = builder.CreateString(text)
	fontOffset = builder.CreateString(font.font)

	TextCommand.Start(builder)
	TextCommand.AddText(builder, textOffset)
	TextCommand.AddFont(builder, fontOffset)
	TextCommand.AddAlignment(builder, font.alignment)
	TextCommand.AddMaxLength(builder, font.maxLength if font.maxLength else 3.402823e+38)
	TextCommand.AddLineHeight(builder, font.lineHeight)
	TextCommand.AddTransform(builder, transform.createMatrix33f(builder))
	TextCommand.AddRangeCount(builder, rangeCount)
	commandOffset = TextCommand.End(builder)

	VectorCommand.Start(builder)
	VectorCommand.AddCommandType(builder, VectorCommandUnion.TextCommand)
	VectorCommand.AddCommand(builder, commandOffset)
	return [VectorCommand.End(builder)]

def writeTextRange(builder, textRange):
	style = textRange.style
	fillMaterialOffset = 0
	fillOpacity = 0.0
	if style.fill:
		fillMaterialOffset = builder.CreateString(style.fill.material)
		fillOpacity = style.opacity*style.fill.opacity

	outlineMaterialOffset = 0
	outlineOpacity = 0.0
	outlineWidth = 0.0
	if style.stroke:
		outlineMaterialOffset = builder.CreateString(style.stroke.material)
		outlineOpacity = style.opacity*style.stroke.opacity
		sizeToWidthFactor = 1.5/style.font.size
		outlineWidth = style.stroke.width*sizeToWidthFactor

	TextRangeCommand.Start(builder)
	TextRangeCommand.AddStart(builder, textRange.start)
	TextRangeCommand.AddCount(builder, textRange.count)
	TextRangeCommand.AddPositionType(builder, textRange.positionType)
	TextRangeCommand.AddPosition(builder,
		CreateVector2f(builder, textRange.position[0], textRange.position[1]))
	TextRangeCommand.AddFillMaterial(builder, fillMaterialOffset)
	TextRangeCommand.AddOutlineMaterial(builder, outlineMaterialOffset)
	TextRangeCommand.AddFillOpacity(builder, fillOpacity)
	TextRangeCommand.AddOutlineOpacity(builder, outlineOpacity)
	TextRangeCommand.AddSize(builder, style.font.size)
	TextRangeCommand.AddEmbolden(builder, style.font.embolden)
	TextRangeCommand.AddSlant(builder, style.font.slant)
	TextRangeCommand.AddOutlineWidth(builder, outlineWidth)
	TextRangeCommand.AddFuziness(builder, 1.0)
	commandOffset = TextRangeCommand.End(builder)

	VectorCommand.Start(builder)
	VectorCommand.AddCommandType(builder, VectorCommandUnion.TextRangeCommand)
	VectorCommand.AddCommand(builder, commandOffset)
	return [VectorCommand.End(builder)]

def readMaterials(node, materials, size, diagonalSize):
	for defNode in node.childNodes:
		if defNode.nodeType != xml.dom.Node.ELEMENT_NODE:
			continue

		if defNode.tagName == 'linearGradient':
			gradient = LinearGradientMaterial(defNode, size, materials)
			materials.addLinearGradient(gradient)
		elif defNode.tagName == 'radialGradient':
			gradient = RadialGradientMaterial(defNode, size, diagonalSize,
				materials)
			materials.addRadialGradient(gradient)

def readText(node, defaultFont, size, diagonalSize, materials, style = None):
	if not node or node.tagName != 'text':
		return None, None, None

	rootStyle = Style.create(node, materials, diagonalSize, style, defaultFont = defaultFont,
		width = size[0], text = True)
	text = u""
	initialPosition = (0.0, 0.0)
	if node.hasAttribute('x') and node.hasAttribute('y'):
		initialPosition = (sizeFromString(node.getAttribute('x'), size[0]),
			sizeFromString(node.getAttribute('y'), size[1]))
	ranges = [TextRange(0, 0, initialPosition, TextPosition.Absolute, rootStyle)]

	for child in node.childNodes:
		if child.nodeType == xml.dom.Node.ELEMENT_NODE:
			rangeStyle = Style.create(child, materials, diagonalSize, rootStyle,
				defaultFont = defaultFont, width = size[0], text = True)
			curText = u""

			for nextChild in child.childNodes:
				if nextChild.nodeType == xml.dom.Node.TEXT_NODE:
					if sys.version_info < (3, 0):
						textPiece = unicode(nextChild.data)
					else:
						textPiece = nextChild.data
					if curText:
						curText += ' '
					curText += textPiece.strip()

			if text:
				curText = ' ' + curText

			position = (0.0, 0.0)
			positionType = TextPosition.Offset

			if not rootStyle.font.maxLength and child.hasAttribute('x') and child.hasAttribute('y'):
				position = (sizeFromString(child.getAttribute('x'), size[0]),
					sizeFromString(child.getAttribute('y'), size[1]))
				positionType = TextPosition.Absolute
			elif child.hasAttribute('dx') or child.hasAttribute('dy'):
				if child.hasAttribute('dx'):
					position = (sizeFromString(child.getAttribute('dx'), size[0]), 0.0)
				if child.hasAttribute('dy'):
					position = (position[0], sizeFromString(child.getAttribute('dy'), size[1]))
				positionType = TextPosition.Offset
			if curText:
				ranges.append(TextRange(len(text), len(curText), position, positionType,
					rangeStyle))
			text += curText
		elif child.nodeType == xml.dom.Node.TEXT_NODE:
			if sys.version_info < (3, 0):
				curText = unicode(child.data)
			else:
				curText = child.data
			curText = curText.strip()
			if curText:
				if text:
					curText = ' ' + curText
				if ranges[-1].style == rootStyle:
					ranges[-1].count += len(curText)
				else:
					ranges.append(TextRange(len(text), len(curText), (0.0, 0.0),
						TextPosition.Offset, rootStyle))
				text += curText

	return rootStyle.font, text, ranges

def readShapes(node, defaultFont, materials, size, diagonalSize, transform, style = None):
	commands = []
	if node.tagName == 'g':
		groupTransform = transform*Transform.fromNode(node)
		groupStyle = Style.create(node, materials, diagonalSize, style, group = True,
			defaultFont = defaultFont, width = size[0])
		for groupNode in node.childNodes:
			if groupNode.nodeType == xml.dom.Node.ELEMENT_NODE:
				commands.extend(readShapes(groupNode, defaultFont, materials, size, diagonalSize,
					groupTransform, groupStyle))
	elif node.tagName == 'circle':
		commands.append(lambda builder,
			transform = transform*Transform.fromNode(node),
			style = Style.create(node, materials, diagonalSize, style),
			center = (sizeFromString(node.getAttribute('cx'), size[0]),
				sizeFromString(node.getAttribute('cy'), size[1])),
			radius = sizeFromString(node.getAttribute('r'), diagonalSize):
			writeEllipse(builder, transform, style, center, (radius, radius)))
	elif node.tagName == 'ellipse':
		commands.append(lambda builder,
			transform = transform*Transform.fromNode(node),
			style = Style.create(node, materials, diagonalSize, style),
			center = (sizeFromString(node.getAttribute('cx'), size[0]),
				sizeFromString(node.getAttribute('cy'), size[1])),
			radius = (sizeFromString(node.getAttribute('rx'), diagonalSize),
				sizeFromString(node.getAttribute('ry'), diagonalSize)):
			writeEllipse(builder, transform, style, center, radius))
	elif node.tagName == 'image':
		commands.append(lambda builder,
			transform = transform*Transform.fromNode(node),
			style = Style.create(node, materials, diagonalSize, style),
			upperLeft = (sizeFromString(node.getAttribute('x'), size[0]),
				sizeFromString(node.getAttribute('y'), size[1])),
			imageSize = (sizeFromString(node.getAttribute('width'), size[0]),
				sizeFromString(node.getAttribute('height'), size[1])),
			location = node.getAttribute('xlink:href'):
			writeImage(builder, transform, style, upperLeft, imageSize, location))
	elif node.tagName == 'line':
		commands.append(lambda builder,
			transform = transform*Transform.fromNode(node),
			style = Style.create(node, materials, diagonalSize, style),
			start = (sizeFromString(node.getAttribute('x1'), size[0]),
				sizeFromString(node.getAttribute('y1'), size[1])),
			end = (sizeFromString(node.getAttribute('x2'), size[0]),
				sizeFromString(node.getAttribute('y2'), size[1])):
			writeLines(builder, transform, style, [start, end]))
	elif node.tagName == 'path':
		commands.append(lambda builder,
			transform = transform*Transform.fromNode(node),
			style = Style.create(node, materials, diagonalSize, style),
			path = node.getAttribute('d'):
			writePath(builder, transform, style, path, size, diagonalSize))
	elif node.tagName == 'polygon':
		commands.append(lambda builder,
			transform = transform*Transform.fromNode(node),
			style = Style.create(node, materials, diagonalSize, style),
			points = node.getAttribute('points'):
			writePolygon(builder, transform, style, points, size))
	elif node.tagName == 'polyline':
		commands.append(lambda builder,
			transform = transform*Transform.fromNode(node),
			style = Style.create(node, materials, diagonalSize, style),
			points = node.getAttribute('points'):
			writePolyline(builder, transform, style, points, size))
	elif node.tagName == 'rect':
		commands.append(lambda builder,
			transform = transform*Transform.fromNode(node),
			style = Style.create(node, materials, diagonalSize, style),
			upperLeft = (sizeFromString(node.getAttribute('x'), size[0]),
				sizeFromString(node.getAttribute('y'), size[1])),
			rectSize = (sizeFromString(node.getAttribute('width'), size[0]),
				sizeFromString(node.getAttribute('height'), size[1])),
			radius = ((sizeFromString(node.getAttribute('rx'), size[0]),
				sizeFromString(node.getAttribute('ry'), size[1]))
				if node.hasAttribute('rx') else (0.0, 0.0)):
			writeRectangle(builder, transform, style, upperLeft, rectSize, radius))
	elif node.tagName == 'text':
		font, text, ranges = readText(node, defaultFont, size, diagonalSize, materials, style)

		commands.append(lambda builder,
			transform = transform*Transform.fromNode(node),
			font = font, text = text, rangeCount = len(ranges):
			writeText(builder, transform, font, text, rangeCount))
		for textRange in ranges:
			commands.append(lambda builder, textRange = textRange:
				writeTextRange(builder, textRange))
	return commands

def convertSVG(streamOrPath, name, defaultFont):
	"""
	Loads an SVG and converts it to a DeepSea vector image FlatBuffer format.

	streamOrPath: the stream or path for the SVG file.
	name: the name of the vector image used to decorate material names.
	defaultFont: the default font to use.
	The binary data is returned.
	"""
	svg = minidom.parse(streamOrPath)
	materials = Materials(name)

	commands = []
	for rootNode in svg.childNodes:
		if rootNode.nodeType != xml.dom.Node.ELEMENT_NODE:
			continue

		if rootNode.tagName == 'svg':
			if rootNode.hasAttribute('viewBox'):
				box = rootNode.getAttribute('viewBox').split()
				if len(box) != 4:
					raise Exception("Invalid view box '" + rootNode.getAttribute('viewbox') + "'")
				if sizeFromString(box[0], 0.0) != 0.0 or sizeFromString(box[1], 0.0) != 0.0:
					raise Exception("View box must have an origin of (0, 0)")
				size = (sizeFromString(box[2], 0.0), sizeFromString(box[3], 0.0))
			elif rootNode.hasAttribute('width') and rootNode.hasAttribute('height'):
				size = (sizeFromString(rootNode.getAttribute('width'), 0.0),
					sizeFromString(rootNode.getAttribute('height'), 0.0))
			else:
				raise Exception("No size set on SVG.")
			diagonalSize = math.sqrt(size[0]*size[0] + size[1]*size[1])/math.sqrt(2)
			for node in rootNode.childNodes:
				if node.nodeType != xml.dom.Node.ELEMENT_NODE:
					continue

				if node.tagName == 'defs':
					readMaterials(node, materials, size, diagonalSize)
				else:
					commands.extend(readShapes(node, defaultFont, materials, size, diagonalSize,
						Transform()))
		break
	
	builder = flatbuffers.Builder(0)

	materials.write(builder)

	commandOffsets = []
	for command in commands:
		commandOffsets.extend(command(builder))

	VectorImage.StartCommandsVector(builder, len(commandOffsets))
	for offset in reversed(commandOffsets):
		builder.PrependUOffsetTRelative(offset)
	commandsOffset = builder.EndVector()

	VectorImage.Start(builder)
	materials.writeToVectorImage(builder)
	VectorImage.AddCommands(builder, commandsOffset)
	VectorImage.AddSize(builder, CreateVector2f(builder, size[0], size[1]))
	builder.Finish(VectorImage.End(builder))
	return builder.Output()
