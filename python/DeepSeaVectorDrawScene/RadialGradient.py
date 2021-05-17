# automatically generated by the FlatBuffers compiler, do not modify

# namespace: DeepSeaVectorDrawScene

import flatbuffers
from flatbuffers.compat import import_numpy
np = import_numpy()

class RadialGradient(object):
    __slots__ = ['_tab']

    @classmethod
    def GetRootAs(cls, buf, offset=0):
        n = flatbuffers.encode.Get(flatbuffers.packer.uoffset, buf, offset)
        x = RadialGradient()
        x.Init(buf, n + offset)
        return x

    @classmethod
    def GetRootAsRadialGradient(cls, buf, offset=0):
        """This method is deprecated. Please switch to GetRootAs."""
        return cls.GetRootAs(buf, offset)
    # RadialGradient
    def Init(self, buf, pos):
        self._tab = flatbuffers.table.Table(buf, pos)

    # RadialGradient
    def Center(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(4))
        if o != 0:
            x = o + self._tab.Pos
            from DeepSeaScene.Vector2f import Vector2f
            obj = Vector2f()
            obj.Init(self._tab.Bytes, x)
            return obj
        return None

    # RadialGradient
    def Radius(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(6))
        if o != 0:
            return self._tab.Get(flatbuffers.number_types.Float32Flags, o + self._tab.Pos)
        return 0.0

    # RadialGradient
    def Focus(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(8))
        if o != 0:
            x = o + self._tab.Pos
            from DeepSeaScene.Vector2f import Vector2f
            obj = Vector2f()
            obj.Init(self._tab.Bytes, x)
            return obj
        return None

    # RadialGradient
    def FocusRadius(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(10))
        if o != 0:
            return self._tab.Get(flatbuffers.number_types.Float32Flags, o + self._tab.Pos)
        return 0.0

    # RadialGradient
    def Edge(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(12))
        if o != 0:
            return self._tab.Get(flatbuffers.number_types.Uint8Flags, o + self._tab.Pos)
        return 0

    # RadialGradient
    def CoordinateSpace(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(14))
        if o != 0:
            return self._tab.Get(flatbuffers.number_types.Uint8Flags, o + self._tab.Pos)
        return 0

    # RadialGradient
    def Transform(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(16))
        if o != 0:
            x = o + self._tab.Pos
            from DeepSeaScene.Matrix33f import Matrix33f
            obj = Matrix33f()
            obj.Init(self._tab.Bytes, x)
            return obj
        return None

    # RadialGradient
    def Stops(self, j):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(18))
        if o != 0:
            x = self._tab.Vector(o)
            x += flatbuffers.number_types.UOffsetTFlags.py_type(j) * 4
            x = self._tab.Indirect(x)
            from DeepSeaVectorDrawScene.GradientStop import GradientStop
            obj = GradientStop()
            obj.Init(self._tab.Bytes, x)
            return obj
        return None

    # RadialGradient
    def StopsLength(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(18))
        if o != 0:
            return self._tab.VectorLen(o)
        return 0

    # RadialGradient
    def StopsIsNone(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(18))
        return o == 0

def Start(builder): builder.StartObject(8)
def RadialGradientStart(builder):
    """This method is deprecated. Please switch to Start."""
    return Start(builder)
def AddCenter(builder, center): builder.PrependStructSlot(0, flatbuffers.number_types.UOffsetTFlags.py_type(center), 0)
def RadialGradientAddCenter(builder, center):
    """This method is deprecated. Please switch to AddCenter."""
    return AddCenter(builder, center)
def AddRadius(builder, radius): builder.PrependFloat32Slot(1, radius, 0.0)
def RadialGradientAddRadius(builder, radius):
    """This method is deprecated. Please switch to AddRadius."""
    return AddRadius(builder, radius)
def AddFocus(builder, focus): builder.PrependStructSlot(2, flatbuffers.number_types.UOffsetTFlags.py_type(focus), 0)
def RadialGradientAddFocus(builder, focus):
    """This method is deprecated. Please switch to AddFocus."""
    return AddFocus(builder, focus)
def AddFocusRadius(builder, focusRadius): builder.PrependFloat32Slot(3, focusRadius, 0.0)
def RadialGradientAddFocusRadius(builder, focusRadius):
    """This method is deprecated. Please switch to AddFocusRadius."""
    return AddFocusRadius(builder, focusRadius)
def AddEdge(builder, edge): builder.PrependUint8Slot(4, edge, 0)
def RadialGradientAddEdge(builder, edge):
    """This method is deprecated. Please switch to AddEdge."""
    return AddEdge(builder, edge)
def AddCoordinateSpace(builder, coordinateSpace): builder.PrependUint8Slot(5, coordinateSpace, 0)
def RadialGradientAddCoordinateSpace(builder, coordinateSpace):
    """This method is deprecated. Please switch to AddCoordinateSpace."""
    return AddCoordinateSpace(builder, coordinateSpace)
def AddTransform(builder, transform): builder.PrependStructSlot(6, flatbuffers.number_types.UOffsetTFlags.py_type(transform), 0)
def RadialGradientAddTransform(builder, transform):
    """This method is deprecated. Please switch to AddTransform."""
    return AddTransform(builder, transform)
def AddStops(builder, stops): builder.PrependUOffsetTRelativeSlot(7, flatbuffers.number_types.UOffsetTFlags.py_type(stops), 0)
def RadialGradientAddStops(builder, stops):
    """This method is deprecated. Please switch to AddStops."""
    return AddStops(builder, stops)
def StartStopsVector(builder, numElems): return builder.StartVector(4, numElems, 4)
def RadialGradientStartStopsVector(builder, numElems):
    """This method is deprecated. Please switch to Start."""
    return StartStopsVector(builder, numElems)
def End(builder): return builder.EndObject()
def RadialGradientEnd(builder):
    """This method is deprecated. Please switch to End."""
    return End(builder)