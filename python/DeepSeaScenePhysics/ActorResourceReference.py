# automatically generated by the FlatBuffers compiler, do not modify

# namespace: DeepSeaScenePhysics

import flatbuffers
from flatbuffers.compat import import_numpy
np = import_numpy()

class ActorResourceReference(object):
    __slots__ = ['_tab']

    @classmethod
    def GetRootAs(cls, buf, offset=0):
        n = flatbuffers.encode.Get(flatbuffers.packer.uoffset, buf, offset)
        x = ActorResourceReference()
        x.Init(buf, n + offset)
        return x

    @classmethod
    def GetRootAsActorResourceReference(cls, buf, offset=0):
        """This method is deprecated. Please switch to GetRootAs."""
        return cls.GetRootAs(buf, offset)
    # ActorResourceReference
    def Init(self, buf, pos):
        self._tab = flatbuffers.table.Table(buf, pos)

    # ActorResourceReference
    def Actor(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(4))
        if o != 0:
            return self._tab.String(o + self._tab.Pos)
        return None

def ActorResourceReferenceStart(builder):
    builder.StartObject(1)

def Start(builder):
    ActorResourceReferenceStart(builder)

def ActorResourceReferenceAddActor(builder, actor):
    builder.PrependUOffsetTRelativeSlot(0, flatbuffers.number_types.UOffsetTFlags.py_type(actor), 0)

def AddActor(builder, actor):
    ActorResourceReferenceAddActor(builder, actor)

def ActorResourceReferenceEnd(builder):
    return builder.EndObject()

def End(builder):
    return ActorResourceReferenceEnd(builder)
