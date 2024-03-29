# automatically generated by the FlatBuffers compiler, do not modify

# namespace: DeepSeaScene

import flatbuffers
from flatbuffers.compat import import_numpy
np = import_numpy()

class TextureInfo(object):
    __slots__ = ['_tab']

    @classmethod
    def GetRootAs(cls, buf, offset=0):
        n = flatbuffers.encode.Get(flatbuffers.packer.uoffset, buf, offset)
        x = TextureInfo()
        x.Init(buf, n + offset)
        return x

    @classmethod
    def GetRootAsTextureInfo(cls, buf, offset=0):
        """This method is deprecated. Please switch to GetRootAs."""
        return cls.GetRootAs(buf, offset)
    # TextureInfo
    def Init(self, buf, pos):
        self._tab = flatbuffers.table.Table(buf, pos)

    # TextureInfo
    def Format(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(4))
        if o != 0:
            return self._tab.Get(flatbuffers.number_types.Uint8Flags, o + self._tab.Pos)
        return 0

    # TextureInfo
    def Decoration(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(6))
        if o != 0:
            return self._tab.Get(flatbuffers.number_types.Uint8Flags, o + self._tab.Pos)
        return 0

    # TextureInfo
    def Dimension(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(8))
        if o != 0:
            return self._tab.Get(flatbuffers.number_types.Uint8Flags, o + self._tab.Pos)
        return 0

    # TextureInfo
    def Width(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(10))
        if o != 0:
            return self._tab.Get(flatbuffers.number_types.Uint32Flags, o + self._tab.Pos)
        return 0

    # TextureInfo
    def Height(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(12))
        if o != 0:
            return self._tab.Get(flatbuffers.number_types.Uint32Flags, o + self._tab.Pos)
        return 0

    # TextureInfo
    def Depth(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(14))
        if o != 0:
            return self._tab.Get(flatbuffers.number_types.Uint32Flags, o + self._tab.Pos)
        return 0

    # TextureInfo
    def MipLevels(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(16))
        if o != 0:
            return self._tab.Get(flatbuffers.number_types.Uint8Flags, o + self._tab.Pos)
        return 0

def TextureInfoStart(builder):
    builder.StartObject(7)

def Start(builder):
    TextureInfoStart(builder)

def TextureInfoAddFormat(builder, format):
    builder.PrependUint8Slot(0, format, 0)

def AddFormat(builder, format):
    TextureInfoAddFormat(builder, format)

def TextureInfoAddDecoration(builder, decoration):
    builder.PrependUint8Slot(1, decoration, 0)

def AddDecoration(builder, decoration):
    TextureInfoAddDecoration(builder, decoration)

def TextureInfoAddDimension(builder, dimension):
    builder.PrependUint8Slot(2, dimension, 0)

def AddDimension(builder, dimension):
    TextureInfoAddDimension(builder, dimension)

def TextureInfoAddWidth(builder, width):
    builder.PrependUint32Slot(3, width, 0)

def AddWidth(builder, width):
    TextureInfoAddWidth(builder, width)

def TextureInfoAddHeight(builder, height):
    builder.PrependUint32Slot(4, height, 0)

def AddHeight(builder, height):
    TextureInfoAddHeight(builder, height)

def TextureInfoAddDepth(builder, depth):
    builder.PrependUint32Slot(5, depth, 0)

def AddDepth(builder, depth):
    TextureInfoAddDepth(builder, depth)

def TextureInfoAddMipLevels(builder, mipLevels):
    builder.PrependUint8Slot(6, mipLevels, 0)

def AddMipLevels(builder, mipLevels):
    TextureInfoAddMipLevels(builder, mipLevels)

def TextureInfoEnd(builder):
    return builder.EndObject()

def End(builder):
    return TextureInfoEnd(builder)
