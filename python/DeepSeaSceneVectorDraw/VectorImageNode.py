# automatically generated by the FlatBuffers compiler, do not modify

# namespace: DeepSeaSceneVectorDraw

import flatbuffers
from flatbuffers.compat import import_numpy
np = import_numpy()

class VectorImageNode(object):
    __slots__ = ['_tab']

    @classmethod
    def GetRootAs(cls, buf, offset=0):
        n = flatbuffers.encode.Get(flatbuffers.packer.uoffset, buf, offset)
        x = VectorImageNode()
        x.Init(buf, n + offset)
        return x

    @classmethod
    def GetRootAsVectorImageNode(cls, buf, offset=0):
        """This method is deprecated. Please switch to GetRootAs."""
        return cls.GetRootAs(buf, offset)
    # VectorImageNode
    def Init(self, buf, pos):
        self._tab = flatbuffers.table.Table(buf, pos)

    # VectorImageNode
    def EmbeddedResources(self, j):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(4))
        if o != 0:
            a = self._tab.Vector(o)
            return self._tab.Get(flatbuffers.number_types.Uint8Flags, a + flatbuffers.number_types.UOffsetTFlags.py_type(j * 1))
        return 0

    # VectorImageNode
    def EmbeddedResourcesAsNumpy(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(4))
        if o != 0:
            return self._tab.GetVectorAsNumpy(flatbuffers.number_types.Uint8Flags, o)
        return 0

    # VectorImageNode
    def EmbeddedResourcesLength(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(4))
        if o != 0:
            return self._tab.VectorLen(o)
        return 0

    # VectorImageNode
    def EmbeddedResourcesIsNone(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(4))
        return o == 0

    # VectorImageNode
    def VectorImage(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(6))
        if o != 0:
            return self._tab.String(o + self._tab.Pos)
        return None

    # VectorImageNode
    def Size(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(8))
        if o != 0:
            x = o + self._tab.Pos
            from DeepSeaScene.Vector2f import Vector2f
            obj = Vector2f()
            obj.Init(self._tab.Bytes, x)
            return obj
        return None

    # VectorImageNode
    def Z(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(10))
        if o != 0:
            return self._tab.Get(flatbuffers.number_types.Int32Flags, o + self._tab.Pos)
        return 0

    # VectorImageNode
    def VectorShaders(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(12))
        if o != 0:
            return self._tab.String(o + self._tab.Pos)
        return None

    # VectorImageNode
    def Material(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(14))
        if o != 0:
            return self._tab.String(o + self._tab.Pos)
        return None

    # VectorImageNode
    def ItemLists(self, j):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(16))
        if o != 0:
            a = self._tab.Vector(o)
            return self._tab.String(a + flatbuffers.number_types.UOffsetTFlags.py_type(j * 4))
        return ""

    # VectorImageNode
    def ItemListsLength(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(16))
        if o != 0:
            return self._tab.VectorLen(o)
        return 0

    # VectorImageNode
    def ItemListsIsNone(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(16))
        return o == 0

def VectorImageNodeStart(builder):
    builder.StartObject(7)

def Start(builder):
    VectorImageNodeStart(builder)

def VectorImageNodeAddEmbeddedResources(builder, embeddedResources):
    builder.PrependUOffsetTRelativeSlot(0, flatbuffers.number_types.UOffsetTFlags.py_type(embeddedResources), 0)

def AddEmbeddedResources(builder, embeddedResources):
    VectorImageNodeAddEmbeddedResources(builder, embeddedResources)

def VectorImageNodeStartEmbeddedResourcesVector(builder, numElems):
    return builder.StartVector(1, numElems, 1)

def StartEmbeddedResourcesVector(builder, numElems):
    return VectorImageNodeStartEmbeddedResourcesVector(builder, numElems)

def VectorImageNodeAddVectorImage(builder, vectorImage):
    builder.PrependUOffsetTRelativeSlot(1, flatbuffers.number_types.UOffsetTFlags.py_type(vectorImage), 0)

def AddVectorImage(builder, vectorImage):
    VectorImageNodeAddVectorImage(builder, vectorImage)

def VectorImageNodeAddSize(builder, size):
    builder.PrependStructSlot(2, flatbuffers.number_types.UOffsetTFlags.py_type(size), 0)

def AddSize(builder, size):
    VectorImageNodeAddSize(builder, size)

def VectorImageNodeAddZ(builder, z):
    builder.PrependInt32Slot(3, z, 0)

def AddZ(builder, z):
    VectorImageNodeAddZ(builder, z)

def VectorImageNodeAddVectorShaders(builder, vectorShaders):
    builder.PrependUOffsetTRelativeSlot(4, flatbuffers.number_types.UOffsetTFlags.py_type(vectorShaders), 0)

def AddVectorShaders(builder, vectorShaders):
    VectorImageNodeAddVectorShaders(builder, vectorShaders)

def VectorImageNodeAddMaterial(builder, material):
    builder.PrependUOffsetTRelativeSlot(5, flatbuffers.number_types.UOffsetTFlags.py_type(material), 0)

def AddMaterial(builder, material):
    VectorImageNodeAddMaterial(builder, material)

def VectorImageNodeAddItemLists(builder, itemLists):
    builder.PrependUOffsetTRelativeSlot(6, flatbuffers.number_types.UOffsetTFlags.py_type(itemLists), 0)

def AddItemLists(builder, itemLists):
    VectorImageNodeAddItemLists(builder, itemLists)

def VectorImageNodeStartItemListsVector(builder, numElems):
    return builder.StartVector(4, numElems, 4)

def StartItemListsVector(builder, numElems):
    return VectorImageNodeStartItemListsVector(builder, numElems)

def VectorImageNodeEnd(builder):
    return builder.EndObject()

def End(builder):
    return VectorImageNodeEnd(builder)
