#ifndef EDITOR_DEFINITIONS_H
#define EDITOR_DEFINITIONS_H

#ifdef _MSC_VER
#pragma once
#endif

namespace AftkRender
{
	class C_SceneGraphNode;
}

namespace EditorLib
{
	class I_DgNode;
	class C_CollisionScene;
	struct C_RayResult;
	struct C_NearestPointResult;
	struct C_CollisionEdge;
	struct C_CollisionProxy;
	struct C_CollisionSpline;
}

namespace Editor
{
	enum E_ManipulatorContextType
	{
		E_ManipulatorContextType_Unknown,

		E_ManipulatorContextType_MoveBoxProp,
		E_ManipulatorContextType_ScaleBoxProp,
		E_ManipulatorContextType_RotateBoxProp,

		E_ManipulatorContextType_MoveSphereProp,
		E_ManipulatorContextType_ScaleSphereProp,

		E_ManipulatorContextType_MoveMatrixProp,
		E_ManipulatorContextType_RotateMatrixProp,
		E_ManipulatorContextType_ScaleMatrixProp,

		E_ManipulatorContextType_MoveSceneNode,
		E_ManipulatorContextType_ScaleSceneNode,
		E_ManipulatorContextType_RotateSceneNode,

		E_ManipulatorContextType_MoveSplineProp,
		E_ManipulatorContextType_ScaleSplineProp,

		E_ManipulatorContextType_MovePolyProp,

		E_ManipulatorContextType_MoveAnimationProp,
		E_ManipulatorContextType_RotateAnimationProp,

		E_ManipulatorContextType_ScaleRectangleProp,

		E_ManipulatorContextType_ScaleRadiusProp,

		E_ManipulatorContextType_Count
	};

	enum E_ManipulatorSpace
	{
		E_ManipulatorSpace_Local,
		E_ManipulatorSpace_World,
	};

	enum E_ManipulatorMode
	{
		E_ManipulatorMode_PointSnap,
		E_ManipulatorMode_EdgeSnap,
		E_ManipulatorMode_GridSnap,
	};

	enum E_ManipulatorType
	{
		E_ManipulatorType_IManipulator,
		E_ManipulatorType_Move,
		E_ManipulatorType_Scale,
		E_ManipulatorType_Rotate,
		E_ManipulatorType_Select,
		E_ManipulatorType_Create,
		E_ManipulatorType_CreateSpline,
		E_ManipulatorType_CreatePoly,
		E_ManipulatorType_CreateAnimation,
		E_ManipulatorType_DropperSelect,

		E_ManipulatorType_Count
	};

	static const char* kVendorName = "Bizarre Creations";
	static const char* kEditorName = "ActorStudio";

	static const char* kRegKeyPath = "HKEY_CURRENT_USER\\Software\\Bizarre Creations\\ActorStudio";

	class I_ManipulatorContext;
	class C_MenuBuilderBase;
	class C_ShortcutHandler;
	class C_MenuBuilder;
	class C_Frame;
	class C_ProgressStatusBar;
	class C_FrameMenuBar;
	class C_DgNodeBase;
	class C_SimpleRenderer;
	class I_View;
}

#endif
