# automatically generated by the FlatBuffers compiler, do not modify

# namespace: DeepSeaVectorDrawScene

import flatbuffers
from flatbuffers.compat import import_numpy
np = import_numpy()

class VectorShaders(object):
    __slots__ = ['_tab']

    @classmethod
    def GetRootAs(cls, buf, offset=0):
        n = flatbuffers.encode.Get(flatbuffers.packer.uoffset, buf, offset)
        x = VectorShaders()
        x.Init(buf, n + offset)
        return x

    @classmethod
    def GetRootAsVectorShaders(cls, buf, offset=0):
        """This method is deprecated. Please switch to GetRootAs."""
        return cls.GetRootAs(buf, offset)
    # VectorShaders
    def Init(self, buf, pos):
        self._tab = flatbuffers.table.Table(buf, pos)

    # VectorShaders
    def Modules(self, j):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(4))
        if o != 0:
            x = self._tab.Vector(o)
            x += flatbuffers.number_types.UOffsetTFlags.py_type(j) * 4
            x = self._tab.Indirect(x)
            from DeepSeaScene.VersionedShaderModule import VersionedShaderModule
            obj = VersionedShaderModule()
            obj.Init(self._tab.Bytes, x)
            return obj
        return None

    # VectorShaders
    def ModulesLength(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(4))
        if o != 0:
            return self._tab.VectorLen(o)
        return 0

    # VectorShaders
    def ModulesIsNone(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(4))
        return o == 0

    # VectorShaders
    def ExtraElements(self, j):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(6))
        if o != 0:
            x = self._tab.Vector(o)
            x += flatbuffers.number_types.UOffsetTFlags.py_type(j) * 4
            x = self._tab.Indirect(x)
            from DeepSeaVectorDrawScene.MaterialElement import MaterialElement
            obj = MaterialElement()
            obj.Init(self._tab.Bytes, x)
            return obj
        return None

    # VectorShaders
    def ExtraElementsLength(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(6))
        if o != 0:
            return self._tab.VectorLen(o)
        return 0

    # VectorShaders
    def ExtraElementsIsNone(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(6))
        return o == 0

    # VectorShaders
    def MaterialDesc(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(8))
        if o != 0:
            return self._tab.String(o + self._tab.Pos)
        return None

    # VectorShaders
    def FillColor(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(10))
        if o != 0:
            return self._tab.String(o + self._tab.Pos)
        return None

    # VectorShaders
    def FillLinearGradient(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(12))
        if o != 0:
            return self._tab.String(o + self._tab.Pos)
        return None

    # VectorShaders
    def FillRadialGradient(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(14))
        if o != 0:
            return self._tab.String(o + self._tab.Pos)
        return None

    # VectorShaders
    def Line(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(16))
        if o != 0:
            return self._tab.String(o + self._tab.Pos)
        return None

    # VectorShaders
    def Image(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(18))
        if o != 0:
            return self._tab.String(o + self._tab.Pos)
        return None

    # VectorShaders
    def TextColor(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(20))
        if o != 0:
            return self._tab.String(o + self._tab.Pos)
        return None

    # VectorShaders
    def TextColorOutline(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(22))
        if o != 0:
            return self._tab.String(o + self._tab.Pos)
        return None

    # VectorShaders
    def TextGradient(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(24))
        if o != 0:
            return self._tab.String(o + self._tab.Pos)
        return None

    # VectorShaders
    def TextGradientOutline(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(26))
        if o != 0:
            return self._tab.String(o + self._tab.Pos)
        return None

def Start(builder): builder.StartObject(12)
def VectorShadersStart(builder):
    """This method is deprecated. Please switch to Start."""
    return Start(builder)
def AddModules(builder, modules): builder.PrependUOffsetTRelativeSlot(0, flatbuffers.number_types.UOffsetTFlags.py_type(modules), 0)
def VectorShadersAddModules(builder, modules):
    """This method is deprecated. Please switch to AddModules."""
    return AddModules(builder, modules)
def StartModulesVector(builder, numElems): return builder.StartVector(4, numElems, 4)
def VectorShadersStartModulesVector(builder, numElems):
    """This method is deprecated. Please switch to Start."""
    return StartModulesVector(builder, numElems)
def AddExtraElements(builder, extraElements): builder.PrependUOffsetTRelativeSlot(1, flatbuffers.number_types.UOffsetTFlags.py_type(extraElements), 0)
def VectorShadersAddExtraElements(builder, extraElements):
    """This method is deprecated. Please switch to AddExtraElements."""
    return AddExtraElements(builder, extraElements)
def StartExtraElementsVector(builder, numElems): return builder.StartVector(4, numElems, 4)
def VectorShadersStartExtraElementsVector(builder, numElems):
    """This method is deprecated. Please switch to Start."""
    return StartExtraElementsVector(builder, numElems)
def AddMaterialDesc(builder, materialDesc): builder.PrependUOffsetTRelativeSlot(2, flatbuffers.number_types.UOffsetTFlags.py_type(materialDesc), 0)
def VectorShadersAddMaterialDesc(builder, materialDesc):
    """This method is deprecated. Please switch to AddMaterialDesc."""
    return AddMaterialDesc(builder, materialDesc)
def AddFillColor(builder, fillColor): builder.PrependUOffsetTRelativeSlot(3, flatbuffers.number_types.UOffsetTFlags.py_type(fillColor), 0)
def VectorShadersAddFillColor(builder, fillColor):
    """This method is deprecated. Please switch to AddFillColor."""
    return AddFillColor(builder, fillColor)
def AddFillLinearGradient(builder, fillLinearGradient): builder.PrependUOffsetTRelativeSlot(4, flatbuffers.number_types.UOffsetTFlags.py_type(fillLinearGradient), 0)
def VectorShadersAddFillLinearGradient(builder, fillLinearGradient):
    """This method is deprecated. Please switch to AddFillLinearGradient."""
    return AddFillLinearGradient(builder, fillLinearGradient)
def AddFillRadialGradient(builder, fillRadialGradient): builder.PrependUOffsetTRelativeSlot(5, flatbuffers.number_types.UOffsetTFlags.py_type(fillRadialGradient), 0)
def VectorShadersAddFillRadialGradient(builder, fillRadialGradient):
    """This method is deprecated. Please switch to AddFillRadialGradient."""
    return AddFillRadialGradient(builder, fillRadialGradient)
def AddLine(builder, line): builder.PrependUOffsetTRelativeSlot(6, flatbuffers.number_types.UOffsetTFlags.py_type(line), 0)
def VectorShadersAddLine(builder, line):
    """This method is deprecated. Please switch to AddLine."""
    return AddLine(builder, line)
def AddImage(builder, image): builder.PrependUOffsetTRelativeSlot(7, flatbuffers.number_types.UOffsetTFlags.py_type(image), 0)
def VectorShadersAddImage(builder, image):
    """This method is deprecated. Please switch to AddImage."""
    return AddImage(builder, image)
def AddTextColor(builder, textColor): builder.PrependUOffsetTRelativeSlot(8, flatbuffers.number_types.UOffsetTFlags.py_type(textColor), 0)
def VectorShadersAddTextColor(builder, textColor):
    """This method is deprecated. Please switch to AddTextColor."""
    return AddTextColor(builder, textColor)
def AddTextColorOutline(builder, textColorOutline): builder.PrependUOffsetTRelativeSlot(9, flatbuffers.number_types.UOffsetTFlags.py_type(textColorOutline), 0)
def VectorShadersAddTextColorOutline(builder, textColorOutline):
    """This method is deprecated. Please switch to AddTextColorOutline."""
    return AddTextColorOutline(builder, textColorOutline)
def AddTextGradient(builder, textGradient): builder.PrependUOffsetTRelativeSlot(10, flatbuffers.number_types.UOffsetTFlags.py_type(textGradient), 0)
def VectorShadersAddTextGradient(builder, textGradient):
    """This method is deprecated. Please switch to AddTextGradient."""
    return AddTextGradient(builder, textGradient)
def AddTextGradientOutline(builder, textGradientOutline): builder.PrependUOffsetTRelativeSlot(11, flatbuffers.number_types.UOffsetTFlags.py_type(textGradientOutline), 0)
def VectorShadersAddTextGradientOutline(builder, textGradientOutline):
    """This method is deprecated. Please switch to AddTextGradientOutline."""
    return AddTextGradientOutline(builder, textGradientOutline)
def End(builder): return builder.EndObject()
def VectorShadersEnd(builder):
    """This method is deprecated. Please switch to End."""
    return End(builder)