# automatically generated by the FlatBuffers compiler, do not modify

# namespace: DeepSeaSceneVectorDraw

import flatbuffers
from flatbuffers.compat import import_numpy
np = import_numpy()

class VectorResourceRef(object):
    __slots__ = ['_tab']

    @classmethod
    def GetRootAs(cls, buf, offset=0):
        n = flatbuffers.encode.Get(flatbuffers.packer.uoffset, buf, offset)
        x = VectorResourceRef()
        x.Init(buf, n + offset)
        return x

    @classmethod
    def GetRootAsVectorResourceRef(cls, buf, offset=0):
        """This method is deprecated. Please switch to GetRootAs."""
        return cls.GetRootAs(buf, offset)
    # VectorResourceRef
    def Init(self, buf, pos):
        self._tab = flatbuffers.table.Table(buf, pos)

    # VectorResourceRef
    def Resources(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(4))
        if o != 0:
            return self._tab.String(o + self._tab.Pos)
        return None

    # VectorResourceRef
    def Name(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(6))
        if o != 0:
            return self._tab.String(o + self._tab.Pos)
        return None

def VectorResourceRefStart(builder): builder.StartObject(2)
def Start(builder):
    return VectorResourceRefStart(builder)
def VectorResourceRefAddResources(builder, resources): builder.PrependUOffsetTRelativeSlot(0, flatbuffers.number_types.UOffsetTFlags.py_type(resources), 0)
def AddResources(builder, resources):
    return VectorResourceRefAddResources(builder, resources)
def VectorResourceRefAddName(builder, name): builder.PrependUOffsetTRelativeSlot(1, flatbuffers.number_types.UOffsetTFlags.py_type(name), 0)
def AddName(builder, name):
    return VectorResourceRefAddName(builder, name)
def VectorResourceRefEnd(builder): return builder.EndObject()
def End(builder):
    return VectorResourceRefEnd(builder)