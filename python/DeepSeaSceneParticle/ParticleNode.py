# automatically generated by the FlatBuffers compiler, do not modify

# namespace: DeepSeaSceneParticle

import flatbuffers
from flatbuffers.compat import import_numpy
np = import_numpy()

class ParticleNode(object):
    __slots__ = ['_tab']

    @classmethod
    def GetRootAs(cls, buf, offset=0):
        n = flatbuffers.encode.Get(flatbuffers.packer.uoffset, buf, offset)
        x = ParticleNode()
        x.Init(buf, n + offset)
        return x

    @classmethod
    def GetRootAsParticleNode(cls, buf, offset=0):
        """This method is deprecated. Please switch to GetRootAs."""
        return cls.GetRootAs(buf, offset)
    # ParticleNode
    def Init(self, buf, pos):
        self._tab = flatbuffers.table.Table(buf, pos)

    # ParticleNode
    def ParticleEmitterFactory(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(4))
        if o != 0:
            return self._tab.String(o + self._tab.Pos)
        return None

    # ParticleNode
    def ItemLists(self, j):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(6))
        if o != 0:
            a = self._tab.Vector(o)
            return self._tab.String(a + flatbuffers.number_types.UOffsetTFlags.py_type(j * 4))
        return ""

    # ParticleNode
    def ItemListsLength(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(6))
        if o != 0:
            return self._tab.VectorLen(o)
        return 0

    # ParticleNode
    def ItemListsIsNone(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(6))
        return o == 0

def ParticleNodeStart(builder):
    builder.StartObject(2)

def Start(builder):
    ParticleNodeStart(builder)

def ParticleNodeAddParticleEmitterFactory(builder, particleEmitterFactory):
    builder.PrependUOffsetTRelativeSlot(0, flatbuffers.number_types.UOffsetTFlags.py_type(particleEmitterFactory), 0)

def AddParticleEmitterFactory(builder, particleEmitterFactory):
    ParticleNodeAddParticleEmitterFactory(builder, particleEmitterFactory)

def ParticleNodeAddItemLists(builder, itemLists):
    builder.PrependUOffsetTRelativeSlot(1, flatbuffers.number_types.UOffsetTFlags.py_type(itemLists), 0)

def AddItemLists(builder, itemLists):
    ParticleNodeAddItemLists(builder, itemLists)

def ParticleNodeStartItemListsVector(builder, numElems):
    return builder.StartVector(4, numElems, 4)

def StartItemListsVector(builder, numElems: int) -> int:
    return ParticleNodeStartItemListsVector(builder, numElems)

def ParticleNodeEnd(builder):
    return builder.EndObject()

def End(builder):
    return ParticleNodeEnd(builder)
