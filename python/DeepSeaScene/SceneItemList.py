# automatically generated by the FlatBuffers compiler, do not modify

# namespace: DeepSeaScene

import flatbuffers
from flatbuffers.compat import import_numpy
np = import_numpy()

class SceneItemList(object):
    __slots__ = ['_tab']

    @classmethod
    def GetRootAs(cls, buf, offset=0):
        n = flatbuffers.encode.Get(flatbuffers.packer.uoffset, buf, offset)
        x = SceneItemList()
        x.Init(buf, n + offset)
        return x

    @classmethod
    def GetRootAsSceneItemList(cls, buf, offset=0):
        """This method is deprecated. Please switch to GetRootAs."""
        return cls.GetRootAs(buf, offset)
    # SceneItemList
    def Init(self, buf, pos):
        self._tab = flatbuffers.table.Table(buf, pos)

    # SceneItemList
    def Type(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(4))
        if o != 0:
            return self._tab.String(o + self._tab.Pos)
        return None

    # SceneItemList
    def Name(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(6))
        if o != 0:
            return self._tab.String(o + self._tab.Pos)
        return None

    # SceneItemList
    def Data(self, j):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(8))
        if o != 0:
            a = self._tab.Vector(o)
            return self._tab.Get(flatbuffers.number_types.Uint8Flags, a + flatbuffers.number_types.UOffsetTFlags.py_type(j * 1))
        return 0

    # SceneItemList
    def DataAsNumpy(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(8))
        if o != 0:
            return self._tab.GetVectorAsNumpy(flatbuffers.number_types.Uint8Flags, o)
        return 0

    # SceneItemList
    def DataLength(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(8))
        if o != 0:
            return self._tab.VectorLen(o)
        return 0

    # SceneItemList
    def DataIsNone(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(8))
        return o == 0

def SceneItemListStart(builder):
    builder.StartObject(3)

def Start(builder):
    SceneItemListStart(builder)

def SceneItemListAddType(builder, type):
    builder.PrependUOffsetTRelativeSlot(0, flatbuffers.number_types.UOffsetTFlags.py_type(type), 0)

def AddType(builder, type):
    SceneItemListAddType(builder, type)

def SceneItemListAddName(builder, name):
    builder.PrependUOffsetTRelativeSlot(1, flatbuffers.number_types.UOffsetTFlags.py_type(name), 0)

def AddName(builder, name):
    SceneItemListAddName(builder, name)

def SceneItemListAddData(builder, data):
    builder.PrependUOffsetTRelativeSlot(2, flatbuffers.number_types.UOffsetTFlags.py_type(data), 0)

def AddData(builder, data):
    SceneItemListAddData(builder, data)

def SceneItemListStartDataVector(builder, numElems):
    return builder.StartVector(1, numElems, 1)

def StartDataVector(builder, numElems):
    return SceneItemListStartDataVector(builder, numElems)

def SceneItemListEnd(builder):
    return builder.EndObject()

def End(builder):
    return SceneItemListEnd(builder)
