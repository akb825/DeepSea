#!/usr/bin/python
# Copyright 2018 Aaron Barany
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
import argparse
import locale
import math
import os
import re
import xml.dom
from xml.dom import minidom

import flatbuffers
from Colors import *
from DeepSeaVectorDraw.ArcCommand import *
from DeepSeaVectorDraw.BezierCommand import *
from DeepSeaVectorDraw.ClosePathCommand import *
from DeepSeaVectorDraw.Color import *
from DeepSeaVectorDraw.ColorMaterial import *
from DeepSeaVectorDraw.DashArray import *
from DeepSeaVectorDraw.EllipseCommand import *
from DeepSeaVectorDraw.FillPathCommand import *
from DeepSeaVectorDraw.GradientEdge import *
from DeepSeaVectorDraw.GradientStop import *
from DeepSeaVectorDraw.ImageCommand import *
from DeepSeaVectorDraw.LinearGradient import *
from DeepSeaVectorDraw.LineCap import *
from DeepSeaVectorDraw.LineCommand import *
from DeepSeaVectorDraw.LineJoin import *
from DeepSeaVectorDraw.MaterialSpace import *
from DeepSeaVectorDraw.Matrix33f import *
from DeepSeaVectorDraw.MoveCommand import *
from DeepSeaVectorDraw.QuadraticCommand import *
from DeepSeaVectorDraw.RadialGradient import *
from DeepSeaVectorDraw.RectangleCommand import *
from DeepSeaVectorDraw.StartPathCommand import *
from DeepSeaVectorDraw.StrokePathCommand import *
from DeepSeaVectorDraw.TextCommand import *
from DeepSeaVectorDraw.TextJustification import *
from DeepSeaVectorDraw.TextPathCommand import *
from DeepSeaVectorDraw.TextPosition import *
from DeepSeaVectorDraw.TextRangeCommand import *
from DeepSeaVectorDraw.Vector2f import *
from DeepSeaVectorDraw.Vector3f import *
from DeepSeaVectorDraw.VectorCommand import *
from DeepSeaVectorDraw.VectorCommandUnion import *
from DeepSeaVectorDraw.VectorImage import *

lineJoinMap = {'miter': LineJoin.Miter, 'bevel': LineJoin.Bevel, 'round': LineJoin.Round}
lineCapMap = {'butt': LineCap.Butt, 'round': LineCap.Round, 'square': LineCap.Square}

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

def transformFromNode(node, transformName = 'transform'):
	"""Extracts the transform from a node as a 3x3 matrix tuple in column-major order."""
	if not node.hasAttribute(transformName):
		return ((1.0, 0.0, 0.0), (0.0, 1.0, 0.0), (0.0, 0.0, 1.0))

	try:
		# Support a single matrix or translate element.
		transformStr = node.getAttribute(transformName)
		if transformStr[:7] == 'matrix(':
			values = re.findall(r"[-+0-9.]+", transformStr[7:-1])
			if len(values) != 6:
				raise Exception()
			return ((float(values[0].strip()), float(values[1].strip()), 0.0),
				(float(values[2].strip()), float(values[3].strip()), 0.0),
				(float(values[4].strip()), float(values[5].strip()), 1.0))
		elif transformStr[:10] == 'translate(':
			values = re.findall(r"[-+0-9.]+", transformStr[10:-1])
			if len(values) != 2:
				raise Exception()
			return ((1.0, 0.0, 0.0), (0.0, 1.0, 0.0),
				(float(values[0].strip()), float(values[1].strip()), 1.0))
		else:
			raise Exception()
	except:
		raise Exception('Invalid transform value "' + transformStr + '"')

class Gradient:
	"""Base class for a gradient."""
	def __init__(self, node):
		self.name = node.getAttribute('id')
		self.transform = transformFromNode(node, 'gradientTransform')
		if node.hasAttribute('gradientUnits'):
			units = node.getAttribute('gradientUnits')
			if units == 'objectBoundingBox':
				self.coordinateSpace = MaterialSpace.Bounds
			else:
				self.coordinateSpace = MaterialSpace.Local
		else:
			self.coordinateSpace = MaterialSpace.Local
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

		self.stops = []
		for stop in node.childNodes:
			position = sizeFromString(stop.getAttribute('offset'), 1.0)
			color = colorFromString(stop.getAttribute('stop-color'))
			opacity = 1.0
			if stop.hasAttribute('stop-opacity'):
				opacity = stop.sizeFromString(stop.getAttribute('stop-opacity'), 1.0)
			color[3] = int(round(float(color[3])*opacity))
			self.stops.append(position, color)

class LinearGradientMaterial(Gradient):
	"""Class describing a linear gradient."""
	def __init__(self, node, size):
		Gradient.__init__(self, node)
		self.start = (sizeFromString(node.getAttribute('x1'), size[0]),
			sizeFromString(node.getAttribute('y1'), size[1]))
		self.end = (sizeFromString(node.getAttribute('x2'), size[0]),
			sizeFromString(node.getAttribute('y2'), size[1]))

	def write(self, builder):
		nameOffset = builder.CreateString(self.name)
		LinearGradientStartGradientVector(builder, len(self.stops))
		for position, color in self.stops:
			CreateGradientStop(builder, position, color[0], color[1], color[2], color[3])
		gradientOffset = builder.EndVector(len(self.stops))

		LinearGradientStart(builder)
		LinearGradientAddName(builder, nameOffset)
		LinearGradientAddGradient(builder, gradientOffset)
		LinearGradientAddStart(builder, CreateVector2f(builder, self.start[0], self.start[1]))
		LinearGradientAddEnd(builder, CreateVector2f(builder, self.end[0], self.end[1]))
		LinearGradientAddEdge(builder, self.edge)
		LinearGradientAddCoordinateSpace(builder, self.coordinateSpace)
		LinearGradientAddTransform(builder, CreateMatrix33f(builder, self.transform[0][0],
			self.transform[0][1], self.transform[0][2], self.transform[1][0], self.transform[1][1],
			self.transform[1][2], self.transform[2][0], self.transform[2][1],
			self.transform[2][2]))
		return LinearGradientEnd(builder)
	
class RadialGradientMaterial(Gradient):
	"""Class describing a radial gradient."""
	def __init__(self, node, size, diagonalSize):
		Gradient.__init__(self, node)
		self.center = (sizeFromString(node.getAttribute('cx'), size[0]),
			sizeFromString(node.getAttribute('cy'), size[1]))
		self.radius = sizeFromString(node.getAttribute('radius'), diagonalSize)
		self.focus = [self.center[0], self.center[1]]
		self.focusRadius = 0.0
		if node.hasAttribute('fx'):
			self.focus[0] = sizeFromString(node.getAttribute('fx'), self.radius)
		if node.hasAttribute('fy'):
			self.focus[1] = sizeFromString(node.getAttribute('fy'), self.radius)
		if node.hasAttribute('fr'):
			self.focusRadius = sizeFromString(node.getAttribute('fr'), self.radius)

	def write(self, builder):
		nameOffset = builder.CreateString(self.name)
		RadialGradientStartGradientVector(builder, len(self.stops))
		for position, color in self.stops:
			CreateGradientStop(builder, position, color[0], color[1], color[2], color[3])
		gradientOffset = builder.EndVector(len(self.stops))

		RadialGradientStart(builder)
		RadialGradientAddName(builder, nameOffset)
		RadialGradientAddGradient(builder, gradientOffset)
		RadialGradientAddCenter(builder, CreateVector2f(builder, self.center[0], self.center[1]))
		RadialGradientAddRadius(builder, self.radius)
		RadialGradientAddFocus(builder, CreateVector2f(builder, self.focus[0], self.focus[1]))
		RadialGradientAddFocusRadius(builder, self.focusRadius)
		RadialGradientAddEdge(builder, self.edge)
		RadialGradientAddCoordinateSpace(builder, self.coordinateSpace)
		RadialGradientAddTransform(builder, CreateMatrix33f(builder, self.transform[0][0],
			self.transform[0][1], self.transform[0][2], self.transform[1][0], self.transform[1][1],
			self.transform[1][2], self.transform[2][0], self.transform[2][1],
			self.transform[2][2]))
		return RadialGradientEnd(builder)

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
		name = self.getColorName(len(self.colors))
		self.colors.append(color)
		return name

	def addLinearGradient(self, linearGradient):
		self.linearGradients[linearGradient.name] = linearGradient

	def addRadialGradient(self, radialGradient):
		self.radialGradients[radialGradient.name] = radialGradient

	def write(self, builder):
		colorOffsets = []
		for i in range(len(self.colors)):
			nameOffset = builder.CreateString(self.getColorName(i))
			ColorMaterialStart(builder)
			ColorMaterialAddName(builder, nameOffset)
			color = self.colors[i]
			ColorMaterialAddColor(builder,
				CreateColor(builder, color[0], color[1], color[2], color[3]))
			colorOffsets.append(ColorMaterialEnd(builder))

		linearGradientOffsets = []
		for gradient in self.linearGradients.values():
			linearGradientOffsets.append(gradient.write(builder))

		radialGradientOffsets = []
		for gradient in self.radialGradients.values():
			radialGradientOffsets.append(gradient.write(builder))

		VectorImageStartColorMaterialsVector(builder, len(colorOffsets))
		for offset in reversed(colorOffsets):
			builder.PrependUOffsetTRelative(offset)
		self.colorsOffset = builder.EndVector(len(colorOffsets))

		VectorImageStartLinearGradientsVector(builder, len(linearGradientOffsets))
		for offset in reversed(linearGradientOffsets):
			builder.PrependUOffsetTRelative(offset)
		self.linearGradientsOffset = builder.EndVector(len(linearGradientOffsets))

		VectorImageStartRadialGradientsVector(builder, len(radialGradientOffsets))
		for offset in reversed(radialGradientOffsets):
			builder.PrependUOffsetTRelative(offset)
		self.radialGradientsOffset =  builder.EndVector(len(radialGradientOffsets))

	def writeToVectorImage(self, builder):
		VectorImageAddColorMaterials(builder, self.colorsOffset)
		VectorImageAddLinearGradients(builder, self.linearGradientsOffset)
		VectorImageAddRadialGradients(builder, self.radialGradientsOffset)

class Stroke:
	"""Class that describes a stroke."""
	def __init__(self, material):
		self.material = material
		self.opacity = 1.0
		self.join = LineJoin.Miter
		self.cap = LineCap.Butt
		self.width = 1.0
		self.miterLimit = 1.0
		self.dashArray = [0.0, 0.0, 0.0, 0.0]

class Fill:
	"""Class that describes a fill."""
	def __init__(self, material):
		self.material = material
		self.opacity = 1.0

class Style:
	"""Style used within a vector element."""
	def __init__(self, node, materials, relativeSize):
		"""Constructs the style with the encoded style."""
		self.stroke = None
		self.fill = None
		self.opacity = 1.0

		if not node.hasAttribute('style'):
			return
		styleString = node.getAttribute('style')
		elementStrings = styleString.split(';')
		for elementStr in elementStrings:
			elementPair = elementStr.split(':')
			if len(elementPair) != 2:
				raise Exception('Invalid style string "' + styleString + '"')
			element = elementPair[0].strip()
			value = elementPair[1].strip()
			if element == 'fill':
				if value == 'none':
					continue

				if value[:4] == 'url(':
					material = value[4:-1]
				else:
					material = materials.addColor(colorFromString(value))
				self.fill = Fill(material)
			elif element == 'fill-opacity':
				self.fill.opacity = float(value)
			elif element == 'stroke':
				if value == 'none':
					continue

				if value[:4] == 'url(':
					material = value[4:-1]
				else:
					material = materials.addColor(colorFromString(value))
				self.stroke = Stroke(material)
			elif element == 'stroke-opacity':
				self.stroke.opacity = float(value)
			elif element == 'stroke-linejoin':
				self.stroke.join = lineJoinMap[value]
			elif element == 'stroke-linecap':
				self.stroke.cap = lineCapMap[value]
			elif element == 'stroke-width':
				self.stroke.width = sizeFromString(value, relativeSize)
			elif element == 'stroke-miterlimit':
				self.stroke.miterLimit = float(value)
			elif element == 'stroke-dasharray':
				if value == 'none':
					continue

				dashArray = value.split(',')
				if len(dashArray) > len(self.stroke.dashArray):
					raise Exception('Dash array may have a maximum of 4 elements.')
				for i in range(len(dashArray)):
					self.stroke.dashArray[i] = sizeFromString(dashArray[i].strip(), relativeSize)
			elif element == 'opacity':
				self.opacity = float(value)

		if self.stroke:
			self.stroke.opacity *= self.opacity
		if self.fill:
			self.fill.opacity *= self.opacity

	def write(self, builder):
		offsets = []
		if self.fill:
			materialOffset = builder.CreateString(self.fill.material)

			FillPathCommandStart(builder)
			FillPathCommandAddMaterial(builder, materialOffset)
			FillPathCommandAddOpacity(builder, self.fill.opacity)
			commandOffset = FillPathCommandEnd(builder)

			VectorCommandStart(builder)
			VectorCommandAddCommandType(builder, VectorCommandUnion.FillPathCommand)
			VectorCommandAddCommand(builder, commandOffset)
			offsets.append(VectorCommandEnd(builder))
		if self.stroke:
			materialOffset = builder.CreateString(self.stroke.material)

			StrokePathCommandStart(builder)
			StrokePathCommandAddMaterial(builder, materialOffset)
			StrokePathCommandAddOpacity(builder, self.stroke.opacity)
			StrokePathCommandAddJoinType(builder, self.stroke.join)
			StrokePathCommandAddCapType(builder, self.stroke.cap)
			StrokePathCommandAddWidth(builder, self.stroke.width)
			StrokePathCommandAddMiterLimit(builder, self.stroke.miterLimit)
			StrokePathCommandAddDashArray(builder, CreateDashArray(builder,
				self.stroke.dashArray[0], self.stroke.dashArray[1], self.stroke.dashArray[2],
				self.stroke.dashArray[3]))
			commandOffset = StrokePathCommandEnd(builder)

			VectorCommandStart(builder)
			VectorCommandAddCommandType(builder, VectorCommandUnion.StrokePathCommand)
			VectorCommandAddCommand(builder, commandOffset)
			offsets.append(VectorCommandEnd(builder))
		return offsets

def writeStartPath(builder, transform):
	StartPathCommandStart(builder)
	StartPathCommandAddTransform(builder, CreateMatrix33f(builder, transform[0][0], transform[0][1],
		transform[0][2], transform[1][0], transform[1][1], transform[1][2], transform[2][0],
		transform[2][1], transform[2][2]))
	commandOffset = StartPathCommandEnd(builder)

	VectorCommandStart(builder)
	VectorCommandAddCommandType(builder, VectorCommandUnion.StartPathCommand)
	VectorCommandAddCommand(builder, commandOffset)
	return [VectorCommandEnd(builder)]

def writeEllipse(builder, transform, style, center, radius):
	offsets = writeStartPath(builder, transform)

	EllipseCommandStart(builder)
	EllipseCommandAddCenter(builder, CreateVector2f(builder, center[0], center[1]))
	EllipseCommandAddRadius(builder, CreateVector2f(builder, radius[0], radius[1]))
	commandOffset = EllipseCommandEnd(builder)

	VectorCommandStart(builder)
	VectorCommandAddCommandType(builder, VectorCommandUnion.EllipseCommand)
	VectorCommandAddCommand(builder, commandOffset)
	offsets.append(VectorCommandEnd(builder))

	offsets.extend(style.write(builder))
	return offsets

def writeImage(builder, transform, style, upperLeft, size, location):
	name = os.path.splitext(os.path.basename(location))[0]
	nameOffset = builder.CreateString(name)

	ImageCommandStart(builder)
	ImageCommandAddImage(builder, nameOffset)
	ImageCommandAddUpperLeft(builder, CreateVector2f(builder, upperLeft[0], upperLeft[1]))
	ImageCommandAddLowerRight(builder, CreateVector2f(builder, upperLeft[0] + size[0],
		upperLeft[1] + size[1]))
	ImageCommandAddOpacity(builder, style.opacity)
	ImageCommandAddTransform(builder, CreateMatrix33f(builder, transform[0][0], transform[0][1],
		transform[0][2], transform[1][0], transform[1][1], transform[1][2], transform[2][0],
		transform[2][1], transform[2][2]))
	commandOffset = ImageCommandEnd(builder)

	VectorCommandStart(builder)
	VectorCommandAddCommandType(builder, VectorCommandUnion.ImageCommand)
	VectorCommandAddCommand(builder, commandOffset)
	return [VectorCommandEnd(builder)]

def writeLines(builder, transform, style, points, closePath = False):
	if not points:
		raise Exception("No points available.")

	offsets = writeStartPath(builder, transform)

	MoveCommandStart(builder)
	MoveCommandAddPosition(builder, CreateVector2f(builder, points[0][0], points[0][1]))
	commandOffset = MoveCommandEnd(builder)

	VectorCommandStart(builder)
	VectorCommandAddCommandType(builder, VectorCommandUnion.MoveCommand)
	VectorCommandAddCommand(builder, commandOffset)
	offsets.append(VectorCommandEnd(builder))

	for point in points[1:]:
		LineCommandStart(builder)
		LineCommandAddEnd(builder, CreateVector2f(builder, point[0], point[1]))
		commandOffset = LineCommandEnd(builder)

		VectorCommandStart(builder)
		VectorCommandAddCommandType(builder, VectorCommandUnion.LineCommand)
		VectorCommandAddCommand(builder, commandOffset)
		offsets.append(VectorCommandEnd(builder))

	if closePath:
		ClosePathCommandStart(builder)
		commandOffset = ClosePathCommandEnd(builder)

		VectorCommandStart(builder)
		VectorCommandAddCommandType(builder, VectorCommandUnion.ClosePathCommand)
		VectorCommandAddCommand(builder, commandOffset)
		offsets.append(VectorCommandEnd(builder))

	offsets.extend(style.write(builder))
	return offsets

def parsePointList(pointStr, size):
	tokens = re.findall(r"[-+0-9.]+(?:cm|mm|Q|in|pc|pt|px|%)?", pointStr)
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
	offsets = writeStartPath(builder, transform)

	tokens = re.findall(
		r"[mMzZlLhHvVcCsSqQtTaAbB]|[-+0-9.]+(?:cm|mm|Q|in|pc|pt|px|deg|grad|rad|turn|%)?", path)
	pos = (0.0, 0.0)
	lastControlPos = None
	lastQuadraticPos = None
	index = 0
	command = ''
	while index < len(tokens):
		if tokens[index][0] == '.' or tokens[index][0] == '-' or tokens[index][0] == '+' or \
			(ord(tokens[index][0]) >= ord('0') and ord(tokens[index][0]) <= ord('9')):
			x = sizeFromString(tokens[index], size[0])
			index += 1
			if command != 'b' and command != 'B' and command != 'h' and command != 'H' and \
				command != 'v' and command != 'V':
				y = sizeFromString(tokens[index], size[1])
				index += 1
			if command == 'm' or command == 'M':
				if command == 'm':
					pos = (pos[0] + x, pos[1] + y)
				else:
					pos = (x, y)

				MoveCommandStart(builder)
				MoveCommandAddPosition(builder, CreateVector2f(builder, pos[0], pos[1]))
				commandOffset = MoveCommandEnd(builder)

				VectorCommandStart(builder)
				VectorCommandAddCommandType(builder, VectorCommandUnion.MoveCommand)
				VectorCommandAddCommand(builder, commandOffset)
				offsets.append(VectorCommandEnd(builder))

				command = 'l'
			elif command == 'l' or command == 'L' or command == 'h' or command == 'H' or \
				command == 'v' or command == 'V':
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

				LineCommandStart(builder)
				LineCommandAddEnd(builder, CreateVector2f(builder, pos[0], pos[1]))
				commandOffset = LineCommandEnd(builder)

				VectorCommandStart(builder)
				VectorCommandAddCommandType(builder, VectorCommandUnion.LineCommand)
				VectorCommandAddCommand(builder, commandOffset)
				offsets.append(VectorCommandEnd(builder))
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

				BezierCommandStart(builder)
				BezierCommandAddControl1(builder, CreateVector2f(builder, control1[0], control1[1]))
				BezierCommandAddControl2(builder, CreateVector2f(builder, control2[0], control2[1]))
				BezierCommandAddEnd(builder, CreateVector2f(builder, end[0], end[1]))
				commandOffset = BezierCommandEnd(builder)

				VectorCommandStart(builder)
				VectorCommandAddCommandType(builder, VectorCommandUnion.BezierCommand)
				VectorCommandAddCommand(builder, commandOffset)
				offsets.append(VectorCommandEnd(builder))

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

				QuadraticCommandStart(builder)
				QuadraticCommandAddControl(builder, CreateVector2f(builder, control[0], control[1]))
				QuadraticCommandAddEnd(builder, CreateVector2f(builder, end[0], end[1]))
				commandOffset = QuadraticCommandEnd(builder)

				VectorCommandStart(builder)
				VectorCommandAddCommandType(builder, VectorCommandUnion.QuadraticCommand)
				VectorCommandAddCommand(builder, commandOffset)
				offsets.append(VectorCommandEnd(builder))

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

				ArcCommandStart(builder)
				ArcCommandAddRadius(builder, CreateVector2f(builder, radius[0], radius[1]))
				ArcCommandAddRotation(builder, rotation)
				ArcCommandAddLargeArc(builder, largeArc)
				ArcCommandAddClockwise(builder, sweep)
				ArcCommandAddEnd(builder, CreateVector2f(builder, end[0], end[1]))
				commandOffset = ArcCommandEnd(builder)

				VectorCommandStart(builder)
				VectorCommandAddCommandType(builder, VectorCommandUnion.ArcCommand)
				VectorCommandAddCommand(builder, commandOffset)
				offsets.append(VectorCommandEnd(builder))

				pos = end
			elif command == 'b' or command == 'B':
				raise Exception('Bearing currently not implemented. ' \
					'It is generally not implemented by other SVG renderers either.')
		elif tokens[index] == 'z' or tokens[index] == 'Z':
			ClosePathCommandStart(builder)
			commandOffset = ClosePathCommandEnd(builder)

			VectorCommandStart(builder)
			VectorCommandAddCommandType(builder, VectorCommandUnion.ClosePathCommand)
			VectorCommandAddCommand(builder, commandOffset)
			offsets.append(VectorCommandEnd(builder))

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
	offsets = writeStartPath(builder, transform)

	RectangleCommandStart(builder)
	RectangleCommandAddUpperLeft(builder, CreateVector2f(builder, upperLeft[0], upperLeft[1]))
	RectangleCommandAddLowerRight(builder, CreateVector2f(builder, upperLeft[0] + rectSize[0],
		upperLeft[1] + rectSize[1]))
	RectangleCommandAddCornerRadius(builder, CreateVector2f(builder, radius[0], radius[1]))
	commandOffset = RectangleCommandEnd(builder)

	VectorCommandStart(builder)
	VectorCommandAddCommandType(builder, VectorCommandUnion.RectangleCommand)
	VectorCommandAddCommand(builder, commandOffset)
	offsets.append(VectorCommandEnd(builder))
	
	offsets.extend(style.write(builder))
	return offsets

def convertSVG(streamOrPath, outputFile):
	"""
	Loads an SVG and converts it to a DeepSea vector image FlatBuffer format.

	streamOrPath: the stream or path for the SVG file.
	outputFile: The path to the file to output to.
	"""
	svg = minidom.parse(streamOrPath)
	name = os.path.splitext(os.path.basename(outputFile))[0]
	materials = Materials(name)

	commands = []
	for rootNode in svg.childNodes:
		if rootNode.tagName == 'svg':
			size = (sizeFromString(rootNode.getAttribute('width'), 0.0),
				sizeFromString(rootNode.getAttribute('height'), 0.0))
			diagonalSize = math.sqrt(size[0]*size[0] + size[1]*size[1])/math.sqrt(2)
			for node in rootNode.childNodes:
				if node.nodeType != xml.dom.Node.ELEMENT_NODE:
					continue

				# Materials
				if node.tagName == 'linearGradient':
					gradient = LinearGradientMaterial(node, size)
					materials.addLinearGradient(gradient)
				if node.tagName == 'radialGradient':
					gradient = RadialGradientMaterial(node, size, diagonalSize)
					materials.addRadialGradient(gradient)
				# Shapes
				elif node.tagName == 'circle':
					commands.append(lambda builder,
						transform = transformFromNode(node),
						style = Style(node, materials, diagonalSize),
						center = (sizeFromString(node.getAttribute('cx'), size[0]),
							sizeFromString(node.getAttribute('cy'), size[1])),
						radius = sizeFromString(node.getAttribute('r'), diagonalSize):
						writeEllipse(builder, transform, style, center, (radius, radius)))
				elif node.tagName == 'ellipse':
					commands.append(lambda builder,
						transform = transformFromNode(node),
						style = Style(node, materials, diagonalSize),
						center = (sizeFromString(node.getAttribute('cx'), size[0]),
							sizeFromString(node.getAttribute('cy'), size[1])),
						radius = (sizeFromString(node.getAttribute('rx'), diagonalSize),
							sizeFromString(node.getAttribute('ry'), diagonalSize)):
						writeEllipse(builder, transform, style, center, radius))
				elif node.tagName == 'image':
					commands.append(lambda builder,
						transform = transformFromNode(node),
						style = Style(node, materials, diagonalSize),
						upperLeft = (sizeFromString(node.getAttribute('x'), size[0]),
							sizeFromString(node.getAttribute('y'), size[1])),
						imageSize = (sizeFromString(node.getAttribute('width'), size[0]),
							sizeFromString(node.getAttribute('height'), size[1])),
						location = node.getAttribute('href'):
						writeImage(builder, transform, style, upperLeft, imageSize, location))
				elif node.tagName == 'line':
					commands.append(lambda builder,
						transform = transformFromNode(node),
						style = Style(node, materials, diagonalSize),
						start = (sizeFromString(node.getAttribute('x1'), size[0]),
							sizeFromString(node.getAttribute('y1'), size[1])),
						end = (sizeFromString(node.getAttribute('x2'), size[0]),
							sizeFromString(node.getAttribute('y2'), size[1])):
						writeLines(builder, transform, style, [start, end]))
				elif node.tagName == 'path':
					commands.append(lambda builder,
						transform = transformFromNode(node),
						style = Style(node, materials, diagonalSize),
						path = node.getAttribute('d'):
						writePath(builder, transform, style, path, size, diagonalSize))
				elif node.tagName == 'polygon':
					commands.append(lambda builder,
						transform = transformFromNode(node),
						style = Style(node, materials, diagonalSize),
						points = node.getAttribute('points'):
						writePolygon(builder, transform, style, points, size))
				elif node.tagName == 'polyline':
					commands.append(lambda builder,
						transform = transformFromNode(node),
						style = Style(node, materials, diagonalSize),
						points = node.getAttribute('points'):
						writePolyline(builder, transform, style, points, size))
				elif node.tagName == 'rect':
					commands.append(lambda builder,
						transform = transformFromNode(node),
						style = Style(node, materials, diagonalSize),
						upperLeft = (sizeFromString(node.getAttribute('x'), size[0]),
							sizeFromString(node.getAttribute('y'), size[1])),
						rectSize = (sizeFromString(node.getAttribute('width'), size[0]),
							sizeFromString(node.getAttribute('height'), size[1])),
						radius = (sizeFromString(node.getAttribute('rx'), size[0]),
							sizeFromString(node.getAttribute('ry'), size[1])) \
							if node.hasAttribute('rx') else (0.0, 0.0):
						writeRectangle(builder, transform, style, upperLeft, rectSize, radius))
						
		break
	
	builder = flatbuffers.Builder(0)

	materials.write(builder)

	commandOffsets = []
	for command in commands:
		commandOffsets.extend(command(builder))

	VectorImageStartCommandsVector(builder, len(commandOffsets))
	for offset in reversed(commandOffsets):
		builder.PrependUOffsetTRelative(offset)
	commandsOffset = builder.EndVector(len(commandOffsets))

	VectorImageStart(builder)
	materials.writeToVectorImage(builder)
	VectorImageAddCommands(builder, commandsOffset)
	VectorImageAddSize(builder, CreateVector2f(builder, size[0], size[1]))
	builder.Finish(VectorImageEnd(builder))

	with open(outputFile, 'wb') as f:
		f.write(builder.Output())

if __name__ == '__main__':
	# Avoid parsing issues due to locale.
	locale.setlocale(locale.LC_ALL, 'C')

	parser = argparse.ArgumentParser(description =
		'Convert an SVG to a vector image to be used by Deep Sea.')
	parser.add_argument('-i', '--input', required = True,
		help = 'input svg to convert')
	parser.add_argument('-o', '--output', required = True,
		help = 'output file name, typically with the extension ".dsvi"')

	args = parser.parse_args()
	convertSVG(args.input, args.output)
