# automatically generated by the FlatBuffers compiler, do not modify

# namespace: DeepSeaSceneVectorDraw

import flatbuffers
from flatbuffers.compat import import_numpy
np = import_numpy()

class TextNode(object):
    __slots__ = ['_tab']

    @classmethod
    def GetRootAs(cls, buf, offset=0):
        n = flatbuffers.encode.Get(flatbuffers.packer.uoffset, buf, offset)
        x = TextNode()
        x.Init(buf, n + offset)
        return x

    @classmethod
    def GetRootAsTextNode(cls, buf, offset=0):
        """This method is deprecated. Please switch to GetRootAs."""
        return cls.GetRootAs(buf, offset)
    # TextNode
    def Init(self, buf, pos):
        self._tab = flatbuffers.table.Table(buf, pos)

    # TextNode
    def EmbeddedResources(self, j):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(4))
        if o != 0:
            a = self._tab.Vector(o)
            return self._tab.Get(flatbuffers.number_types.Uint8Flags, a + flatbuffers.number_types.UOffsetTFlags.py_type(j * 1))
        return 0

    # TextNode
    def EmbeddedResourcesAsNumpy(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(4))
        if o != 0:
            return self._tab.GetVectorAsNumpy(flatbuffers.number_types.Uint8Flags, o)
        return 0

    # TextNode
    def EmbeddedResourcesLength(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(4))
        if o != 0:
            return self._tab.VectorLen(o)
        return 0

    # TextNode
    def EmbeddedResourcesIsNone(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(4))
        return o == 0

    # TextNode
    def Text(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(6))
        if o != 0:
            return self._tab.String(o + self._tab.Pos)
        return None

    # TextNode
    def Alignment(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(8))
        if o != 0:
            return self._tab.Get(flatbuffers.number_types.Uint8Flags, o + self._tab.Pos)
        return 0

    # TextNode
    def MaxWidth(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(10))
        if o != 0:
            return self._tab.Get(flatbuffers.number_types.Float32Flags, o + self._tab.Pos)
        return 0.0

    # TextNode
    def LineScale(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(12))
        if o != 0:
            return self._tab.Get(flatbuffers.number_types.Float32Flags, o + self._tab.Pos)
        return 0.0

    # TextNode
    def Z(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(14))
        if o != 0:
            return self._tab.Get(flatbuffers.number_types.Int32Flags, o + self._tab.Pos)
        return 0

    # TextNode
    def FirstChar(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(16))
        if o != 0:
            return self._tab.Get(flatbuffers.number_types.Uint32Flags, o + self._tab.Pos)
        return 0

    # TextNode
    def CharCount(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(18))
        if o != 0:
            return self._tab.Get(flatbuffers.number_types.Uint32Flags, o + self._tab.Pos)
        return 0

    # TextNode
    def Shader(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(20))
        if o != 0:
            return self._tab.String(o + self._tab.Pos)
        return None

    # TextNode
    def Material(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(22))
        if o != 0:
            return self._tab.String(o + self._tab.Pos)
        return None

    # TextNode
    def FontTexture(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(24))
        if o != 0:
            return self._tab.String(o + self._tab.Pos)
        return None

    # TextNode
    def ItemLists(self, j):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(26))
        if o != 0:
            a = self._tab.Vector(o)
            return self._tab.String(a + flatbuffers.number_types.UOffsetTFlags.py_type(j * 4))
        return ""

    # TextNode
    def ItemListsLength(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(26))
        if o != 0:
            return self._tab.VectorLen(o)
        return 0

    # TextNode
    def ItemListsIsNone(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(26))
        return o == 0

def TextNodeStart(builder): builder.StartObject(12)
def Start(builder):
    return TextNodeStart(builder)
def TextNodeAddEmbeddedResources(builder, embeddedResources): builder.PrependUOffsetTRelativeSlot(0, flatbuffers.number_types.UOffsetTFlags.py_type(embeddedResources), 0)
def AddEmbeddedResources(builder, embeddedResources):
    return TextNodeAddEmbeddedResources(builder, embeddedResources)
def TextNodeStartEmbeddedResourcesVector(builder, numElems): return builder.StartVector(1, numElems, 1)
def StartEmbeddedResourcesVector(builder, numElems):
    return TextNodeStartEmbeddedResourcesVector(builder, numElems)
def TextNodeAddText(builder, text): builder.PrependUOffsetTRelativeSlot(1, flatbuffers.number_types.UOffsetTFlags.py_type(text), 0)
def AddText(builder, text):
    return TextNodeAddText(builder, text)
def TextNodeAddAlignment(builder, alignment): builder.PrependUint8Slot(2, alignment, 0)
def AddAlignment(builder, alignment):
    return TextNodeAddAlignment(builder, alignment)
def TextNodeAddMaxWidth(builder, maxWidth): builder.PrependFloat32Slot(3, maxWidth, 0.0)
def AddMaxWidth(builder, maxWidth):
    return TextNodeAddMaxWidth(builder, maxWidth)
def TextNodeAddLineScale(builder, lineScale): builder.PrependFloat32Slot(4, lineScale, 0.0)
def AddLineScale(builder, lineScale):
    return TextNodeAddLineScale(builder, lineScale)
def TextNodeAddZ(builder, z): builder.PrependInt32Slot(5, z, 0)
def AddZ(builder, z):
    return TextNodeAddZ(builder, z)
def TextNodeAddFirstChar(builder, firstChar): builder.PrependUint32Slot(6, firstChar, 0)
def AddFirstChar(builder, firstChar):
    return TextNodeAddFirstChar(builder, firstChar)
def TextNodeAddCharCount(builder, charCount): builder.PrependUint32Slot(7, charCount, 0)
def AddCharCount(builder, charCount):
    return TextNodeAddCharCount(builder, charCount)
def TextNodeAddShader(builder, shader): builder.PrependUOffsetTRelativeSlot(8, flatbuffers.number_types.UOffsetTFlags.py_type(shader), 0)
def AddShader(builder, shader):
    return TextNodeAddShader(builder, shader)
def TextNodeAddMaterial(builder, material): builder.PrependUOffsetTRelativeSlot(9, flatbuffers.number_types.UOffsetTFlags.py_type(material), 0)
def AddMaterial(builder, material):
    return TextNodeAddMaterial(builder, material)
def TextNodeAddFontTexture(builder, fontTexture): builder.PrependUOffsetTRelativeSlot(10, flatbuffers.number_types.UOffsetTFlags.py_type(fontTexture), 0)
def AddFontTexture(builder, fontTexture):
    return TextNodeAddFontTexture(builder, fontTexture)
def TextNodeAddItemLists(builder, itemLists): builder.PrependUOffsetTRelativeSlot(11, flatbuffers.number_types.UOffsetTFlags.py_type(itemLists), 0)
def AddItemLists(builder, itemLists):
    return TextNodeAddItemLists(builder, itemLists)
def TextNodeStartItemListsVector(builder, numElems): return builder.StartVector(4, numElems, 4)
def StartItemListsVector(builder, numElems):
    return TextNodeStartItemListsVector(builder, numElems)
def TextNodeEnd(builder): return builder.EndObject()
def End(builder):
    return TextNodeEnd(builder)