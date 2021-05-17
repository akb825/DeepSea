# automatically generated by the FlatBuffers compiler, do not modify

# namespace: DeepSeaScene

import flatbuffers
from flatbuffers.compat import import_numpy
np = import_numpy()

class ModelList(object):
    __slots__ = ['_tab']

    @classmethod
    def GetRootAs(cls, buf, offset=0):
        n = flatbuffers.encode.Get(flatbuffers.packer.uoffset, buf, offset)
        x = ModelList()
        x.Init(buf, n + offset)
        return x

    @classmethod
    def GetRootAsModelList(cls, buf, offset=0):
        """This method is deprecated. Please switch to GetRootAs."""
        return cls.GetRootAs(buf, offset)
    # ModelList
    def Init(self, buf, pos):
        self._tab = flatbuffers.table.Table(buf, pos)

    # ModelList
    def InstanceData(self, j):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(4))
        if o != 0:
            x = self._tab.Vector(o)
            x += flatbuffers.number_types.UOffsetTFlags.py_type(j) * 4
            x = self._tab.Indirect(x)
            from DeepSeaScene.ObjectData import ObjectData
            obj = ObjectData()
            obj.Init(self._tab.Bytes, x)
            return obj
        return None

    # ModelList
    def InstanceDataLength(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(4))
        if o != 0:
            return self._tab.VectorLen(o)
        return 0

    # ModelList
    def InstanceDataIsNone(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(4))
        return o == 0

    # ModelList
    def SortType(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(6))
        if o != 0:
            return self._tab.Get(flatbuffers.number_types.Uint8Flags, o + self._tab.Pos)
        return 0

    # ModelList
    def DynamicRenderStates(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(8))
        if o != 0:
            x = self._tab.Indirect(o + self._tab.Pos)
            from DeepSeaScene.DynamicRenderStates import DynamicRenderStates
            obj = DynamicRenderStates()
            obj.Init(self._tab.Bytes, x)
            return obj
        return None

    # ModelList
    def CullList(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(10))
        if o != 0:
            return self._tab.String(o + self._tab.Pos)
        return None

def Start(builder): builder.StartObject(4)
def ModelListStart(builder):
    """This method is deprecated. Please switch to Start."""
    return Start(builder)
def AddInstanceData(builder, instanceData): builder.PrependUOffsetTRelativeSlot(0, flatbuffers.number_types.UOffsetTFlags.py_type(instanceData), 0)
def ModelListAddInstanceData(builder, instanceData):
    """This method is deprecated. Please switch to AddInstanceData."""
    return AddInstanceData(builder, instanceData)
def StartInstanceDataVector(builder, numElems): return builder.StartVector(4, numElems, 4)
def ModelListStartInstanceDataVector(builder, numElems):
    """This method is deprecated. Please switch to Start."""
    return StartInstanceDataVector(builder, numElems)
def AddSortType(builder, sortType): builder.PrependUint8Slot(1, sortType, 0)
def ModelListAddSortType(builder, sortType):
    """This method is deprecated. Please switch to AddSortType."""
    return AddSortType(builder, sortType)
def AddDynamicRenderStates(builder, dynamicRenderStates): builder.PrependUOffsetTRelativeSlot(2, flatbuffers.number_types.UOffsetTFlags.py_type(dynamicRenderStates), 0)
def ModelListAddDynamicRenderStates(builder, dynamicRenderStates):
    """This method is deprecated. Please switch to AddDynamicRenderStates."""
    return AddDynamicRenderStates(builder, dynamicRenderStates)
def AddCullList(builder, cullList): builder.PrependUOffsetTRelativeSlot(3, flatbuffers.number_types.UOffsetTFlags.py_type(cullList), 0)
def ModelListAddCullList(builder, cullList):
    """This method is deprecated. Please switch to AddCullList."""
    return AddCullList(builder, cullList)
def End(builder): return builder.EndObject()
def ModelListEnd(builder):
    """This method is deprecated. Please switch to End."""
    return End(builder)