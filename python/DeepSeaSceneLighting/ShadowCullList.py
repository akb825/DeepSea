# automatically generated by the FlatBuffers compiler, do not modify

# namespace: DeepSeaSceneLighting

import flatbuffers
from flatbuffers.compat import import_numpy
np = import_numpy()

class ShadowCullList(object):
    __slots__ = ['_tab']

    @classmethod
    def GetRootAs(cls, buf, offset=0):
        n = flatbuffers.encode.Get(flatbuffers.packer.uoffset, buf, offset)
        x = ShadowCullList()
        x.Init(buf, n + offset)
        return x

    @classmethod
    def GetRootAsShadowCullList(cls, buf, offset=0):
        """This method is deprecated. Please switch to GetRootAs."""
        return cls.GetRootAs(buf, offset)
    # ShadowCullList
    def Init(self, buf, pos):
        self._tab = flatbuffers.table.Table(buf, pos)

    # ShadowCullList
    def ShadowManager(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(4))
        if o != 0:
            return self._tab.String(o + self._tab.Pos)
        return None

    # ShadowCullList
    def Shadows(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(6))
        if o != 0:
            return self._tab.String(o + self._tab.Pos)
        return None

    # ShadowCullList
    def Surface(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(8))
        if o != 0:
            return self._tab.Get(flatbuffers.number_types.Uint8Flags, o + self._tab.Pos)
        return 0

def ShadowCullListStart(builder):
    builder.StartObject(3)

def Start(builder):
    ShadowCullListStart(builder)

def ShadowCullListAddShadowManager(builder, shadowManager):
    builder.PrependUOffsetTRelativeSlot(0, flatbuffers.number_types.UOffsetTFlags.py_type(shadowManager), 0)

def AddShadowManager(builder, shadowManager):
    ShadowCullListAddShadowManager(builder, shadowManager)

def ShadowCullListAddShadows(builder, shadows):
    builder.PrependUOffsetTRelativeSlot(1, flatbuffers.number_types.UOffsetTFlags.py_type(shadows), 0)

def AddShadows(builder, shadows):
    ShadowCullListAddShadows(builder, shadows)

def ShadowCullListAddSurface(builder, surface):
    builder.PrependUint8Slot(2, surface, 0)

def AddSurface(builder, surface):
    ShadowCullListAddSurface(builder, surface)

def ShadowCullListEnd(builder):
    return builder.EndObject()

def End(builder):
    return ShadowCullListEnd(builder)
