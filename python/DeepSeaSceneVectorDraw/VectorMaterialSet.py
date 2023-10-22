# automatically generated by the FlatBuffers compiler, do not modify

# namespace: DeepSeaSceneVectorDraw

import flatbuffers
from flatbuffers.compat import import_numpy
np = import_numpy()

class VectorMaterialSet(object):
    __slots__ = ['_tab']

    @classmethod
    def GetRootAs(cls, buf, offset=0):
        n = flatbuffers.encode.Get(flatbuffers.packer.uoffset, buf, offset)
        x = VectorMaterialSet()
        x.Init(buf, n + offset)
        return x

    @classmethod
    def GetRootAsVectorMaterialSet(cls, buf, offset=0):
        """This method is deprecated. Please switch to GetRootAs."""
        return cls.GetRootAs(buf, offset)
    # VectorMaterialSet
    def Init(self, buf, pos):
        self._tab = flatbuffers.table.Table(buf, pos)

    # VectorMaterialSet
    def Materials(self, j):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(4))
        if o != 0:
            x = self._tab.Vector(o)
            x += flatbuffers.number_types.UOffsetTFlags.py_type(j) * 4
            x = self._tab.Indirect(x)
            from DeepSeaSceneVectorDraw.Material import Material
            obj = Material()
            obj.Init(self._tab.Bytes, x)
            return obj
        return None

    # VectorMaterialSet
    def MaterialsLength(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(4))
        if o != 0:
            return self._tab.VectorLen(o)
        return 0

    # VectorMaterialSet
    def MaterialsIsNone(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(4))
        return o == 0

    # VectorMaterialSet
    def Srgb(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(6))
        if o != 0:
            return bool(self._tab.Get(flatbuffers.number_types.BoolFlags, o + self._tab.Pos))
        return False

def VectorMaterialSetStart(builder):
    builder.StartObject(2)

def Start(builder):
    VectorMaterialSetStart(builder)

def VectorMaterialSetAddMaterials(builder, materials):
    builder.PrependUOffsetTRelativeSlot(0, flatbuffers.number_types.UOffsetTFlags.py_type(materials), 0)

def AddMaterials(builder, materials):
    VectorMaterialSetAddMaterials(builder, materials)

def VectorMaterialSetStartMaterialsVector(builder, numElems):
    return builder.StartVector(4, numElems, 4)

def StartMaterialsVector(builder, numElems: int) -> int:
    return VectorMaterialSetStartMaterialsVector(builder, numElems)

def VectorMaterialSetAddSrgb(builder, srgb):
    builder.PrependBoolSlot(1, srgb, 0)

def AddSrgb(builder, srgb):
    VectorMaterialSetAddSrgb(builder, srgb)

def VectorMaterialSetEnd(builder):
    return builder.EndObject()

def End(builder):
    return VectorMaterialSetEnd(builder)
