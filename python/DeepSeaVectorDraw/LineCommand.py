# automatically generated by the FlatBuffers compiler, do not modify

# namespace: DeepSeaVectorDraw

import flatbuffers
from flatbuffers.compat import import_numpy
np = import_numpy()

class LineCommand(object):
    __slots__ = ['_tab']

    @classmethod
    def GetRootAs(cls, buf, offset=0):
        n = flatbuffers.encode.Get(flatbuffers.packer.uoffset, buf, offset)
        x = LineCommand()
        x.Init(buf, n + offset)
        return x

    @classmethod
    def GetRootAsLineCommand(cls, buf, offset=0):
        """This method is deprecated. Please switch to GetRootAs."""
        return cls.GetRootAs(buf, offset)
    # LineCommand
    def Init(self, buf, pos):
        self._tab = flatbuffers.table.Table(buf, pos)

    # LineCommand
    def End(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(4))
        if o != 0:
            x = o + self._tab.Pos
            from DeepSeaVectorDraw.Vector2f import Vector2f
            obj = Vector2f()
            obj.Init(self._tab.Bytes, x)
            return obj
        return None

def LineCommandStart(builder): builder.StartObject(1)
def Start(builder):
    return LineCommandStart(builder)
def LineCommandAddEnd(builder, end): builder.PrependStructSlot(0, flatbuffers.number_types.UOffsetTFlags.py_type(end), 0)
def AddEnd(builder, end):
    return LineCommandAddEnd(builder, end)
def LineCommandEnd(builder): return builder.EndObject()
def End(builder):
    return LineCommandEnd(builder)