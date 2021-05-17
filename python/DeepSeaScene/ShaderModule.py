# automatically generated by the FlatBuffers compiler, do not modify

# namespace: DeepSeaScene

import flatbuffers
from flatbuffers.compat import import_numpy
np = import_numpy()

class ShaderModule(object):
    __slots__ = ['_tab']

    @classmethod
    def GetRootAs(cls, buf, offset=0):
        n = flatbuffers.encode.Get(flatbuffers.packer.uoffset, buf, offset)
        x = ShaderModule()
        x.Init(buf, n + offset)
        return x

    @classmethod
    def GetRootAsShaderModule(cls, buf, offset=0):
        """This method is deprecated. Please switch to GetRootAs."""
        return cls.GetRootAs(buf, offset)
    # ShaderModule
    def Init(self, buf, pos):
        self._tab = flatbuffers.table.Table(buf, pos)

    # ShaderModule
    def Name(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(4))
        if o != 0:
            return self._tab.String(o + self._tab.Pos)
        return None

    # ShaderModule
    def Modules(self, j):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(6))
        if o != 0:
            x = self._tab.Vector(o)
            x += flatbuffers.number_types.UOffsetTFlags.py_type(j) * 4
            x = self._tab.Indirect(x)
            from DeepSeaScene.VersionedShaderModule import VersionedShaderModule
            obj = VersionedShaderModule()
            obj.Init(self._tab.Bytes, x)
            return obj
        return None

    # ShaderModule
    def ModulesLength(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(6))
        if o != 0:
            return self._tab.VectorLen(o)
        return 0

    # ShaderModule
    def ModulesIsNone(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(6))
        return o == 0

def Start(builder): builder.StartObject(2)
def ShaderModuleStart(builder):
    """This method is deprecated. Please switch to Start."""
    return Start(builder)
def AddName(builder, name): builder.PrependUOffsetTRelativeSlot(0, flatbuffers.number_types.UOffsetTFlags.py_type(name), 0)
def ShaderModuleAddName(builder, name):
    """This method is deprecated. Please switch to AddName."""
    return AddName(builder, name)
def AddModules(builder, modules): builder.PrependUOffsetTRelativeSlot(1, flatbuffers.number_types.UOffsetTFlags.py_type(modules), 0)
def ShaderModuleAddModules(builder, modules):
    """This method is deprecated. Please switch to AddModules."""
    return AddModules(builder, modules)
def StartModulesVector(builder, numElems): return builder.StartVector(4, numElems, 4)
def ShaderModuleStartModulesVector(builder, numElems):
    """This method is deprecated. Please switch to Start."""
    return StartModulesVector(builder, numElems)
def End(builder): return builder.EndObject()
def ShaderModuleEnd(builder):
    """This method is deprecated. Please switch to End."""
    return End(builder)