# automatically generated by the FlatBuffers compiler, do not modify

# namespace: DeepSeaVectorDraw

import flatbuffers
from flatbuffers.compat import import_numpy
np = import_numpy()

class Font(object):
    __slots__ = ['_tab']

    @classmethod
    def GetRootAs(cls, buf, offset=0):
        n = flatbuffers.encode.Get(flatbuffers.packer.uoffset, buf, offset)
        x = Font()
        x.Init(buf, n + offset)
        return x

    @classmethod
    def GetRootAsFont(cls, buf, offset=0):
        """This method is deprecated. Please switch to GetRootAs."""
        return cls.GetRootAs(buf, offset)
    # Font
    def Init(self, buf, pos):
        self._tab = flatbuffers.table.Table(buf, pos)

    # Font
    def Name(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(4))
        if o != 0:
            return self._tab.String(o + self._tab.Pos)
        return None

    # Font
    def FaceGroup(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(6))
        if o != 0:
            return self._tab.String(o + self._tab.Pos)
        return None

    # Font
    def Faces(self, j):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(8))
        if o != 0:
            a = self._tab.Vector(o)
            return self._tab.String(a + flatbuffers.number_types.UOffsetTFlags.py_type(j * 4))
        return ""

    # Font
    def FacesLength(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(8))
        if o != 0:
            return self._tab.VectorLen(o)
        return 0

    # Font
    def FacesIsNone(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(8))
        return o == 0

    # Font
    def Quality(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(10))
        if o != 0:
            return self._tab.Get(flatbuffers.number_types.Uint8Flags, o + self._tab.Pos)
        return 0

    # Font
    def CacheSize(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(12))
        if o != 0:
            return self._tab.Get(flatbuffers.number_types.Uint8Flags, o + self._tab.Pos)
        return 0

def FontStart(builder):
    builder.StartObject(5)

def Start(builder):
    FontStart(builder)

def FontAddName(builder, name):
    builder.PrependUOffsetTRelativeSlot(0, flatbuffers.number_types.UOffsetTFlags.py_type(name), 0)

def AddName(builder, name):
    FontAddName(builder, name)

def FontAddFaceGroup(builder, faceGroup):
    builder.PrependUOffsetTRelativeSlot(1, flatbuffers.number_types.UOffsetTFlags.py_type(faceGroup), 0)

def AddFaceGroup(builder, faceGroup):
    FontAddFaceGroup(builder, faceGroup)

def FontAddFaces(builder, faces):
    builder.PrependUOffsetTRelativeSlot(2, flatbuffers.number_types.UOffsetTFlags.py_type(faces), 0)

def AddFaces(builder, faces):
    FontAddFaces(builder, faces)

def FontStartFacesVector(builder, numElems):
    return builder.StartVector(4, numElems, 4)

def StartFacesVector(builder, numElems):
    return FontStartFacesVector(builder, numElems)

def FontAddQuality(builder, quality):
    builder.PrependUint8Slot(3, quality, 0)

def AddQuality(builder, quality):
    FontAddQuality(builder, quality)

def FontAddCacheSize(builder, cacheSize):
    builder.PrependUint8Slot(4, cacheSize, 0)

def AddCacheSize(builder, cacheSize):
    FontAddCacheSize(builder, cacheSize)

def FontEnd(builder):
    return builder.EndObject()

def End(builder):
    return FontEnd(builder)
