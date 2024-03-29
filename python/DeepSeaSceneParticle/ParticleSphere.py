# automatically generated by the FlatBuffers compiler, do not modify

# namespace: DeepSeaSceneParticle

import flatbuffers
from flatbuffers.compat import import_numpy
np = import_numpy()

class ParticleSphere(object):
    __slots__ = ['_tab']

    @classmethod
    def GetRootAs(cls, buf, offset=0):
        n = flatbuffers.encode.Get(flatbuffers.packer.uoffset, buf, offset)
        x = ParticleSphere()
        x.Init(buf, n + offset)
        return x

    @classmethod
    def GetRootAsParticleSphere(cls, buf, offset=0):
        """This method is deprecated. Please switch to GetRootAs."""
        return cls.GetRootAs(buf, offset)
    # ParticleSphere
    def Init(self, buf, pos):
        self._tab = flatbuffers.table.Table(buf, pos)

    # ParticleSphere
    def Center(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(4))
        if o != 0:
            x = o + self._tab.Pos
            from DeepSeaScene.Vector3f import Vector3f
            obj = Vector3f()
            obj.Init(self._tab.Bytes, x)
            return obj
        return None

    # ParticleSphere
    def Radius(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(6))
        if o != 0:
            return self._tab.Get(flatbuffers.number_types.Float32Flags, o + self._tab.Pos)
        return 0.0

def ParticleSphereStart(builder):
    builder.StartObject(2)

def Start(builder):
    ParticleSphereStart(builder)

def ParticleSphereAddCenter(builder, center):
    builder.PrependStructSlot(0, flatbuffers.number_types.UOffsetTFlags.py_type(center), 0)

def AddCenter(builder, center):
    ParticleSphereAddCenter(builder, center)

def ParticleSphereAddRadius(builder, radius):
    builder.PrependFloat32Slot(1, radius, 0.0)

def AddRadius(builder, radius):
    ParticleSphereAddRadius(builder, radius)

def ParticleSphereEnd(builder):
    return builder.EndObject()

def End(builder):
    return ParticleSphereEnd(builder)
