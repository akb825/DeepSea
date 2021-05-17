# automatically generated by the FlatBuffers compiler, do not modify

# namespace: DeepSeaScene

import flatbuffers
from flatbuffers.compat import import_numpy
np = import_numpy()

class FileReference(object):
    __slots__ = ['_tab']

    @classmethod
    def GetRootAs(cls, buf, offset=0):
        n = flatbuffers.encode.Get(flatbuffers.packer.uoffset, buf, offset)
        x = FileReference()
        x.Init(buf, n + offset)
        return x

    @classmethod
    def GetRootAsFileReference(cls, buf, offset=0):
        """This method is deprecated. Please switch to GetRootAs."""
        return cls.GetRootAs(buf, offset)
    # FileReference
    def Init(self, buf, pos):
        self._tab = flatbuffers.table.Table(buf, pos)

    # FileReference
    def Type(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(4))
        if o != 0:
            return self._tab.Get(flatbuffers.number_types.Uint8Flags, o + self._tab.Pos)
        return 0

    # FileReference
    def Path(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(6))
        if o != 0:
            return self._tab.String(o + self._tab.Pos)
        return None

def Start(builder): builder.StartObject(2)
def FileReferenceStart(builder):
    """This method is deprecated. Please switch to Start."""
    return Start(builder)
def AddType(builder, type): builder.PrependUint8Slot(0, type, 0)
def FileReferenceAddType(builder, type):
    """This method is deprecated. Please switch to AddType."""
    return AddType(builder, type)
def AddPath(builder, path): builder.PrependUOffsetTRelativeSlot(1, flatbuffers.number_types.UOffsetTFlags.py_type(path), 0)
def FileReferenceAddPath(builder, path):
    """This method is deprecated. Please switch to AddPath."""
    return AddPath(builder, path)
def End(builder): return builder.EndObject()
def FileReferenceEnd(builder):
    """This method is deprecated. Please switch to End."""
    return End(builder)