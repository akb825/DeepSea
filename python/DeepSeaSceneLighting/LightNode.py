# automatically generated by the FlatBuffers compiler, do not modify

# namespace: DeepSeaSceneLighting

import flatbuffers
from flatbuffers.compat import import_numpy
np = import_numpy()

class LightNode(object):
    __slots__ = ['_tab']

    @classmethod
    def GetRootAs(cls, buf, offset=0):
        n = flatbuffers.encode.Get(flatbuffers.packer.uoffset, buf, offset)
        x = LightNode()
        x.Init(buf, n + offset)
        return x

    @classmethod
    def GetRootAsLightNode(cls, buf, offset=0):
        """This method is deprecated. Please switch to GetRootAs."""
        return cls.GetRootAs(buf, offset)
    # LightNode
    def Init(self, buf, pos):
        self._tab = flatbuffers.table.Table(buf, pos)

    # LightNode
    def TemplateLightType(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(4))
        if o != 0:
            return self._tab.Get(flatbuffers.number_types.Uint8Flags, o + self._tab.Pos)
        return 0

    # LightNode
    def TemplateLight(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(6))
        if o != 0:
            from flatbuffers.table import Table
            obj = Table(bytearray(), 0)
            self._tab.Union(obj, o)
            return obj
        return None

    # LightNode
    def LightBaseName(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(8))
        if o != 0:
            return self._tab.String(o + self._tab.Pos)
        return None

    # LightNode
    def SingleInstance(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(10))
        if o != 0:
            return bool(self._tab.Get(flatbuffers.number_types.BoolFlags, o + self._tab.Pos))
        return False

    # LightNode
    def ItemLists(self, j):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(12))
        if o != 0:
            a = self._tab.Vector(o)
            return self._tab.String(a + flatbuffers.number_types.UOffsetTFlags.py_type(j * 4))
        return ""

    # LightNode
    def ItemListsLength(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(12))
        if o != 0:
            return self._tab.VectorLen(o)
        return 0

    # LightNode
    def ItemListsIsNone(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(12))
        return o == 0

def LightNodeStart(builder):
    builder.StartObject(5)

def Start(builder):
    LightNodeStart(builder)

def LightNodeAddTemplateLightType(builder, templateLightType):
    builder.PrependUint8Slot(0, templateLightType, 0)

def AddTemplateLightType(builder, templateLightType):
    LightNodeAddTemplateLightType(builder, templateLightType)

def LightNodeAddTemplateLight(builder, templateLight):
    builder.PrependUOffsetTRelativeSlot(1, flatbuffers.number_types.UOffsetTFlags.py_type(templateLight), 0)

def AddTemplateLight(builder, templateLight):
    LightNodeAddTemplateLight(builder, templateLight)

def LightNodeAddLightBaseName(builder, lightBaseName):
    builder.PrependUOffsetTRelativeSlot(2, flatbuffers.number_types.UOffsetTFlags.py_type(lightBaseName), 0)

def AddLightBaseName(builder, lightBaseName):
    LightNodeAddLightBaseName(builder, lightBaseName)

def LightNodeAddSingleInstance(builder, singleInstance):
    builder.PrependBoolSlot(3, singleInstance, 0)

def AddSingleInstance(builder, singleInstance):
    LightNodeAddSingleInstance(builder, singleInstance)

def LightNodeAddItemLists(builder, itemLists):
    builder.PrependUOffsetTRelativeSlot(4, flatbuffers.number_types.UOffsetTFlags.py_type(itemLists), 0)

def AddItemLists(builder, itemLists):
    LightNodeAddItemLists(builder, itemLists)

def LightNodeStartItemListsVector(builder, numElems):
    return builder.StartVector(4, numElems, 4)

def StartItemListsVector(builder, numElems):
    return LightNodeStartItemListsVector(builder, numElems)

def LightNodeEnd(builder):
    return builder.EndObject()

def End(builder):
    return LightNodeEnd(builder)
