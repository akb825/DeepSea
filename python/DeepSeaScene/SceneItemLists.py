# automatically generated by the FlatBuffers compiler, do not modify

# namespace: DeepSeaScene

import flatbuffers
from flatbuffers.compat import import_numpy
np = import_numpy()

class SceneItemLists(object):
    __slots__ = ['_tab']

    @classmethod
    def GetRootAs(cls, buf, offset=0):
        n = flatbuffers.encode.Get(flatbuffers.packer.uoffset, buf, offset)
        x = SceneItemLists()
        x.Init(buf, n + offset)
        return x

    @classmethod
    def GetRootAsSceneItemLists(cls, buf, offset=0):
        """This method is deprecated. Please switch to GetRootAs."""
        return cls.GetRootAs(buf, offset)
    # SceneItemLists
    def Init(self, buf, pos):
        self._tab = flatbuffers.table.Table(buf, pos)

    # SceneItemLists
    def ItemLists(self, j):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(4))
        if o != 0:
            x = self._tab.Vector(o)
            x += flatbuffers.number_types.UOffsetTFlags.py_type(j) * 4
            x = self._tab.Indirect(x)
            from DeepSeaScene.SceneItemList import SceneItemList
            obj = SceneItemList()
            obj.Init(self._tab.Bytes, x)
            return obj
        return None

    # SceneItemLists
    def ItemListsLength(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(4))
        if o != 0:
            return self._tab.VectorLen(o)
        return 0

    # SceneItemLists
    def ItemListsIsNone(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(4))
        return o == 0

def SceneItemListsStart(builder):
    builder.StartObject(1)

def Start(builder):
    SceneItemListsStart(builder)

def SceneItemListsAddItemLists(builder, itemLists):
    builder.PrependUOffsetTRelativeSlot(0, flatbuffers.number_types.UOffsetTFlags.py_type(itemLists), 0)

def AddItemLists(builder, itemLists):
    SceneItemListsAddItemLists(builder, itemLists)

def SceneItemListsStartItemListsVector(builder, numElems):
    return builder.StartVector(4, numElems, 4)

def StartItemListsVector(builder, numElems: int) -> int:
    return SceneItemListsStartItemListsVector(builder, numElems)

def SceneItemListsEnd(builder):
    return builder.EndObject()

def End(builder):
    return SceneItemListsEnd(builder)
