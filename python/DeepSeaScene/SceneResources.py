# automatically generated by the FlatBuffers compiler, do not modify

# namespace: DeepSeaScene

import flatbuffers
from flatbuffers.compat import import_numpy
np = import_numpy()

class SceneResources(object):
    __slots__ = ['_tab']

    @classmethod
    def GetRootAs(cls, buf, offset=0):
        n = flatbuffers.encode.Get(flatbuffers.packer.uoffset, buf, offset)
        x = SceneResources()
        x.Init(buf, n + offset)
        return x

    @classmethod
    def GetRootAsSceneResources(cls, buf, offset=0):
        """This method is deprecated. Please switch to GetRootAs."""
        return cls.GetRootAs(buf, offset)
    # SceneResources
    def Init(self, buf, pos):
        self._tab = flatbuffers.table.Table(buf, pos)

    # SceneResources
    def Resources(self, j):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(4))
        if o != 0:
            x = self._tab.Vector(o)
            x += flatbuffers.number_types.UOffsetTFlags.py_type(j) * 4
            x = self._tab.Indirect(x)
            from DeepSeaScene.SceneResource import SceneResource
            obj = SceneResource()
            obj.Init(self._tab.Bytes, x)
            return obj
        return None

    # SceneResources
    def ResourcesLength(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(4))
        if o != 0:
            return self._tab.VectorLen(o)
        return 0

    # SceneResources
    def ResourcesIsNone(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(4))
        return o == 0

def SceneResourcesStart(builder):
    builder.StartObject(1)

def Start(builder):
    SceneResourcesStart(builder)

def SceneResourcesAddResources(builder, resources):
    builder.PrependUOffsetTRelativeSlot(0, flatbuffers.number_types.UOffsetTFlags.py_type(resources), 0)

def AddResources(builder, resources):
    SceneResourcesAddResources(builder, resources)

def SceneResourcesStartResourcesVector(builder, numElems):
    return builder.StartVector(4, numElems, 4)

def StartResourcesVector(builder, numElems):
    return SceneResourcesStartResourcesVector(builder, numElems)

def SceneResourcesEnd(builder):
    return builder.EndObject()

def End(builder):
    return SceneResourcesEnd(builder)
