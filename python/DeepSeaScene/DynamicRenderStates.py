# automatically generated by the FlatBuffers compiler, do not modify

# namespace: DeepSeaScene

import flatbuffers
from flatbuffers.compat import import_numpy
np = import_numpy()

class DynamicRenderStates(object):
    __slots__ = ['_tab']

    @classmethod
    def GetRootAs(cls, buf, offset=0):
        n = flatbuffers.encode.Get(flatbuffers.packer.uoffset, buf, offset)
        x = DynamicRenderStates()
        x.Init(buf, n + offset)
        return x

    @classmethod
    def GetRootAsDynamicRenderStates(cls, buf, offset=0):
        """This method is deprecated. Please switch to GetRootAs."""
        return cls.GetRootAs(buf, offset)
    # DynamicRenderStates
    def Init(self, buf, pos):
        self._tab = flatbuffers.table.Table(buf, pos)

    # DynamicRenderStates
    def LineWidth(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(4))
        if o != 0:
            return self._tab.Get(flatbuffers.number_types.Float32Flags, o + self._tab.Pos)
        return 0.0

    # DynamicRenderStates
    def DepthBiasConstantFactor(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(6))
        if o != 0:
            return self._tab.Get(flatbuffers.number_types.Float32Flags, o + self._tab.Pos)
        return 0.0

    # DynamicRenderStates
    def DepthBiasClamp(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(8))
        if o != 0:
            return self._tab.Get(flatbuffers.number_types.Float32Flags, o + self._tab.Pos)
        return 0.0

    # DynamicRenderStates
    def DepthBiasSlopeFactor(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(10))
        if o != 0:
            return self._tab.Get(flatbuffers.number_types.Float32Flags, o + self._tab.Pos)
        return 0.0

    # DynamicRenderStates
    def BlendConstants(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(12))
        if o != 0:
            x = o + self._tab.Pos
            from DeepSeaScene.Color4f import Color4f
            obj = Color4f()
            obj.Init(self._tab.Bytes, x)
            return obj
        return None

    # DynamicRenderStates
    def DepthBounds(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(14))
        if o != 0:
            x = o + self._tab.Pos
            from DeepSeaScene.Vector2f import Vector2f
            obj = Vector2f()
            obj.Init(self._tab.Bytes, x)
            return obj
        return None

    # DynamicRenderStates
    def FrontStencilCompareMask(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(16))
        if o != 0:
            return self._tab.Get(flatbuffers.number_types.Uint32Flags, o + self._tab.Pos)
        return 0

    # DynamicRenderStates
    def BackStencilCompareMask(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(18))
        if o != 0:
            return self._tab.Get(flatbuffers.number_types.Uint32Flags, o + self._tab.Pos)
        return 0

    # DynamicRenderStates
    def FrontStencilWriteMask(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(20))
        if o != 0:
            return self._tab.Get(flatbuffers.number_types.Uint32Flags, o + self._tab.Pos)
        return 0

    # DynamicRenderStates
    def BackStencilWriteMask(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(22))
        if o != 0:
            return self._tab.Get(flatbuffers.number_types.Uint32Flags, o + self._tab.Pos)
        return 0

    # DynamicRenderStates
    def FrontStencilReference(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(24))
        if o != 0:
            return self._tab.Get(flatbuffers.number_types.Uint32Flags, o + self._tab.Pos)
        return 0

    # DynamicRenderStates
    def BackStencilReference(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(26))
        if o != 0:
            return self._tab.Get(flatbuffers.number_types.Uint32Flags, o + self._tab.Pos)
        return 0

def DynamicRenderStatesStart(builder): builder.StartObject(12)
def Start(builder):
    return DynamicRenderStatesStart(builder)
def DynamicRenderStatesAddLineWidth(builder, lineWidth): builder.PrependFloat32Slot(0, lineWidth, 0.0)
def AddLineWidth(builder, lineWidth):
    return DynamicRenderStatesAddLineWidth(builder, lineWidth)
def DynamicRenderStatesAddDepthBiasConstantFactor(builder, depthBiasConstantFactor): builder.PrependFloat32Slot(1, depthBiasConstantFactor, 0.0)
def AddDepthBiasConstantFactor(builder, depthBiasConstantFactor):
    return DynamicRenderStatesAddDepthBiasConstantFactor(builder, depthBiasConstantFactor)
def DynamicRenderStatesAddDepthBiasClamp(builder, depthBiasClamp): builder.PrependFloat32Slot(2, depthBiasClamp, 0.0)
def AddDepthBiasClamp(builder, depthBiasClamp):
    return DynamicRenderStatesAddDepthBiasClamp(builder, depthBiasClamp)
def DynamicRenderStatesAddDepthBiasSlopeFactor(builder, depthBiasSlopeFactor): builder.PrependFloat32Slot(3, depthBiasSlopeFactor, 0.0)
def AddDepthBiasSlopeFactor(builder, depthBiasSlopeFactor):
    return DynamicRenderStatesAddDepthBiasSlopeFactor(builder, depthBiasSlopeFactor)
def DynamicRenderStatesAddBlendConstants(builder, blendConstants): builder.PrependStructSlot(4, flatbuffers.number_types.UOffsetTFlags.py_type(blendConstants), 0)
def AddBlendConstants(builder, blendConstants):
    return DynamicRenderStatesAddBlendConstants(builder, blendConstants)
def DynamicRenderStatesAddDepthBounds(builder, depthBounds): builder.PrependStructSlot(5, flatbuffers.number_types.UOffsetTFlags.py_type(depthBounds), 0)
def AddDepthBounds(builder, depthBounds):
    return DynamicRenderStatesAddDepthBounds(builder, depthBounds)
def DynamicRenderStatesAddFrontStencilCompareMask(builder, frontStencilCompareMask): builder.PrependUint32Slot(6, frontStencilCompareMask, 0)
def AddFrontStencilCompareMask(builder, frontStencilCompareMask):
    return DynamicRenderStatesAddFrontStencilCompareMask(builder, frontStencilCompareMask)
def DynamicRenderStatesAddBackStencilCompareMask(builder, backStencilCompareMask): builder.PrependUint32Slot(7, backStencilCompareMask, 0)
def AddBackStencilCompareMask(builder, backStencilCompareMask):
    return DynamicRenderStatesAddBackStencilCompareMask(builder, backStencilCompareMask)
def DynamicRenderStatesAddFrontStencilWriteMask(builder, frontStencilWriteMask): builder.PrependUint32Slot(8, frontStencilWriteMask, 0)
def AddFrontStencilWriteMask(builder, frontStencilWriteMask):
    return DynamicRenderStatesAddFrontStencilWriteMask(builder, frontStencilWriteMask)
def DynamicRenderStatesAddBackStencilWriteMask(builder, backStencilWriteMask): builder.PrependUint32Slot(9, backStencilWriteMask, 0)
def AddBackStencilWriteMask(builder, backStencilWriteMask):
    return DynamicRenderStatesAddBackStencilWriteMask(builder, backStencilWriteMask)
def DynamicRenderStatesAddFrontStencilReference(builder, frontStencilReference): builder.PrependUint32Slot(10, frontStencilReference, 0)
def AddFrontStencilReference(builder, frontStencilReference):
    return DynamicRenderStatesAddFrontStencilReference(builder, frontStencilReference)
def DynamicRenderStatesAddBackStencilReference(builder, backStencilReference): builder.PrependUint32Slot(11, backStencilReference, 0)
def AddBackStencilReference(builder, backStencilReference):
    return DynamicRenderStatesAddBackStencilReference(builder, backStencilReference)
def DynamicRenderStatesEnd(builder): return builder.EndObject()
def End(builder):
    return DynamicRenderStatesEnd(builder)