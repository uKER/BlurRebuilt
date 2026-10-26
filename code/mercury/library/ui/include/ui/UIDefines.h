#ifndef UIDEFINES_H
#define UIDEFINES_H

#ifdef _MSC_VER
#pragma once
#endif

// Virtual screen size
#define UI_VSCREEN_SIZEX			(640.0f)
#define UI_VSCREEN_SIZEY			(480.0f)
#define UI_VSCREEN_SIZEX_WIDE		(UI_VSCREEN_SIZEX * 4.0f / 3.0f) // 853.333333

#define LUA_ENUM_TABLE_NAME			"UIEnums"
#define LUA_GLOBAL_TABLE_NAME		"UIGlobals"
#define LUA_TEXT_TABLE_NAME			"UIText"
#define LUA_FONT_TABLE_NAME			"UIFont"
#define LUA_TEXTURE_TABLE_NAME		"UITexture"
#define LUA_SCENE_TABLE_NAME		"UIScene"
#define LUA_TEXTURE_SUBTABLE_NAME	"Textures"
#define LUA_FONTSEQ_TABLE_NAME		"FontSequences"
#define LUA_FONTSTYLE_TABLE_NAME	"FontStyle"
#define LUA_COLUMNTYPE_NAME			"ColumnType"
#define LUA_LAYER_TABLE_NAME		"Layer"
#define LUA_PANEL_TABLE_NAME		"Panel"

#if (defined PLATFORM_PS3) 
// PS3
#define STRCPY(x,y,z)		strncpy( x, z, y )
#define STRCPY_S(x,y,z)		strncpy( x, z, y )
#define STRCAT(x,y,z)		strncat( x, z, y )
#define STRCAT_S(x,y,z)		strncat( x, z, y )
#elif (defined PLATFORM_PC)
// PC
#define STRCPY				strcpy_s
#define STRCPY_S			strcpy_s
#define STRCAT				strcat_s
#define STRCAT_S			strcat_s
#else
// 360 and everything else
#define STRCPY				strcpy_s
#define STRCPY_S			strcpy_s
#define STRCAT				strcat_s
#define STRCAT_S			strcat_s
#endif

namespace UI
{

/// @addtogroup UI_Types
/// @{

// platform types
enum E_UiPlatform
{
	E_UiPlatform_Unknown = 0,
	E_UiPlatform_Xenon,
	E_UiPlatform_PC,
	E_UiPlatform_PS3,
};

// profile/save system errors
enum E_UiGameProfile_Error
{
	E_UiGameProfile_Error_None = -1,	// used in a debug enum

	E_UiGameProfile_Error_Okay = 0,
	E_UiGameProfile_Error_FileDoesNotExist,
	E_UiGameProfile_Error_FileExists,
	E_UiGameProfile_Error_CannotEnumerateFiles,
	E_UiGameProfile_Error_CannotReadFile,
	E_UiGameProfile_Error_CannotWriteFile,
	E_UiGameProfile_Error_CannotDeleteFile,
	E_UiGameProfile_Error_DeviceNotConnected,
	E_UiGameProfile_Error_NoFreeSpace,
	E_UiGameProfile_Error_UserCancelled,
	E_UiGameProfile_Error_DifferentOwner,

	E_UiGameProfile_Error_Count,
};

enum SB_COLUMN_TYPE
{
	SB_COLUMN_TYPE_STRING,
	SB_COLUMN_TYPE_POS,
	SB_COLUMN_TYPE_WIDE_STRING,
	SB_COLUMN_TYPE_NUMBER,
	SB_COLUMN_TYPE_TIME,
	SB_COLUMN_TYPE_TIME_AND_DATE,
	SB_COLUMN_TYPE_CASH,

	SB_COLUMN_TYPE_END_MARKER
};

/// Gadget render justication
enum E_Justify
{
	// Top (and text)
	E_Justify_TopLeft,				///< @a
	E_Justify_TopCentre,			///< @a
	E_Justify_TopRight,				///< @a
	// Middle
	E_Justify_MiddleLeft,			///< @a
	E_Justify_MiddleCentre,			///< @a
	E_Justify_MiddleRight,			///< @a
	// Bottom
	E_Justify_BottomLeft,			///< @a
	E_Justify_BottomCentre,			///< @a
	E_Justify_BottomRight,			///< @a
	// None
	E_Justify_None,					///< No justification (currently just used to stop parenting to screen)

	E_Justify_Count
};

typedef struct UIMessage
{
	char	Type[32];
	F32		values[4];
} UIMessage;

typedef S32 UIButtonTypeID;

enum E_UIButtonType
{
	E_UIButtonType_None = -1,			///< Dont try and create a gadget from this.
	E_UIButtonType_Dummy = 0,			///< @a
	E_UIButtonType_SafeArea,			///< @a
	E_UIButtonType_RenderState,			///< @a
	E_UIButtonType_StaticGraphic,		///< @a
	E_UIButtonType_StaticLetter,		///< @a
	E_UIButtonType_StaticText,			///< @a
	E_UIButtonType_StaticScene,			///< @a
	E_UIButtonType_StaticShape,			///< @a
	E_UIButtonType_StaticBox,			///< @a
	E_UIButtonType_Panel,				///< @a
	E_UIButtonType_VsSpinner,			///< @a
	E_UIButtonType_HorizontalMenu,		///< @a
	E_UIButtonType_VerticalMenu,		///< @a
	E_UIButtonType_GroupStart,			///< @a
	E_UIButtonType_GroupEnd,			///< @a
	E_UIButtonType_ParticleEmitter,		///< @a
	E_UIButtonType_Credits,				///< @a
	E_UIButtonType_Slider,				///< @a
	E_UIButtonType_ScrollBar,			///< @a
	E_UIButtonType_Camera,				///< @a
	E_UIButtonType_ItemSpinner,			///< @a
	E_UIButtonType_Path,				///< @a
	E_UIButtonType_RenderBranch,		///< @a
	E_UIButtonType_DummyLerp,			///< @a
	E_UIButtonType_FriendsList,			///< @a
	E_UIButtonType_AdobeBox,			///< @a
	E_UIButtonType_AdobeTrail,			///< @a
	E_UIButtonType_AdobeAnimation,		///< @a
	E_UIButtonType_Keyboard,			///< @a
	E_UIButtonType_NodeList,			///< @a
	E_UIButtonType_Node,				///< @a
	E_UIButtonType_Model,				///< @a
	E_UIButtonType_Graph,				///< @a
	E_UIButtonType_ProgressBar,			///< @a
	E_UIButtonType_Legal,				///< @a
	E_UIButtonType_Table,				///< @a

	E_UIButtonType_EndMarker			///< Use this for the start of the user list
};

enum SCREEN_TIME
{
	SCREEN_TIME_NORMAL,
	SCREEN_TIME_START,
	SCREEN_TIME_END,

	SCREEN_TIME_MAX
};

enum EFFECT_TYPE
{
	EFFECT_TYPE_MOVE,			///< Move XYZ - use [fromx, fromy, fromz : tox, toy, toz]
	EFFECT_TYPE_MOVEX,			///< Move X - use [from : to]
	EFFECT_TYPE_MOVEY,			///< Move Y - use [from : to]
	EFFECT_TYPE_MOVEZ,			///< Move Z - use [from : to]
	EFFECT_TYPE_COLOUR,			///< Set alpha and colour channels - use [from : to]
	EFFECT_TYPE_ALPHA,			///< Set alpha channel - use [from : to]
	EFFECT_TYPE_SIZE,			///< Set width, height and depth - use [fromx, fromy, fromz : tox, toy, toz]
	EFFECT_TYPE_SIZEX,			///< Set width - use [from : to]
	EFFECT_TYPE_SIZEY,			///< Set height - use [from : to]
	EFFECT_TYPE_SCALE,			///< Set XYZ scale - use [fromx, fromy, fromz : tox, toy, toz]
	EFFECT_TYPE_SCALEX,			///< Set X scale - use [from : to]
	EFFECT_TYPE_SCALEY,			///< Set Y scale - use [from : to]
	EFFECT_TYPE_TEXTUREMOVE0,	///< Move UV coordinates in channel 0 - use [fromx, fromy : tox, toy]
	EFFECT_TYPE_TEXTUREMOVE1,	///< Move UV coordinates in channel 1 - use [fromx, fromy : tox, toy]
	EFFECT_TYPE_TEXTUREMOVE2,	///< Move UV coordinates in channel 2 - use [fromx, fromy : tox, toy]
	EFFECT_TYPE_TEXTURESIZE0,	///< Alter UV size in channel 0 - use [fromx, fromy : tox, toy]
	EFFECT_TYPE_ROTATE,			///< Eular rotate - use [fromx, fromy, fromz : tox, toy, toz]
	EFFECT_TYPE_ANIMATE,		///< ?
	EFFECT_TYPE_COLOUR0,		///< Change top left colour - use [from : to]
	EFFECT_TYPE_COLOUR1,		///< Change top right colour - use [from : to]
	EFFECT_TYPE_COLOUR2,		///< Change bottom right colour - use [from : to]
	EFFECT_TYPE_COLOUR3,		///< Change bottom left colour - use [from : to]

	EFFECT_TYPE_MAX
};

enum EFFECT_WHEN
{
	EFFECT_WHEN_START,
	EFFECT_WHEN_END,
	EFFECT_WHEN_MAINLOOP,
	EFFECT_WHEN_VARIABLE,
	EFFECT_WHEN_SELECTED,
	EFFECT_WHEN_LOCKED,
	EFFECT_WHEN_LOCKED_AND_SELECTED,
	EFFECT_WHEN_SB_TOP,
	EFFECT_WHEN_SB_YOU,

	EFFECT_WHEN_MAX
};

enum UI_GROUP_TYPES
{
	UI_GROUP_TYPES_OFFSET,
	UI_GROUP_TYPES_SIZE,
	UI_GROUP_TYPES_SCALE,
	UI_GROUP_TYPES_ROTATE,
	UI_GROUP_TYPES_PIVOT_OFFSET,

	UI_GROUP_TYPES_MAX
};

/// @}

}; // namespace UI

#endif
