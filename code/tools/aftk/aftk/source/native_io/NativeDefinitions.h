#ifndef NATIVE_DEFINITIONS_H
#define NATIVE_DEFINITIONS_H

#ifdef _MSC_VER_
#pragma once
#endif

//Lock files for exclusive access.
#define AFTK_LOCKING_2

#define NATIVE_MAX_BLOCK_DEPTH						256
#define NATIVE_MAX_CONTOUR_BITS						8
#define NATIVE_MAX_TEXTURE_COORDINATE_SET_BITS		8
#define NATIVE_MAX_BLIND_DATA_PER_POLY_BITS			8
#define NATIVE_MAX_MATERIAL_PER_POLY_BITS			8
#define NATIVE_NUM_TEXTURE_COORDINATE_ELEMENTS		4
#define NATIVE_LOWEST_FILE_VERSION					200
#define NATIVE_FILE_VERSION							211
#define NATIVE_COLOUR_BPP							32

//If defined, take extra care to check that we're reading valid blocks
#define NATIVE_BE_EXTRA_CAREFUL

#define NATIVE_ID_LENGTH						4
#define NATIVE_FILE_TYPE_ID_A3D					C_NativeUtility::GetBlockIdInt("A3D2")
#define NATIVE_COMPRESSION_TYPE_NONE			C_NativeUtility::GetBlockIdInt("CNON")

//Data types
#define NATIVE_TYPE_ID_F32						C_NativeUtility::GetBlockIdInt("F32 ")
#define NATIVE_TYPE_ID_F64						C_NativeUtility::GetBlockIdInt("F64 ")
#define NATIVE_TYPE_ID_U8						C_NativeUtility::GetBlockIdInt("U8  ")
#define NATIVE_TYPE_ID_U16						C_NativeUtility::GetBlockIdInt("U16 ")
#define NATIVE_TYPE_ID_U32						C_NativeUtility::GetBlockIdInt("U32 ")
#define NATIVE_TYPE_ID_U64						C_NativeUtility::GetBlockIdInt("U64 ")
#define NATIVE_TYPE_ID_S8						C_NativeUtility::GetBlockIdInt("S8  ")
#define NATIVE_TYPE_ID_S16						C_NativeUtility::GetBlockIdInt("S16 ")
#define NATIVE_TYPE_ID_S32						C_NativeUtility::GetBlockIdInt("S32 ")
#define NATIVE_TYPE_ID_S64						C_NativeUtility::GetBlockIdInt("S64 ")

#define NATIVE_TYPE_ID_VECTOR2F64				C_NativeUtility::GetBlockIdInt("2F64")
#define NATIVE_TYPE_ID_VECTOR2F32				C_NativeUtility::GetBlockIdInt("VEC2")
#define NATIVE_TYPE_ID_VECTOR2S64				C_NativeUtility::GetBlockIdInt("2S64")
#define NATIVE_TYPE_ID_VECTOR2S32				C_NativeUtility::GetBlockIdInt("2S32")
#define NATIVE_TYPE_ID_VECTOR2S16				C_NativeUtility::GetBlockIdInt("2S16")
#define NATIVE_TYPE_ID_VECTOR2S8				C_NativeUtility::GetBlockIdInt("2S8 ")
#define NATIVE_TYPE_ID_VECTOR2U64				C_NativeUtility::GetBlockIdInt("2U64")
#define NATIVE_TYPE_ID_VECTOR2U32				C_NativeUtility::GetBlockIdInt("2U32")
#define NATIVE_TYPE_ID_VECTOR2U16				C_NativeUtility::GetBlockIdInt("2U16")
#define NATIVE_TYPE_ID_VECTOR2U8				C_NativeUtility::GetBlockIdInt("2U8 ")

#define NATIVE_TYPE_ID_VECTOR3F64				C_NativeUtility::GetBlockIdInt("3F64")
#define NATIVE_TYPE_ID_VECTOR3F32				C_NativeUtility::GetBlockIdInt("VEC3")
#define NATIVE_TYPE_ID_VECTOR3S64				C_NativeUtility::GetBlockIdInt("3S64")
#define NATIVE_TYPE_ID_VECTOR3S32				C_NativeUtility::GetBlockIdInt("3S32")
#define NATIVE_TYPE_ID_VECTOR3S16				C_NativeUtility::GetBlockIdInt("3S16")
#define NATIVE_TYPE_ID_VECTOR3S8				C_NativeUtility::GetBlockIdInt("3S8 ")
#define NATIVE_TYPE_ID_VECTOR3U64				C_NativeUtility::GetBlockIdInt("3U64")
#define NATIVE_TYPE_ID_VECTOR3U32				C_NativeUtility::GetBlockIdInt("3U32")
#define NATIVE_TYPE_ID_VECTOR3U16				C_NativeUtility::GetBlockIdInt("3U16")
#define NATIVE_TYPE_ID_VECTOR3U8				C_NativeUtility::GetBlockIdInt("3U8 ")

#define NATIVE_TYPE_ID_VECTOR4F64				C_NativeUtility::GetBlockIdInt("4F64")
#define NATIVE_TYPE_ID_VECTOR4F32				C_NativeUtility::GetBlockIdInt("VEC4")
#define NATIVE_TYPE_ID_VECTOR4S64				C_NativeUtility::GetBlockIdInt("4S64")
#define NATIVE_TYPE_ID_VECTOR4S32				C_NativeUtility::GetBlockIdInt("4S32")
#define NATIVE_TYPE_ID_VECTOR4S16				C_NativeUtility::GetBlockIdInt("4S16")
#define NATIVE_TYPE_ID_VECTOR4S8				C_NativeUtility::GetBlockIdInt("4S8 ")
#define NATIVE_TYPE_ID_VECTOR4U64				C_NativeUtility::GetBlockIdInt("4U64")
#define NATIVE_TYPE_ID_VECTOR4U32				C_NativeUtility::GetBlockIdInt("4U32")
#define NATIVE_TYPE_ID_VECTOR4U16				C_NativeUtility::GetBlockIdInt("4U16")
#define NATIVE_TYPE_ID_VECTOR4U8				C_NativeUtility::GetBlockIdInt("4U8 ")

//Yes, i do intend for these two matrices to have the same id
#define NATIVE_TYPE_ID_MATRIX44F32				C_NativeUtility::GetBlockIdInt("MTRX")
#define NATIVE_TYPE_ID_MATRIX					C_NativeUtility::GetBlockIdInt("MTRX")
#define NATIVE_TYPE_ID_BOOLEAN					C_NativeUtility::GetBlockIdInt("BOOL")
#define NATIVE_TYPE_ID_STRING					C_NativeUtility::GetBlockIdInt("STR ")
#define NATIVE_TYPE_ID_BLOB						C_NativeUtility::GetBlockIdInt("BLOB")
#define NATIVE_TYPE_ID_NULL						C_NativeUtility::GetBlockIdInt("NULL")

#define NATIVE_BLOCK_ID_PROTOTYPE				C_NativeUtility::GetBlockIdInt("PROT")
#define NATIVE_BLOCK_ID_SCENE					C_NativeUtility::GetBlockIdInt("SCN ")
#define NATIVE_BLOCK_ID_LAYER_NODE				C_NativeUtility::GetBlockIdInt("LAYR")

//C_Node types
#define NATIVE_BLOCK_ID_NODE					C_NativeUtility::GetBlockIdInt("NODE")
#define NATIVE_BLOCK_ID_MATERIAL_NODE			C_NativeUtility::GetBlockIdInt("MTL ")
#define NATIVE_BLOCK_ID_SHADER_NODE				C_NativeUtility::GetBlockIdInt("SHDR")
#define NATIVE_BLOCK_ID_SHADER_IMAGE_NODE		C_NativeUtility::GetBlockIdInt("SIMG")
#define NATIVE_BLOCK_ID_BLIND_DATA_NODE			C_NativeUtility::GetBlockIdInt("BDTA")

#define NATIVE_BLOCK_ID_SCENE_NODE				C_NativeUtility::GetBlockIdInt("NULL")
#define NATIVE_BLOCK_ID_SCENE_JOINT_NODE		C_NativeUtility::GetBlockIdInt("JNT ")
#define NATIVE_BLOCK_ID_MESH_REMAPPABLE_NODE	C_NativeUtility::GetBlockIdInt("MRMN")
#define NATIVE_BLOCK_ID_MESH_NODE				C_NativeUtility::GetBlockIdInt("MESH")
#define NATIVE_BLOCK_ID_MESH_BONED_INSTANCE_NODE	C_NativeUtility::GetBlockIdInt("MBIN")
#define NATIVE_BLOCK_ID_MESH_REMAPPED_INSTANCE_NODE C_NativeUtility::GetBlockIdInt("MRIN")
#define NATIVE_BLOCK_ID_SPLINE_NODE				C_NativeUtility::GetBlockIdInt("SPLN")
#define NATIVE_BLOCK_ID_GROUP_NODE				C_NativeUtility::GetBlockIdInt("GRP ")
#define NATIVE_BLOCK_ID_LIGHT_NODE				C_NativeUtility::GetBlockIdInt("LGHT")
#define NATIVE_BLOCK_ID_NON_AMBIENT_LIGHT_NODE	C_NativeUtility::GetBlockIdInt("NALN")
#define NATIVE_BLOCK_ID_POINT_LIGHT_NODE		C_NativeUtility::GetBlockIdInt("PTLN")
#define NATIVE_BLOCK_ID_AREA_LIGHT_NODE			C_NativeUtility::GetBlockIdInt("AREA")
#define NATIVE_BLOCK_ID_DIRECTIONAL_LIGHT_NODE	C_NativeUtility::GetBlockIdInt("DRLN")
#define NATIVE_BLOCK_ID_SPOT_LIGHT_NODE			C_NativeUtility::GetBlockIdInt("SPLT")
#define NATIVE_BLOCK_ID_HEMISPHERE_LIGHT_NODE	C_NativeUtility::GetBlockIdInt("HMLT")
#define NATIVE_BLOCK_ID_INSTANCE_NODE			C_NativeUtility::GetBlockIdInt("INST")
#define NATIVE_BLOCK_ID_XREF_NODE				C_NativeUtility::GetBlockIdInt("XREF")
#define NATIVE_BLOCK_ID_ACTOR_NODE				C_NativeUtility::GetBlockIdInt("ACT ")
#define NATIVE_BLOCK_ID_CAMERA_NODE				C_NativeUtility::GetBlockIdInt("CAM ")
#define NATIVE_BLOCK_ID_SCENE_LOD_GROUP_NODE	C_NativeUtility::GetBlockIdInt("LODG")
#define NATIVE_BLOCK_ID_BLEND_WEIGHTED_NODE		C_NativeUtility::GetBlockIdInt("BWND")
#define NATIVE_BLOCK_ID_UNIT_CONVERSION_NODE	C_NativeUtility::GetBlockIdInt("UNTC")
#define NATIVE_BLOCK_ID_DISTANCE_DIM_NODE		C_NativeUtility::GetBlockIdInt("DDIM")
#define NATIVE_BLOCK_ID_AIM_CONSTRAINT_NODE		C_NativeUtility::GetBlockIdInt("AIM ")
#define NATIVE_BLOCK_ID_CONSTRAINT_NODE			C_NativeUtility::GetBlockIdInt("CNST")
#define NATIVE_BLOCK_ID_LOOK_AT_CONSTRAINT_NODE	C_NativeUtility::GetBlockIdInt("LOOK")
#define NATIVE_BLOCK_ID_POINT_CONSTRAINT_NODE	C_NativeUtility::GetBlockIdInt("PNTC")
#define NATIVE_BLOCK_ID_ORIENT_CONSTRAINT_NODE	C_NativeUtility::GetBlockIdInt("ORIC")

#define NATIVE_BLOCK_ID_MATERIAL_LIST			C_NativeUtility::GetBlockIdInt("MTLS")
#define NATIVE_BLOCK_ID_BLIND_DATA_LIST			C_NativeUtility::GetBlockIdInt("BDLS")
#define NATIVE_BLOCK_ID_GENERIC_ATTRIBUTE_LIST	C_NativeUtility::GetBlockIdInt("GATL")
#define NATIVE_BLOCK_ID_LEGACY_SCENE_NODE_LIST	C_NativeUtility::GetBlockIdInt("SNLS")
#define NATIVE_BLOCK_ID_UNMANAGED_DG_NODE_LIST	C_NativeUtility::GetBlockIdInt("UDGS")
#define NATIVE_BLOCK_ID_LAYER_LIST				C_NativeUtility::GetBlockIdInt("LYRS")
#define NATIVE_BLOCK_ID_LOD_LAYER_LIST			C_NativeUtility::GetBlockIdInt("LODS")
#define NATIVE_BLOCK_ID_PLUG_LIST				C_NativeUtility::GetBlockIdInt("PLGS")
#define NATIVE_BLOCK_ID_PLUG_CHILD_LIST			C_NativeUtility::GetBlockIdInt("PGCS")
#define NATIVE_BLOCK_ID_PLUG_CONNECTION_LIST	C_NativeUtility::GetBlockIdInt("PGCL")
#define NATIVE_BLOCK_ID_SHADER_LIST				C_NativeUtility::GetBlockIdInt("SDRS")

#define NATIVE_BLOCK_ID_POLYGON_LIST			C_NativeUtility::GetBlockIdInt("PLYS")
#define NATIVE_BLOCK_ID_POLYGON_VERTEX_LIST		C_NativeUtility::GetBlockIdInt("PVTS")

#define NATIVE_BLOCK_ID_GENERIC_ATTRIBUTE		C_NativeUtility::GetBlockIdInt("GATT")
#define NATIVE_BLOCK_ID_MAP						C_NativeUtility::GetBlockIdInt("MAP ")

#define NATIVE_BLOCK_ID_POLYGON					C_NativeUtility::GetBlockIdInt("POLY")
#define NATIVE_BLOCK_ID_POLYGON_VERTEX			C_NativeUtility::GetBlockIdInt("PVRT")

#define NATIVE_BLOCK_ID_PLUG					C_NativeUtility::GetBlockIdInt("PLUG")

#define NATIVE_BLOCK_ID_INFLUENCE_CLUSTER_NODE	C_NativeUtility::GetBlockIdInt("SCLS")
#define NATIVE_BLOCK_ID_WEIGHT_ENVELOPE			C_NativeUtility::GetBlockIdInt("SENV")
#define NATIVE_BLOCK_ID_INFLUENCE_CLUSTER_LIST	C_NativeUtility::GetBlockIdInt("SCLL")

#define NATIVE_BLOCK_ID_POLYGON_CLUSTER_LIST	C_NativeUtility::GetBlockIdInt("CCLL")
#define NATIVE_BLOCK_ID_COMPONENT_CLUSTER		C_NativeUtility::GetBlockIdInt("CCLS")
#define NATIVE_BLOCK_ID_POLYGON_CLUSTER			C_NativeUtility::GetBlockIdInt("PCLS")

#define NATIVE_BLOCK_ID_ANIMATION_CURVE_NODE	C_NativeUtility::GetBlockIdInt("ANCV")

#define NATIVE_BLOCK_ID_XREF_SCENE				C_NativeUtility::GetBlockIdInt("XRS ")

#define NATIVE_BLOCK_ID_XREF_LIST				C_NativeUtility::GetBlockIdInt("XRLS")

#define NATIVE_BLOCK_ID_ANIMATION_TIMING		C_NativeUtility::GetBlockIdInt("ANTM")

#define NATIVE_BLOCK_ID_MEMORY_MANAGER_CUES		C_NativeUtility::GetBlockIdInt("MEMC")


#endif
