# automatically generated by the FlatBuffers compiler, do not modify

# namespace: DeepSeaScene

import flatbuffers
from flatbuffers.compat import import_numpy
np = import_numpy()

class AlignedBox3f(object):
    __slots__ = ['_tab']

    # AlignedBox3f
    def Init(self, buf, pos):
        self._tab = flatbuffers.table.Table(buf, pos)

    # AlignedBox3f
    def Min(self, obj):
        obj.Init(self._tab.Bytes, self._tab.Pos + 0)
        return obj

    # AlignedBox3f
    def Max(self, obj):
        obj.Init(self._tab.Bytes, self._tab.Pos + 12)
        return obj


def CreateAlignedBox3f(builder, min_x, min_y, min_z, max_x, max_y, max_z):
    builder.Prep(4, 24)
    builder.Prep(4, 12)
    builder.PrependFloat32(max_z)
    builder.PrependFloat32(max_y)
    builder.PrependFloat32(max_x)
    builder.Prep(4, 12)
    builder.PrependFloat32(min_z)
    builder.PrependFloat32(min_y)
    builder.PrependFloat32(min_x)
    return builder.Offset()
