# automatically generated by the FlatBuffers compiler, do not modify

# namespace: DeepSeaVectorDraw

import flatbuffers

class ArcCommand(object):
    __slots__ = ['_tab']

    @classmethod
    def GetRootAsArcCommand(cls, buf, offset):
        n = flatbuffers.encode.Get(flatbuffers.packer.uoffset, buf, offset)
        x = ArcCommand()
        x.Init(buf, n + offset)
        return x

    # ArcCommand
    def Init(self, buf, pos):
        self._tab = flatbuffers.table.Table(buf, pos)

    # ArcCommand
    def Radius(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(4))
        if o != 0:
            x = o + self._tab.Pos
            from .Vector2f import Vector2f
            obj = Vector2f()
            obj.Init(self._tab.Bytes, x)
            return obj
        return None

    # ArcCommand
    def Rotation(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(6))
        if o != 0:
            return self._tab.Get(flatbuffers.number_types.Float32Flags, o + self._tab.Pos)
        return 0.0

    # ArcCommand
    def LargeArc(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(8))
        if o != 0:
            return self._tab.Get(flatbuffers.number_types.BoolFlags, o + self._tab.Pos)
        return 0

    # ArcCommand
    def Clockwise(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(10))
        if o != 0:
            return self._tab.Get(flatbuffers.number_types.BoolFlags, o + self._tab.Pos)
        return 0

    # ArcCommand
    def End(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(12))
        if o != 0:
            x = o + self._tab.Pos
            from .Vector2f import Vector2f
            obj = Vector2f()
            obj.Init(self._tab.Bytes, x)
            return obj
        return None

def ArcCommandStart(builder): builder.StartObject(5)
def ArcCommandAddRadius(builder, radius): builder.PrependStructSlot(0, flatbuffers.number_types.UOffsetTFlags.py_type(radius), 0)
def ArcCommandAddRotation(builder, rotation): builder.PrependFloat32Slot(1, rotation, 0.0)
def ArcCommandAddLargeArc(builder, largeArc): builder.PrependBoolSlot(2, largeArc, 0)
def ArcCommandAddClockwise(builder, clockwise): builder.PrependBoolSlot(3, clockwise, 0)
def ArcCommandAddEnd(builder, end): builder.PrependStructSlot(4, flatbuffers.number_types.UOffsetTFlags.py_type(end), 0)
def ArcCommandEnd(builder): return builder.EndObject()