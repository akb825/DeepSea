# automatically generated by the FlatBuffers compiler, do not modify

# namespace: DeepSeaSceneLighting

import flatbuffers
from flatbuffers.compat import import_numpy
np = import_numpy()

class SceneShadowManagerPrepare(object):
    __slots__ = ['_tab']

    @classmethod
    def GetRootAs(cls, buf, offset=0):
        n = flatbuffers.encode.Get(flatbuffers.packer.uoffset, buf, offset)
        x = SceneShadowManagerPrepare()
        x.Init(buf, n + offset)
        return x

    @classmethod
    def GetRootAsSceneShadowManagerPrepare(cls, buf, offset=0):
        """This method is deprecated. Please switch to GetRootAs."""
        return cls.GetRootAs(buf, offset)
    # SceneShadowManagerPrepare
    def Init(self, buf, pos):
        self._tab = flatbuffers.table.Table(buf, pos)

    # SceneShadowManagerPrepare
    def ShadowManager(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(4))
        if o != 0:
            return self._tab.String(o + self._tab.Pos)
        return None

def SceneShadowManagerPrepareStart(builder):
    builder.StartObject(1)

def Start(builder):
    SceneShadowManagerPrepareStart(builder)

def SceneShadowManagerPrepareAddShadowManager(builder, shadowManager):
    builder.PrependUOffsetTRelativeSlot(0, flatbuffers.number_types.UOffsetTFlags.py_type(shadowManager), 0)

def AddShadowManager(builder, shadowManager):
    SceneShadowManagerPrepareAddShadowManager(builder, shadowManager)

def SceneShadowManagerPrepareEnd(builder):
    return builder.EndObject()

def End(builder):
    return SceneShadowManagerPrepareEnd(builder)
