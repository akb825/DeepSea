# automatically generated by the FlatBuffers compiler, do not modify

# namespace: DeepSeaScene

import flatbuffers
from flatbuffers.compat import import_numpy
np = import_numpy()

class ViewMipmapList(object):
    __slots__ = ['_tab']

    @classmethod
    def GetRootAs(cls, buf, offset=0):
        n = flatbuffers.encode.Get(flatbuffers.packer.uoffset, buf, offset)
        x = ViewMipmapList()
        x.Init(buf, n + offset)
        return x

    @classmethod
    def GetRootAsViewMipmapList(cls, buf, offset=0):
        """This method is deprecated. Please switch to GetRootAs."""
        return cls.GetRootAs(buf, offset)
    # ViewMipmapList
    def Init(self, buf, pos):
        self._tab = flatbuffers.table.Table(buf, pos)

    # ViewMipmapList
    def Textures(self, j):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(4))
        if o != 0:
            a = self._tab.Vector(o)
            return self._tab.String(a + flatbuffers.number_types.UOffsetTFlags.py_type(j * 4))
        return ""

    # ViewMipmapList
    def TexturesLength(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(4))
        if o != 0:
            return self._tab.VectorLen(o)
        return 0

    # ViewMipmapList
    def TexturesIsNone(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(4))
        return o == 0

def ViewMipmapListStart(builder):
    builder.StartObject(1)

def Start(builder):
    ViewMipmapListStart(builder)

def ViewMipmapListAddTextures(builder, textures):
    builder.PrependUOffsetTRelativeSlot(0, flatbuffers.number_types.UOffsetTFlags.py_type(textures), 0)

def AddTextures(builder, textures):
    ViewMipmapListAddTextures(builder, textures)

def ViewMipmapListStartTexturesVector(builder, numElems):
    return builder.StartVector(4, numElems, 4)

def StartTexturesVector(builder, numElems):
    return ViewMipmapListStartTexturesVector(builder, numElems)

def ViewMipmapListEnd(builder):
    return builder.EndObject()

def End(builder):
    return ViewMipmapListEnd(builder)
