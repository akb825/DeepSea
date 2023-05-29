# automatically generated by the FlatBuffers compiler, do not modify

# namespace: DeepSeaScene

import flatbuffers
from flatbuffers.compat import import_numpy
np = import_numpy()

class Surface(object):
    __slots__ = ['_tab']

    @classmethod
    def GetRootAs(cls, buf, offset=0):
        n = flatbuffers.encode.Get(flatbuffers.packer.uoffset, buf, offset)
        x = Surface()
        x.Init(buf, n + offset)
        return x

    @classmethod
    def GetRootAsSurface(cls, buf, offset=0):
        """This method is deprecated. Please switch to GetRootAs."""
        return cls.GetRootAs(buf, offset)
    # Surface
    def Init(self, buf, pos):
        self._tab = flatbuffers.table.Table(buf, pos)

    # Surface
    def Name(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(4))
        if o != 0:
            return self._tab.String(o + self._tab.Pos)
        return None

    # Surface
    def Type(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(6))
        if o != 0:
            return self._tab.Get(flatbuffers.number_types.Uint8Flags, o + self._tab.Pos)
        return 0

    # Surface
    def Usage(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(8))
        if o != 0:
            return self._tab.Get(flatbuffers.number_types.Uint32Flags, o + self._tab.Pos)
        return 0

    # Surface
    def MemoryHints(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(10))
        if o != 0:
            return self._tab.Get(flatbuffers.number_types.Uint32Flags, o + self._tab.Pos)
        return 0

    # Surface
    def Format(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(12))
        if o != 0:
            return self._tab.Get(flatbuffers.number_types.Uint8Flags, o + self._tab.Pos)
        return 0

    # Surface
    def Decoration(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(14))
        if o != 0:
            return self._tab.Get(flatbuffers.number_types.Uint8Flags, o + self._tab.Pos)
        return 0

    # Surface
    def Dimension(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(16))
        if o != 0:
            return self._tab.Get(flatbuffers.number_types.Uint8Flags, o + self._tab.Pos)
        return 0

    # Surface
    def Width(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(18))
        if o != 0:
            return self._tab.Get(flatbuffers.number_types.Uint32Flags, o + self._tab.Pos)
        return 0

    # Surface
    def WidthRatio(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(20))
        if o != 0:
            return self._tab.Get(flatbuffers.number_types.Float32Flags, o + self._tab.Pos)
        return 0.0

    # Surface
    def Height(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(22))
        if o != 0:
            return self._tab.Get(flatbuffers.number_types.Uint32Flags, o + self._tab.Pos)
        return 0

    # Surface
    def HeightRatio(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(24))
        if o != 0:
            return self._tab.Get(flatbuffers.number_types.Float32Flags, o + self._tab.Pos)
        return 0.0

    # Surface
    def Depth(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(26))
        if o != 0:
            return self._tab.Get(flatbuffers.number_types.Uint32Flags, o + self._tab.Pos)
        return 0

    # Surface
    def MipLevels(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(28))
        if o != 0:
            return self._tab.Get(flatbuffers.number_types.Uint32Flags, o + self._tab.Pos)
        return 0

    # Surface
    def Samples(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(30))
        if o != 0:
            return self._tab.Get(flatbuffers.number_types.Uint32Flags, o + self._tab.Pos)
        return 0

    # Surface
    def Resolve(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(32))
        if o != 0:
            return bool(self._tab.Get(flatbuffers.number_types.BoolFlags, o + self._tab.Pos))
        return False

    # Surface
    def WindowFramebuffer(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(34))
        if o != 0:
            return bool(self._tab.Get(flatbuffers.number_types.BoolFlags, o + self._tab.Pos))
        return False

def SurfaceStart(builder):
    builder.StartObject(16)

def Start(builder):
    SurfaceStart(builder)

def SurfaceAddName(builder, name):
    builder.PrependUOffsetTRelativeSlot(0, flatbuffers.number_types.UOffsetTFlags.py_type(name), 0)

def AddName(builder, name):
    SurfaceAddName(builder, name)

def SurfaceAddType(builder, type):
    builder.PrependUint8Slot(1, type, 0)

def AddType(builder, type):
    SurfaceAddType(builder, type)

def SurfaceAddUsage(builder, usage):
    builder.PrependUint32Slot(2, usage, 0)

def AddUsage(builder, usage):
    SurfaceAddUsage(builder, usage)

def SurfaceAddMemoryHints(builder, memoryHints):
    builder.PrependUint32Slot(3, memoryHints, 0)

def AddMemoryHints(builder, memoryHints):
    SurfaceAddMemoryHints(builder, memoryHints)

def SurfaceAddFormat(builder, format):
    builder.PrependUint8Slot(4, format, 0)

def AddFormat(builder, format):
    SurfaceAddFormat(builder, format)

def SurfaceAddDecoration(builder, decoration):
    builder.PrependUint8Slot(5, decoration, 0)

def AddDecoration(builder, decoration):
    SurfaceAddDecoration(builder, decoration)

def SurfaceAddDimension(builder, dimension):
    builder.PrependUint8Slot(6, dimension, 0)

def AddDimension(builder, dimension):
    SurfaceAddDimension(builder, dimension)

def SurfaceAddWidth(builder, width):
    builder.PrependUint32Slot(7, width, 0)

def AddWidth(builder, width):
    SurfaceAddWidth(builder, width)

def SurfaceAddWidthRatio(builder, widthRatio):
    builder.PrependFloat32Slot(8, widthRatio, 0.0)

def AddWidthRatio(builder, widthRatio):
    SurfaceAddWidthRatio(builder, widthRatio)

def SurfaceAddHeight(builder, height):
    builder.PrependUint32Slot(9, height, 0)

def AddHeight(builder, height):
    SurfaceAddHeight(builder, height)

def SurfaceAddHeightRatio(builder, heightRatio):
    builder.PrependFloat32Slot(10, heightRatio, 0.0)

def AddHeightRatio(builder, heightRatio):
    SurfaceAddHeightRatio(builder, heightRatio)

def SurfaceAddDepth(builder, depth):
    builder.PrependUint32Slot(11, depth, 0)

def AddDepth(builder, depth):
    SurfaceAddDepth(builder, depth)

def SurfaceAddMipLevels(builder, mipLevels):
    builder.PrependUint32Slot(12, mipLevels, 0)

def AddMipLevels(builder, mipLevels):
    SurfaceAddMipLevels(builder, mipLevels)

def SurfaceAddSamples(builder, samples):
    builder.PrependUint32Slot(13, samples, 0)

def AddSamples(builder, samples):
    SurfaceAddSamples(builder, samples)

def SurfaceAddResolve(builder, resolve):
    builder.PrependBoolSlot(14, resolve, 0)

def AddResolve(builder, resolve):
    SurfaceAddResolve(builder, resolve)

def SurfaceAddWindowFramebuffer(builder, windowFramebuffer):
    builder.PrependBoolSlot(15, windowFramebuffer, 0)

def AddWindowFramebuffer(builder, windowFramebuffer):
    SurfaceAddWindowFramebuffer(builder, windowFramebuffer)

def SurfaceEnd(builder):
    return builder.EndObject()

def End(builder):
    return SurfaceEnd(builder)
