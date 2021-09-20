# automatically generated by the FlatBuffers compiler, do not modify

# namespace: DeepSeaSceneLighting

import flatbuffers
from flatbuffers.compat import import_numpy
np = import_numpy()

class DeferredShadowLightInfo(object):
    __slots__ = ['_tab']

    @classmethod
    def GetRootAs(cls, buf, offset=0):
        n = flatbuffers.encode.Get(flatbuffers.packer.uoffset, buf, offset)
        x = DeferredShadowLightInfo()
        x.Init(buf, n + offset)
        return x

    @classmethod
    def GetRootAsDeferredShadowLightInfo(cls, buf, offset=0):
        """This method is deprecated. Please switch to GetRootAs."""
        return cls.GetRootAs(buf, offset)
    # DeferredShadowLightInfo
    def Init(self, buf, pos):
        self._tab = flatbuffers.table.Table(buf, pos)

    # DeferredShadowLightInfo
    def Shader(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(4))
        if o != 0:
            return self._tab.String(o + self._tab.Pos)
        return None

    # DeferredShadowLightInfo
    def Material(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(6))
        if o != 0:
            return self._tab.String(o + self._tab.Pos)
        return None

    # DeferredShadowLightInfo
    def TransformGroup(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(8))
        if o != 0:
            return self._tab.String(o + self._tab.Pos)
        return None

def Start(builder): builder.StartObject(3)
def DeferredShadowLightInfoStart(builder):
    """This method is deprecated. Please switch to Start."""
    return Start(builder)
def AddShader(builder, shader): builder.PrependUOffsetTRelativeSlot(0, flatbuffers.number_types.UOffsetTFlags.py_type(shader), 0)
def DeferredShadowLightInfoAddShader(builder, shader):
    """This method is deprecated. Please switch to AddShader."""
    return AddShader(builder, shader)
def AddMaterial(builder, material): builder.PrependUOffsetTRelativeSlot(1, flatbuffers.number_types.UOffsetTFlags.py_type(material), 0)
def DeferredShadowLightInfoAddMaterial(builder, material):
    """This method is deprecated. Please switch to AddMaterial."""
    return AddMaterial(builder, material)
def AddTransformGroup(builder, transformGroup): builder.PrependUOffsetTRelativeSlot(2, flatbuffers.number_types.UOffsetTFlags.py_type(transformGroup), 0)
def DeferredShadowLightInfoAddTransformGroup(builder, transformGroup):
    """This method is deprecated. Please switch to AddTransformGroup."""
    return AddTransformGroup(builder, transformGroup)
def End(builder): return builder.EndObject()
def DeferredShadowLightInfoEnd(builder):
    """This method is deprecated. Please switch to End."""
    return End(builder)