# automatically generated by the FlatBuffers compiler, do not modify

# namespace: DeepSeaVectorDraw

import flatbuffers
from flatbuffers.compat import import_numpy
np = import_numpy()

class StrokePathCommand(object):
    __slots__ = ['_tab']

    @classmethod
    def GetRootAsStrokePathCommand(cls, buf, offset):
        n = flatbuffers.encode.Get(flatbuffers.packer.uoffset, buf, offset)
        x = StrokePathCommand()
        x.Init(buf, n + offset)
        return x

    # StrokePathCommand
    def Init(self, buf, pos):
        self._tab = flatbuffers.table.Table(buf, pos)

    # StrokePathCommand
    def Material(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(4))
        if o != 0:
            return self._tab.String(o + self._tab.Pos)
        return None

    # StrokePathCommand
    def Opacity(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(6))
        if o != 0:
            return self._tab.Get(flatbuffers.number_types.Float32Flags, o + self._tab.Pos)
        return 0.0

    # StrokePathCommand
    def JoinType(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(8))
        if o != 0:
            return self._tab.Get(flatbuffers.number_types.Uint8Flags, o + self._tab.Pos)
        return 0

    # StrokePathCommand
    def CapType(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(10))
        if o != 0:
            return self._tab.Get(flatbuffers.number_types.Uint8Flags, o + self._tab.Pos)
        return 0

    # StrokePathCommand
    def Width(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(12))
        if o != 0:
            return self._tab.Get(flatbuffers.number_types.Float32Flags, o + self._tab.Pos)
        return 0.0

    # StrokePathCommand
    def MiterLimit(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(14))
        if o != 0:
            return self._tab.Get(flatbuffers.number_types.Float32Flags, o + self._tab.Pos)
        return 0.0

    # StrokePathCommand
    def DashArray(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(16))
        if o != 0:
            x = o + self._tab.Pos
            from DeepSeaVectorDraw.DashArray import DashArray
            obj = DashArray()
            obj.Init(self._tab.Bytes, x)
            return obj
        return None

def StrokePathCommandStart(builder): builder.StartObject(7)
def StrokePathCommandAddMaterial(builder, material): builder.PrependUOffsetTRelativeSlot(0, flatbuffers.number_types.UOffsetTFlags.py_type(material), 0)
def StrokePathCommandAddOpacity(builder, opacity): builder.PrependFloat32Slot(1, opacity, 0.0)
def StrokePathCommandAddJoinType(builder, joinType): builder.PrependUint8Slot(2, joinType, 0)
def StrokePathCommandAddCapType(builder, capType): builder.PrependUint8Slot(3, capType, 0)
def StrokePathCommandAddWidth(builder, width): builder.PrependFloat32Slot(4, width, 0.0)
def StrokePathCommandAddMiterLimit(builder, miterLimit): builder.PrependFloat32Slot(5, miterLimit, 0.0)
def StrokePathCommandAddDashArray(builder, dashArray): builder.PrependStructSlot(6, flatbuffers.number_types.UOffsetTFlags.py_type(dashArray), 0)
def StrokePathCommandEnd(builder): return builder.EndObject()
