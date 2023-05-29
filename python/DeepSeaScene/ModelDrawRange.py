# automatically generated by the FlatBuffers compiler, do not modify

# namespace: DeepSeaScene

import flatbuffers
from flatbuffers.compat import import_numpy
np = import_numpy()

class ModelDrawRange(object):
    __slots__ = ['_tab']

    @classmethod
    def GetRootAs(cls, buf, offset=0):
        n = flatbuffers.encode.Get(flatbuffers.packer.uoffset, buf, offset)
        x = ModelDrawRange()
        x.Init(buf, n + offset)
        return x

    @classmethod
    def GetRootAsModelDrawRange(cls, buf, offset=0):
        """This method is deprecated. Please switch to GetRootAs."""
        return cls.GetRootAs(buf, offset)
    # ModelDrawRange
    def Init(self, buf, pos):
        self._tab = flatbuffers.table.Table(buf, pos)

    # ModelDrawRange
    def DrawRangeType(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(4))
        if o != 0:
            return self._tab.Get(flatbuffers.number_types.Uint8Flags, o + self._tab.Pos)
        return 0

    # ModelDrawRange
    def DrawRange(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(6))
        if o != 0:
            from flatbuffers.table import Table
            obj = Table(bytearray(), 0)
            self._tab.Union(obj, o)
            return obj
        return None

def ModelDrawRangeStart(builder):
    builder.StartObject(2)

def Start(builder):
    ModelDrawRangeStart(builder)

def ModelDrawRangeAddDrawRangeType(builder, drawRangeType):
    builder.PrependUint8Slot(0, drawRangeType, 0)

def AddDrawRangeType(builder, drawRangeType):
    ModelDrawRangeAddDrawRangeType(builder, drawRangeType)

def ModelDrawRangeAddDrawRange(builder, drawRange):
    builder.PrependUOffsetTRelativeSlot(1, flatbuffers.number_types.UOffsetTFlags.py_type(drawRange), 0)

def AddDrawRange(builder, drawRange):
    ModelDrawRangeAddDrawRange(builder, drawRange)

def ModelDrawRangeEnd(builder):
    return builder.EndObject()

def End(builder):
    return ModelDrawRangeEnd(builder)
