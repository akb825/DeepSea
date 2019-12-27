# automatically generated by the FlatBuffers compiler, do not modify

# namespace: DeepSeaScene

import flatbuffers

class SceneResources(object):
    __slots__ = ['_tab']

    @classmethod
    def GetRootAsSceneResources(cls, buf, offset):
        n = flatbuffers.encode.Get(flatbuffers.packer.uoffset, buf, offset)
        x = SceneResources()
        x.Init(buf, n + offset)
        return x

    # SceneResources
    def Init(self, buf, pos):
        self._tab = flatbuffers.table.Table(buf, pos)

    # SceneResources
    def Buffers(self, j):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(4))
        if o != 0:
            x = self._tab.Vector(o)
            x += flatbuffers.number_types.UOffsetTFlags.py_type(j) * 4
            x = self._tab.Indirect(x)
            from .Buffer import Buffer
            obj = Buffer()
            obj.Init(self._tab.Bytes, x)
            return obj
        return None

    # SceneResources
    def BuffersLength(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(4))
        if o != 0:
            return self._tab.VectorLen(o)
        return 0

    # SceneResources
    def Textures(self, j):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(6))
        if o != 0:
            x = self._tab.Vector(o)
            x += flatbuffers.number_types.UOffsetTFlags.py_type(j) * 4
            x = self._tab.Indirect(x)
            from .Texture import Texture
            obj = Texture()
            obj.Init(self._tab.Bytes, x)
            return obj
        return None

    # SceneResources
    def TexturesLength(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(6))
        if o != 0:
            return self._tab.VectorLen(o)
        return 0

    # SceneResources
    def ShaderVariableGroupDescs(self, j):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(8))
        if o != 0:
            x = self._tab.Vector(o)
            x += flatbuffers.number_types.UOffsetTFlags.py_type(j) * 4
            x = self._tab.Indirect(x)
            from .ShaderVariableGroupDesc import ShaderVariableGroupDesc
            obj = ShaderVariableGroupDesc()
            obj.Init(self._tab.Bytes, x)
            return obj
        return None

    # SceneResources
    def ShaderVariableGroupDescsLength(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(8))
        if o != 0:
            return self._tab.VectorLen(o)
        return 0

    # SceneResources
    def ShaderVariableGroups(self, j):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(10))
        if o != 0:
            x = self._tab.Vector(o)
            x += flatbuffers.number_types.UOffsetTFlags.py_type(j) * 4
            x = self._tab.Indirect(x)
            from .ShaderData import ShaderData
            obj = ShaderData()
            obj.Init(self._tab.Bytes, x)
            return obj
        return None

    # SceneResources
    def ShaderVariableGroupsLength(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(10))
        if o != 0:
            return self._tab.VectorLen(o)
        return 0

    # SceneResources
    def MaterialDescs(self, j):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(12))
        if o != 0:
            x = self._tab.Vector(o)
            x += flatbuffers.number_types.UOffsetTFlags.py_type(j) * 4
            x = self._tab.Indirect(x)
            from .MaterialDesc import MaterialDesc
            obj = MaterialDesc()
            obj.Init(self._tab.Bytes, x)
            return obj
        return None

    # SceneResources
    def MaterialDescsLength(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(12))
        if o != 0:
            return self._tab.VectorLen(o)
        return 0

    # SceneResources
    def Materials(self, j):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(14))
        if o != 0:
            x = self._tab.Vector(o)
            x += flatbuffers.number_types.UOffsetTFlags.py_type(j) * 4
            x = self._tab.Indirect(x)
            from .ShaderData import ShaderData
            obj = ShaderData()
            obj.Init(self._tab.Bytes, x)
            return obj
        return None

    # SceneResources
    def MaterialsLength(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(14))
        if o != 0:
            return self._tab.VectorLen(o)
        return 0

    # SceneResources
    def ShaderModules(self, j):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(16))
        if o != 0:
            x = self._tab.Vector(o)
            x += flatbuffers.number_types.UOffsetTFlags.py_type(j) * 4
            x = self._tab.Indirect(x)
            from .ShaderModule import ShaderModule
            obj = ShaderModule()
            obj.Init(self._tab.Bytes, x)
            return obj
        return None

    # SceneResources
    def ShaderModulesLength(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(16))
        if o != 0:
            return self._tab.VectorLen(o)
        return 0

    # SceneResources
    def Shaders(self, j):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(18))
        if o != 0:
            x = self._tab.Vector(o)
            x += flatbuffers.number_types.UOffsetTFlags.py_type(j) * 4
            x = self._tab.Indirect(x)
            from .Shader import Shader
            obj = Shader()
            obj.Init(self._tab.Bytes, x)
            return obj
        return None

    # SceneResources
    def ShadersLength(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(18))
        if o != 0:
            return self._tab.VectorLen(o)
        return 0

    # SceneResources
    def DrawGeometries(self, j):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(20))
        if o != 0:
            x = self._tab.Vector(o)
            x += flatbuffers.number_types.UOffsetTFlags.py_type(j) * 4
            x = self._tab.Indirect(x)
            from .DrawGeometry import DrawGeometry
            obj = DrawGeometry()
            obj.Init(self._tab.Bytes, x)
            return obj
        return None

    # SceneResources
    def DrawGeometriesLength(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(20))
        if o != 0:
            return self._tab.VectorLen(o)
        return 0

    # SceneResources
    def SceneNodes(self, j):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(22))
        if o != 0:
            x = self._tab.Vector(o)
            x += flatbuffers.number_types.UOffsetTFlags.py_type(j) * 4
            x = self._tab.Indirect(x)
            from .NamedSceneNode import NamedSceneNode
            obj = NamedSceneNode()
            obj.Init(self._tab.Bytes, x)
            return obj
        return None

    # SceneResources
    def SceneNodesLength(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(22))
        if o != 0:
            return self._tab.VectorLen(o)
        return 0

def SceneResourcesStart(builder): builder.StartObject(10)
def SceneResourcesAddBuffers(builder, buffers): builder.PrependUOffsetTRelativeSlot(0, flatbuffers.number_types.UOffsetTFlags.py_type(buffers), 0)
def SceneResourcesStartBuffersVector(builder, numElems): return builder.StartVector(4, numElems, 4)
def SceneResourcesAddTextures(builder, textures): builder.PrependUOffsetTRelativeSlot(1, flatbuffers.number_types.UOffsetTFlags.py_type(textures), 0)
def SceneResourcesStartTexturesVector(builder, numElems): return builder.StartVector(4, numElems, 4)
def SceneResourcesAddShaderVariableGroupDescs(builder, shaderVariableGroupDescs): builder.PrependUOffsetTRelativeSlot(2, flatbuffers.number_types.UOffsetTFlags.py_type(shaderVariableGroupDescs), 0)
def SceneResourcesStartShaderVariableGroupDescsVector(builder, numElems): return builder.StartVector(4, numElems, 4)
def SceneResourcesAddShaderVariableGroups(builder, shaderVariableGroups): builder.PrependUOffsetTRelativeSlot(3, flatbuffers.number_types.UOffsetTFlags.py_type(shaderVariableGroups), 0)
def SceneResourcesStartShaderVariableGroupsVector(builder, numElems): return builder.StartVector(4, numElems, 4)
def SceneResourcesAddMaterialDescs(builder, materialDescs): builder.PrependUOffsetTRelativeSlot(4, flatbuffers.number_types.UOffsetTFlags.py_type(materialDescs), 0)
def SceneResourcesStartMaterialDescsVector(builder, numElems): return builder.StartVector(4, numElems, 4)
def SceneResourcesAddMaterials(builder, materials): builder.PrependUOffsetTRelativeSlot(5, flatbuffers.number_types.UOffsetTFlags.py_type(materials), 0)
def SceneResourcesStartMaterialsVector(builder, numElems): return builder.StartVector(4, numElems, 4)
def SceneResourcesAddShaderModules(builder, shaderModules): builder.PrependUOffsetTRelativeSlot(6, flatbuffers.number_types.UOffsetTFlags.py_type(shaderModules), 0)
def SceneResourcesStartShaderModulesVector(builder, numElems): return builder.StartVector(4, numElems, 4)
def SceneResourcesAddShaders(builder, shaders): builder.PrependUOffsetTRelativeSlot(7, flatbuffers.number_types.UOffsetTFlags.py_type(shaders), 0)
def SceneResourcesStartShadersVector(builder, numElems): return builder.StartVector(4, numElems, 4)
def SceneResourcesAddDrawGeometries(builder, drawGeometries): builder.PrependUOffsetTRelativeSlot(8, flatbuffers.number_types.UOffsetTFlags.py_type(drawGeometries), 0)
def SceneResourcesStartDrawGeometriesVector(builder, numElems): return builder.StartVector(4, numElems, 4)
def SceneResourcesAddSceneNodes(builder, sceneNodes): builder.PrependUOffsetTRelativeSlot(9, flatbuffers.number_types.UOffsetTFlags.py_type(sceneNodes), 0)
def SceneResourcesStartSceneNodesVector(builder, numElems): return builder.StartVector(4, numElems, 4)
def SceneResourcesEnd(builder): return builder.EndObject()
