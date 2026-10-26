#ifndef MAYA_IO_STRING_TABLE_H
#define MAYA_IO_STRING_TABLE_H

#ifdef _MSC_VER_
#pragma once
#endif

#define TRANS_ACQUIRE_PLUG_FAIL					"Failed to acquire plug."
#define TRANS_ACQUIRE_OBJECT_FAIL				"Failed to acquire MObject."
#define TRANS_ACQUIRE_DAG_PATH_FAIL				"Failed to acquire DAG path."
#define TRANS_ACQUIRE_DAG_IT_FAIL				"Failed to acquire DAG iterator."
#define TRANS_ACQUIRE_DAG_NODE_FAIL				"Failed to acquire DAG node."
#define TRANS_ACQUIRE_SELECTION_LIST_FAIL		"Failed to get active selection list."
#define TRANS_EXTEND_TO_SHAPE_FAIL				"Failed to extend DAG path to shape."

#define TRANS_EXECUTE_COMMAND_FAIL				"Failed to execute command."

#define TRANS_CONNECT_PLUGS_FAIL				"Failed to connect plugs."

#define TRANS_QUERY_PLUG_FAIL					"Failed to query plug."
#define TRANS_QUERY_SELECTION_LIST_FAIL			"Failed to query active selection list."
#define TRANS_QUERY_SCENE_NODE_EXCLUSION_LIST_FAIL	"Failed to query scene node export exclusion list."

#define TRANS_EXPORT_BONED_INSTANCE_FAIL		"Failed to export boned instance node."
#define TRANS_EXPORT_SPEED_TREE_FAIL			"Failed to export speed tree node."

#define TRANS_OUT_OF_MEMORY						"Out of memory."
#define TRANS_NEW_MESH_OUT_OF_MEMORY			"Out of memory allocating new mesh node."
#define TRANS_NEW_BONED_INSTANCE_OUT_OF_MEMORY	"Out of memory allocating new boned instance."
#define TRANS_NEW_SPEED_TREE_OUT_OF_MEMORY		"Out of memory allocating new speed tree node."
#define TRANS_NEW_INSTANCE_OUT_OF_MEMORY		"Out of memory allocating new instance node."

#define TRANS_SCENE_PRE_PROCESS_FAIL			"Failed to pre-process scene."
#define TRANS_RESOLVE_BONED_INSTANCES_FAIL		"Failed to resolve boned instances."

#define TRANS_EXPORT_AFTK_MAYA_TO_MAYA_DAG_PATH_FAIL	"Failed to get Aftk node from Maya DAG Path."
#define TRANS_EXPORT_MAYA_DAG_PATH_TO_AFTK_MAYA_FAIL	"Failed to get Maya DAG Path from Aftk node."
#define TRANS_EXPORT_SHAPE_TOP_LEVEL_FAIL		"Failed to add shape at DAG top level."

#define TRANS_BONED_INSTANCE_PARENT_INVALID			"Boned instance parent is invalid."
#define TRANS_BONED_INSTANCE_PARENT_INVALID_TYPE	"Boned instance parent is of an invalid type."

#define TRANS_ACQUIRE_BIND_POSE_FAIL				"Failed to acquire bind pose from transform."
#define TRANS_REVERT_BIND_POSE_FAIL				"Could not revert node to base pose."

#define TRANS_SYNC_MESH_FAIL					"Failed to sync mesh node." 
#define TRANS_LOD_THRESHOLD_LOD_LEVEL_MISMATCH	"The number of lod thresholds and lod levels does not match."

#define TRANS_LOD_INSTANCED_ERROR				"Failed to export LOD level. One of the LOD levels has been instanced. This is not allowed."
#define	TRANS_EXPORT_INSTANCED_LOD_MISMATCH		"Failed to export instanced LOD group. One or more of the LOD group children was not instanced."
#define TRANS_COPY_LOD_GROUP_PROPORTIES_FAIL	"Failed to copy LOD group transform proporties."
#define TRANS_DAG_PATH_FROM_NAME_STRING			"Failed to get Maya DAG path from node name string."
#define TRANS_INSUFFICIENT_BONES_FOR_BONED_INSTANCE		"Insufficient bones to export boned instance."
#define TRANS_TOO_MANY_BONES_FOR_BONED_INSTANCE			"Too many bones for boned instance."
#define TRANS_DAG_NODE_FROM_DAG_STRING_FAIL			"Failed to get DAG node from DAG string."
#define TRANS_AFTK_NODE_FROM_DAG_PATH_FAIL			"Failed to get AFTK node from DAG path."
#define TRANS_MAYA_CLIP_NAME_FROM_AFTK_CLIP_NODE_FAIL	"Failed to get Maya clip name from Aftk clip node."
#define TRANS_AFTK_CLIP_NODE_FROM_MAYA_CLIP_NAME_FAIL	"Failed to get Aftk clip node from Maya clip name."
#define TRANS_MAYA_CLIP_SOURCE_NAME_FROM_AFTK_CLIP_SOURCE_NODE_FAIL "Failed to get Maya clip source name from Aftk clip node."
#define TRANS_AFTK_CLIP_SOURCE_NODE_FROM_MAYA_CLIP_SOURCE_NAME_FAIL "Failed to get Maya clip source name from Aftk clip node."
#define TRANS_MAYA_CLIP_MIXER_NAME_FROM_AFTK_CLIP_MIXER_NODE_FAIL	"Failed to get Maya clip mixer name from Aftk clip mixer node"
#define TRANS_AFTK_CLIP_MIXER_NODE_FROM_MAYA_CLIP_MIXER_NAME_FAIL	"Failed to get Aftk clip mixer node from Maya clip mixer name"
#define TRANS_DAG_PATH_FROM_NAME_STRING_FAIL	"Failed to get DAG path from name string."
#define TRANS_DAG_PATH_FROM_AFTK_NODE_FAIL		"Failed to get DAG path from Aftk node."

#define TRANS_QUERY_REFERENCE_FILE_NAME_FAIL	"Failed to get reference file name."
#define TRANS_XREF_LOOKUP_FAIL					"Failed to find x-ref by file name."
#define TRANS_RENAME_FILE_EXTENSION_FAIL		"Failed to rename file extension."
#define TRANS_RENAME_FILE_PATH_FAIL				"Failed to rename file path."
#define TRANS_COPY_XREF_FILE_FAIL				"Failed to x-ref copy file."

#define TRANS_MULTIMATERIAL_FAIL "A polygon on node [" << pAftkSceneMeshNode->GetName() << "] was assigned to more than one material. Run the fix corrupt texture script."
#define TRANS_MULTIMATERIAL_WARN "A polygon on node [" << pAftkSceneMeshNode->GetName() << "] was assigned to more than one material. Run the fix corrupt texture script."
#define TRANS_ZERO_POLY_MESH_FAIL				"Failed to export mesh " << fnMesh.name().asChar() << " as it was reported to have no polygons. Please delete this object in order to continue."
#define TRANS_SET_DIRECTORY_FAIL				"Failed to set directory."
#define TRANS_EXPORT_REFERENCE_FAIL				"Failed to export reference."

#define TRANS_MISSING_TEXTURES_FAIL				"Export aborted due to missing textures."
#define TRANS_MISSING_TEXTURES_WARN				"Missing textures, error ignored."
#define TRANS_CORRUPT_NORMAL_FAIL				"Failed to export [" << pAftkSceneNode->GetName() << "] as it contains invalid normals."
#define TRANS_CORRUPT_NORMAL_WARN				"Node [" << pAftkSceneNode->GetName() << "] contains invalid normals. Set to zero."

#define TRANS_INFINITE_POSITION_FAIL			"Failed to export [" << pAftkSceneNode->GetName() << "] as it contains corrupt positions."
#define TRANS_INFINITE_POSITION_WARN			"Node [" << pAftkSceneNode->GetName() << "] contains corrupt positions. This node will be automatically removed by the exporter."

#define TRANS_INFINITE_NORMAL_FAIL				"Failed to export [" << pAftkSceneNode->GetName() << "] as it contains corrupt normals."
#define TRANS_INFINITE_NORMAL_WARN				"Node [" << pAftkSceneNode->GetName() << "] contains corrupt normals."

#define TRANS_INFINITE_UV_FAIL					"Failed to export [" << pAftkSceneNode->GetName() << "] as it contains corrupt UVs."
#define TRANS_INFINITE_UV_WARN					"Node [" << pAftkSceneNode->GetName() << "] contains corrupt UVs."

#define TRANS_INFINITE_MATRIX_FAIL				"Failed to export [" << fnTransform.name().asChar() << "] as it has a matrix with infinite numbers. Set to identity."
#define TRANS_INFINITE_MATRIX_WARN				"Node [" << fnTransform.name().asChar() << "] has a matrix with infinite numbers. Set to identity."
#define TRANS_ACQUIRE_FN_FAIL					"Failed to acquire function set for node [" << pAftkSceneNode->GetName() << "]."
#define TRANS_ACQUIRE_FN_WARN					"Failed to acquire function set for node [" << pAftkSceneNode->GetName() << "]. This node will be automatically removed by the exporter."
#define TRANS_FIND_BONED_INSTANCE_PARENT_FAIL   "Could not find boned instance parent [" << toFind << "]. Check how you're referencing in the boned instance source scene."

#define TRANS_INVERTED_MATRIX_FAIL				"Instance [" << pInstance->GetName() << "] has a flipped matrix relative to [" << pInstance->GetSourceNodePtr()->GetName() << "] and will appear inside out."
#define TRANS_INVERTED_MATRIX_WARN				"Instance [" << pInstance->GetName() << "] has a flipped matrix relative to [" << pInstance->GetSourceNodePtr()->GetName() << "] and will appear inside out. Error ignored."

#define TRANS_FIND_SPEED_TREE_NODE_WARN			"Failed to find speed-tree DG node for DAG node [" << mFnShape.name().asChar() << "]."
#define TRANS_TOO_MANY_UV_SETS					"Meshes with > " << MAYA_IO_MAX_UV_SETS << " UV sets are not supported."


#endif
