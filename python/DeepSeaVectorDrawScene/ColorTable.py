# automatically generated by the FlatBuffers compiler, do not modify

# namespace: DeepSeaVectorDrawScene

import flatbuffers
from flatbuffers.compat import import_numpy
np = import_numpy()

class ColorTable(object):
    __slots__ = ['_tab']

    @classmethod
    def GetRootAs(cls, buf, offset=0):
        n = flatbuffers.encode.Get(flatbuffers.packer.uoffset, buf, offset)
        x = ColorTable()
        x.Init(buf, n + offset)
        return x

    @classmethod
    def GetRootAsColorTable(cls, buf, offset=0):
        """This method is deprecated. Please switch to GetRootAs."""
        return cls.GetRootAs(buf, offset)
    # ColorTable
    def Init(self, buf, pos):
        self._tab = flatbuffers.table.Table(buf, pos)

    # ColorTable
    def Red(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(4))
        if o != 0:
            return self._tab.Get(flatbuffers.number_types.Uint8Flags, o + self._tab.Pos)
        return 0

    # ColorTable
    def Green(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(6))
        if o != 0:
            return self._tab.Get(flatbuffers.number_types.Uint8Flags, o + self._tab.Pos)
        return 0

    # ColorTable
    def Blue(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(8))
        if o != 0:
            return self._tab.Get(flatbuffers.number_types.Uint8Flags, o + self._tab.Pos)
        return 0

    # ColorTable
    def Alpha(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(10))
        if o != 0:
            return self._tab.Get(flatbuffers.number_types.Uint8Flags, o + self._tab.Pos)
        return 0

def Start(builder): builder.StartObject(4)
def ColorTableStart(builder):
    """This method is deprecated. Please switch to Start."""
    return Start(builder)
def AddRed(builder, red): builder.PrependUint8Slot(0, red, 0)
def ColorTableAddRed(builder, red):
    """This method is deprecated. Please switch to AddRed."""
    return AddRed(builder, red)
def AddGreen(builder, green): builder.PrependUint8Slot(1, green, 0)
def ColorTableAddGreen(builder, green):
    """This method is deprecated. Please switch to AddGreen."""
    return AddGreen(builder, green)
def AddBlue(builder, blue): builder.PrependUint8Slot(2, blue, 0)
def ColorTableAddBlue(builder, blue):
    """This method is deprecated. Please switch to AddBlue."""
    return AddBlue(builder, blue)
def AddAlpha(builder, alpha): builder.PrependUint8Slot(3, alpha, 0)
def ColorTableAddAlpha(builder, alpha):
    """This method is deprecated. Please switch to AddAlpha."""
    return AddAlpha(builder, alpha)
def End(builder): return builder.EndObject()
def ColorTableEnd(builder):
    """This method is deprecated. Please switch to End."""
    return End(builder)