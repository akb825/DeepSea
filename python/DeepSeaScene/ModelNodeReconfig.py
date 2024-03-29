# automatically generated by the FlatBuffers compiler, do not modify

# namespace: DeepSeaScene

import flatbuffers
from flatbuffers.compat import import_numpy
np = import_numpy()

class ModelNodeReconfig(object):
    __slots__ = ['_tab']

    @classmethod
    def GetRootAs(cls, buf, offset=0):
        n = flatbuffers.encode.Get(flatbuffers.packer.uoffset, buf, offset)
        x = ModelNodeReconfig()
        x.Init(buf, n + offset)
        return x

    @classmethod
    def GetRootAsModelNodeReconfig(cls, buf, offset=0):
        """This method is deprecated. Please switch to GetRootAs."""
        return cls.GetRootAs(buf, offset)
    # ModelNodeReconfig
    def Init(self, buf, pos):
        self._tab = flatbuffers.table.Table(buf, pos)

    # ModelNodeReconfig
    def Name(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(4))
        if o != 0:
            return self._tab.String(o + self._tab.Pos)
        return None

    # ModelNodeReconfig
    def Models(self, j):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(6))
        if o != 0:
            x = self._tab.Vector(o)
            x += flatbuffers.number_types.UOffsetTFlags.py_type(j) * 4
            x = self._tab.Indirect(x)
            from DeepSeaScene.ModelReconfig import ModelReconfig
            obj = ModelReconfig()
            obj.Init(self._tab.Bytes, x)
            return obj
        return None

    # ModelNodeReconfig
    def ModelsLength(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(6))
        if o != 0:
            return self._tab.VectorLen(o)
        return 0

    # ModelNodeReconfig
    def ModelsIsNone(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(6))
        return o == 0

    # ModelNodeReconfig
    def ExtraItemLists(self, j):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(8))
        if o != 0:
            a = self._tab.Vector(o)
            return self._tab.String(a + flatbuffers.number_types.UOffsetTFlags.py_type(j * 4))
        return ""

    # ModelNodeReconfig
    def ExtraItemListsLength(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(8))
        if o != 0:
            return self._tab.VectorLen(o)
        return 0

    # ModelNodeReconfig
    def ExtraItemListsIsNone(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(8))
        return o == 0

def ModelNodeReconfigStart(builder):
    builder.StartObject(3)

def Start(builder):
    ModelNodeReconfigStart(builder)

def ModelNodeReconfigAddName(builder, name):
    builder.PrependUOffsetTRelativeSlot(0, flatbuffers.number_types.UOffsetTFlags.py_type(name), 0)

def AddName(builder, name):
    ModelNodeReconfigAddName(builder, name)

def ModelNodeReconfigAddModels(builder, models):
    builder.PrependUOffsetTRelativeSlot(1, flatbuffers.number_types.UOffsetTFlags.py_type(models), 0)

def AddModels(builder, models):
    ModelNodeReconfigAddModels(builder, models)

def ModelNodeReconfigStartModelsVector(builder, numElems):
    return builder.StartVector(4, numElems, 4)

def StartModelsVector(builder, numElems):
    return ModelNodeReconfigStartModelsVector(builder, numElems)

def ModelNodeReconfigAddExtraItemLists(builder, extraItemLists):
    builder.PrependUOffsetTRelativeSlot(2, flatbuffers.number_types.UOffsetTFlags.py_type(extraItemLists), 0)

def AddExtraItemLists(builder, extraItemLists):
    ModelNodeReconfigAddExtraItemLists(builder, extraItemLists)

def ModelNodeReconfigStartExtraItemListsVector(builder, numElems):
    return builder.StartVector(4, numElems, 4)

def StartExtraItemListsVector(builder, numElems):
    return ModelNodeReconfigStartExtraItemListsVector(builder, numElems)

def ModelNodeReconfigEnd(builder):
    return builder.EndObject()

def End(builder):
    return ModelNodeReconfigEnd(builder)
