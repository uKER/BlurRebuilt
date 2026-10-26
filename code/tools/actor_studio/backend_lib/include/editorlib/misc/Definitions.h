#pragma once

#ifndef EDITOR_LIB_DEFINITIONS_H
#define EDITOR_LIB_DEFINITIONS_H

#include <EditorLib/Misc/NodeId.h>

namespace Aftk
{
	class C_Scene;
	class C_SceneNode;
	class C_SceneMeshNode;
	class C_SceneSplineNode;
	class C_DynamicPlug;
	class C_GenericAttribute;
}

namespace Util
{
	enum E_VarType;
}

namespace EditorLib
{
	enum E_DirtyEventType
	{
		E_DirtyEventType_Unknown,
		E_DirtyEventType_Patch
	};

	enum E_AppType
	{
		E_AppType_Unknown,
		E_AppType_Interactive,
		E_AppType_Batch
	};

	enum E_NearestPointFilter
	{
		E_NearestPointFilter_Vertex,
		E_NearestPointFilter_Edge,

		E_NearestPointFilter_Count
	};

	#define DEF(e, str) e,
	enum E_ObjType
	{
		#include <Editor_defs/ObjectTypeMapping.h>
		E_ObjType_Count
	};
	#undef DEF

	enum E_LevelType
	{
		E_LevelType_LevelBase,
		E_LevelType_ReferenceLevel,
		E_LevelType_XtLevel,
	};

	enum E_RotationOrder
	{
		E_RotationOrder_XYZ,
		E_RotationOrder_YZX,
		E_RotationOrder_ZXY,
		E_RotationOrder_XZY,
		E_RotationOrder_YXZ,
		E_RotationOrder_ZYX,
	};

	enum E_Layer_Owner
	{
		E_LayerOwner_None,
		E_LayerOwner_Level,
	};

	enum E_LayerSetType
	{
		E_LayerSetType_Virtual,
		E_LayerSetType_Xt,
	};

	enum E_LayerType
	{
		E_LayerType_Unknown,
		E_LayerType_Reference,
		E_LayerType_Actor
	};

	enum E_LevelEvent;
	enum E_DatabaseEvent;
	enum E_LayerEvent;
	enum E_DgNodeTransform;

	class I_DgObject;
	class C_SceneNode;
	class C_Syntax;
	class C_Argument;
	class C_ArgumentDatabase;
	class C_GuidManager;
	class C_CommandFactory;
	class C_CommandProcessor;
	class I_Layer;
	class I_LayerSet;
	class C_LayerManager;
	class I_Level;
	class C_LevelBase;
	class C_XtLevel;
	class C_LevelPack;
	class C_LevelManager;
	class C_UndoManager;
	class C_XtManager;
	class C_XtTypeDatabase;
	class C_XtVisualisationData;
	class C_DgManager;
	class I_DgNode;
	class C_AftkRefManager;
	class C_AftkRef;
	class C_Clipboard;
	class C_Selection;
	class C_ArgumentDatabase;
	class C_ArgumentParser;
	class I_Plug;
	class C_NullPlug;
	class C_DynamicPlug;
	class C_DgNode;
	class C_SceneNodeHelpers;
	class C_SceneAftkNode;
	class C_Core;
	class C_SceneXtActorNode;
	class C_SceneXtNode;
	struct C_GlobalSignals;
	class I_XtType;
	class C_XtActorType;
	class C_XtDgActorType;
	class C_XtSceneActorType;
	class C_XtActorComponentType;
	class C_DgXtPropNodeBase;
	class C_XtObject;
	class C_DgXtCompNode;
	class C_CollisionManager;
	class C_CollisionScene;
	class C_PythonManager;
	class C_NetworkManager;
	class C_VerificationSystem;
	class I_UpdateHint;
	class I_ImmediateHint;
	class C_SceneXtNodeBase;
	class C_XtUserData;
	struct C_ImmediateHintCollection;
	struct C_UpdateHintCollection;
	struct C_CoreConfig;
	struct I_CollisionSource;
	struct C_Change;
	struct C_XtDomUpdate;
	template<typename T> class C_StaticPlug;
	typedef C_Factory<I_DgNode>					C_DgNodeFactory;
	typedef Btl::Vector<Util::E_VarType>		VarTypeArray;
	
	struct C_AftkLevelRef
	{
		const C_AftkRef* pAftkRef;
		bool isExtActor;
	};

	typedef Ut::UtList<C_AftkLevelRef*>			AftkLevelRefList;
	typedef Ut::UtList<C_AftkRef*>				AftkRefList;
	typedef Ut::UtList<const C_AftkRef*>		ConstAftkRefList;
	typedef Btl::OpenHashMap<const I_DgNode*, I_Layer*> DgLayerMap;
	typedef Btl::OpenHashMap<const I_Level*, I_LayerSet*> LayerSetMap;
	typedef Ut::UtList<I_Layer*>				LayerList;

	typedef UtMaths::MVector<UtMaths::VectorXYZ<F32> >					Vector3;
	typedef UtMaths::MVector<UtMaths::VectorXYZW<F32> >					Vector4;
	typedef UtMaths::MMatrix<UtMaths::BasicMatrix<Vector4, 4> >			Matrix44;

	typedef Ut::OpenHashMap<Aftk::C_SceneNode*, I_DgNode*, Ut::HashKeyOpPtr> 
																		AftkDgMap;
	typedef Ut::OpenHashMap<C_NodeId, I_DgNode*, HashKeyOpNodeId > 
																		DgIdPtrMap;

	typedef Btl::OpenHashSet<C_NodeId, HashKeyOpNodeId > 
																		DgIdSet;

	typedef Btl::Vector<C_NodeId>										NodeIdArray;

	typedef Btl::Vector<Sys::WeakPtr<C_DgXtCompNode> >			DgActorComponentArray;
	typedef Btl::Vector<const Sys::WeakPtr<C_DgXtCompNode> >	ConstDgActorComponentArray;

	static XtId	kXtInvalidId		= 0xffffffff;
	static XtId	kInvalidId			= 0xffffffff;
}

#endif
