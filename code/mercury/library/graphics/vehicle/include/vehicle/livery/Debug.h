/////////////////////////////////////////////////////////////
// Debug stuff
/////////////////////////////////////////////////////////////

#ifndef __LIVERY_DEBUG_H__
#define __LIVERY_DEBUG_H__

#include "Vehicle/Livery/Canvas.h"

namespace Vehicle
{

namespace Livery
{

//LiveryEditor
extern Debug::Float g_debugAmaxLiveryFlashMinTime;
extern Debug::Float g_debugAmaxLiveryFlashMaxTime;

extern Debug::Enum<E_Canvas> g_debugAmaxLiveryCurrentCanvas;

extern Debug::Bool g_debugAmaxLiverySave;
extern Debug::Bool g_debugAmaxLiveryLoad;
extern Debug::Bool g_debugLiveryForceLookAt;
extern Debug::Bool g_debugAmaxLiveryClearAllCanvases;

extern Debug::Bool g_debugAmaxLiveryRenderCanvas;

extern Debug::Bool g_debugAmaxLiveryAddLayer;
extern Debug::Bool g_debugAmaxLiveryRemoveLayer;
extern Debug::Bool g_debugAmaxLiveryNextLayer;
extern Debug::Bool g_debugAmaxLiveryPrevLayer;
extern Debug::Bool g_debugAmaxLiveryCopyLayer;
extern Debug::Bool g_debugAmaxLiveryPasteLayer;

extern Debug::Bool g_debugAmaxLiveryFlipHoriz;
extern Debug::Bool g_debugAmaxLiveryFlipVert;
extern Debug::Bool g_debugAmaxLiveryMirrorHoriz;
extern Debug::Bool g_debugAmaxLiveryMirrorVert;
	
extern Debug::Bool g_debugAmaxLiveryClearCanvas;

extern Debug::Int g_debugAmaxLiveryPosX;
extern Debug::Int g_debugAmaxLiveryPosY;
extern Debug::Bool g_debugDisplayCanvasTexture;

extern Debug::String g_debugLiverySaveName;
extern Debug::String g_debugLiveryLoadName;

//Canvas

extern Debug::Bool g_debugAmaxLiveryPromoteLayer;
extern Debug::Bool g_debugAmaxLiveryDemoteLayer;

extern Debug::Float g_debugAmaxLiveryFlashScaleR;
extern Debug::Float g_debugAmaxLiveryFlashScaleG;
extern Debug::Float g_debugAmaxLiveryFlashScaleB;

//Layer

extern Debug::Bool g_debugAmaxLiveryUpdateCanvas;

extern Debug::Float g_debugAmaxLiveryLiveryPositionX;
extern Debug::Float g_debugAmaxLiveryLiveryPositionY;
extern Debug::Float g_debugAmaxLiveryLiverySizeX;
extern Debug::Float g_debugAmaxLiveryLiverySizeY;
extern Debug::Int g_debugAmaxLiveryLiveryRotationZ;
extern Debug::Int g_debugAmaxLiveryLiveryTextureIndex;
extern Debug::Colour g_debugAmaxLiveryLiveryColour;

} //namespace Livery

} //namespace Vehicle

#endif
