# automatically generated by the FlatBuffers compiler, do not modify

# namespace: DeepSeaVectorDraw

import flatbuffers
from flatbuffers.compat import import_numpy
np = import_numpy()

class FaceGroup(object):
    __slots__ = ['_tab']

    @classmethod
    def GetRootAs(cls, buf, offset=0):
        n = flatbuffers.encode.Get(flatbuffers.packer.uoffset, buf, offset)
        x = FaceGroup()
        x.Init(buf, n + offset)
        return x

    @classmethod
    def GetRootAsFaceGroup(cls, buf, offset=0):
        """This method is deprecated. Please switch to GetRootAs."""
        return cls.GetRootAs(buf, offset)
    # FaceGroup
    def Init(self, buf, pos):
        self._tab = flatbuffers.table.Table(buf, pos)

    # FaceGroup
    def Name(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(4))
        if o != 0:
            return self._tab.String(o + self._tab.Pos)
        return None

    # FaceGroup
    def Faces(self, j):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(6))
        if o != 0:
            x = self._tab.Vector(o)
            x += flatbuffers.number_types.UOffsetTFlags.py_type(j) * 4
            x = self._tab.Indirect(x)
            from DeepSeaVectorDraw.Resource import Resource
            obj = Resource()
            obj.Init(self._tab.Bytes, x)
            return obj
        return None

    # FaceGroup
    def FacesLength(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(6))
        if o != 0:
            return self._tab.VectorLen(o)
        return 0

    # FaceGroup
    def FacesIsNone(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(6))
        return o == 0

def FaceGroupStart(builder): builder.StartObject(2)
def Start(builder):
    return FaceGroupStart(builder)
def FaceGroupAddName(builder, name): builder.PrependUOffsetTRelativeSlot(0, flatbuffers.number_types.UOffsetTFlags.py_type(name), 0)
def AddName(builder, name):
    return FaceGroupAddName(builder, name)
def FaceGroupAddFaces(builder, faces): builder.PrependUOffsetTRelativeSlot(1, flatbuffers.number_types.UOffsetTFlags.py_type(faces), 0)
def AddFaces(builder, faces):
    return FaceGroupAddFaces(builder, faces)
def FaceGroupStartFacesVector(builder, numElems): return builder.StartVector(4, numElems, 4)
def StartFacesVector(builder, numElems):
    return FaceGroupStartFacesVector(builder, numElems)
def FaceGroupEnd(builder): return builder.EndObject()
def End(builder):
    return FaceGroupEnd(builder)