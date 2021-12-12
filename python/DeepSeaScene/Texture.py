# automatically generated by the FlatBuffers compiler, do not modify

# namespace: DeepSeaScene

import flatbuffers
from flatbuffers.compat import import_numpy
np = import_numpy()

class Texture(object):
    __slots__ = ['_tab']

    @classmethod
    def GetRootAs(cls, buf, offset=0):
        n = flatbuffers.encode.Get(flatbuffers.packer.uoffset, buf, offset)
        x = Texture()
        x.Init(buf, n + offset)
        return x

    @classmethod
    def GetRootAsTexture(cls, buf, offset=0):
        """This method is deprecated. Please switch to GetRootAs."""
        return cls.GetRootAs(buf, offset)
    # Texture
    def Init(self, buf, pos):
        self._tab = flatbuffers.table.Table(buf, pos)

    # Texture
    def Name(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(4))
        if o != 0:
            return self._tab.String(o + self._tab.Pos)
        return None

    # Texture
    def Usage(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(6))
        if o != 0:
            return self._tab.Get(flatbuffers.number_types.Uint32Flags, o + self._tab.Pos)
        return 0

    # Texture
    def MemoryHints(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(8))
        if o != 0:
            return self._tab.Get(flatbuffers.number_types.Uint32Flags, o + self._tab.Pos)
        return 0

    # Texture
    def DataType(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(10))
        if o != 0:
            return self._tab.Get(flatbuffers.number_types.Uint8Flags, o + self._tab.Pos)
        return 0

    # Texture
    def Data(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(12))
        if o != 0:
            from flatbuffers.table import Table
            obj = Table(bytearray(), 0)
            self._tab.Union(obj, o)
            return obj
        return None

    # Texture
    def TextureInfo(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(14))
        if o != 0:
            x = self._tab.Indirect(o + self._tab.Pos)
            from DeepSeaScene.TextureInfo import TextureInfo
            obj = TextureInfo()
            obj.Init(self._tab.Bytes, x)
            return obj
        return None

def TextureStart(builder): builder.StartObject(6)
def Start(builder):
    return TextureStart(builder)
def TextureAddName(builder, name): builder.PrependUOffsetTRelativeSlot(0, flatbuffers.number_types.UOffsetTFlags.py_type(name), 0)
def AddName(builder, name):
    return TextureAddName(builder, name)
def TextureAddUsage(builder, usage): builder.PrependUint32Slot(1, usage, 0)
def AddUsage(builder, usage):
    return TextureAddUsage(builder, usage)
def TextureAddMemoryHints(builder, memoryHints): builder.PrependUint32Slot(2, memoryHints, 0)
def AddMemoryHints(builder, memoryHints):
    return TextureAddMemoryHints(builder, memoryHints)
def TextureAddDataType(builder, dataType): builder.PrependUint8Slot(3, dataType, 0)
def AddDataType(builder, dataType):
    return TextureAddDataType(builder, dataType)
def TextureAddData(builder, data): builder.PrependUOffsetTRelativeSlot(4, flatbuffers.number_types.UOffsetTFlags.py_type(data), 0)
def AddData(builder, data):
    return TextureAddData(builder, data)
def TextureAddTextureInfo(builder, textureInfo): builder.PrependUOffsetTRelativeSlot(5, flatbuffers.number_types.UOffsetTFlags.py_type(textureInfo), 0)
def AddTextureInfo(builder, textureInfo):
    return TextureAddTextureInfo(builder, textureInfo)
def TextureEnd(builder): return builder.EndObject()
def End(builder):
    return TextureEnd(builder)