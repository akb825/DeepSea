# automatically generated by the FlatBuffers compiler, do not modify

# namespace: DeepSeaScene

import flatbuffers
from flatbuffers.compat import import_numpy
np = import_numpy()

class ClearDepthStencil(object):
    __slots__ = ['_tab']

    @classmethod
    def GetRootAs(cls, buf, offset=0):
        n = flatbuffers.encode.Get(flatbuffers.packer.uoffset, buf, offset)
        x = ClearDepthStencil()
        x.Init(buf, n + offset)
        return x

    @classmethod
    def GetRootAsClearDepthStencil(cls, buf, offset=0):
        """This method is deprecated. Please switch to GetRootAs."""
        return cls.GetRootAs(buf, offset)
    # ClearDepthStencil
    def Init(self, buf, pos):
        self._tab = flatbuffers.table.Table(buf, pos)

    # ClearDepthStencil
    def Depth(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(4))
        if o != 0:
            return self._tab.Get(flatbuffers.number_types.Float32Flags, o + self._tab.Pos)
        return 0.0

    # ClearDepthStencil
    def Stencil(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(6))
        if o != 0:
            return self._tab.Get(flatbuffers.number_types.Uint32Flags, o + self._tab.Pos)
        return 0

def ClearDepthStencilStart(builder):
    builder.StartObject(2)

def Start(builder):
    ClearDepthStencilStart(builder)

def ClearDepthStencilAddDepth(builder, depth):
    builder.PrependFloat32Slot(0, depth, 0.0)

def AddDepth(builder, depth):
    ClearDepthStencilAddDepth(builder, depth)

def ClearDepthStencilAddStencil(builder, stencil):
    builder.PrependUint32Slot(1, stencil, 0)

def AddStencil(builder, stencil):
    ClearDepthStencilAddStencil(builder, stencil)

def ClearDepthStencilEnd(builder):
    return builder.EndObject()

def End(builder):
    return ClearDepthStencilEnd(builder)
