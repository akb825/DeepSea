# automatically generated by the FlatBuffers compiler, do not modify

# namespace: DeepSeaVectorDraw

import flatbuffers

class FillPathCommand(object):
    __slots__ = ['_tab']

    @classmethod
    def GetRootAsFillPathCommand(cls, buf, offset):
        n = flatbuffers.encode.Get(flatbuffers.packer.uoffset, buf, offset)
        x = FillPathCommand()
        x.Init(buf, n + offset)
        return x

    # FillPathCommand
    def Init(self, buf, pos):
        self._tab = flatbuffers.table.Table(buf, pos)

    # FillPathCommand
    def Material(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(4))
        if o != 0:
            return self._tab.String(o + self._tab.Pos)
        return None

    # FillPathCommand
    def Opacity(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(6))
        if o != 0:
            return self._tab.Get(flatbuffers.number_types.Float32Flags, o + self._tab.Pos)
        return 0.0

    # FillPathCommand
    def FillRule(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(8))
        if o != 0:
            return self._tab.Get(flatbuffers.number_types.Uint8Flags, o + self._tab.Pos)
        return 0

def FillPathCommandStart(builder): builder.StartObject(3)
def FillPathCommandAddMaterial(builder, material): builder.PrependUOffsetTRelativeSlot(0, flatbuffers.number_types.UOffsetTFlags.py_type(material), 0)
def FillPathCommandAddOpacity(builder, opacity): builder.PrependFloat32Slot(1, opacity, 0.0)
def FillPathCommandAddFillRule(builder, fillRule): builder.PrependUint8Slot(2, fillRule, 0)
def FillPathCommandEnd(builder): return builder.EndObject()
