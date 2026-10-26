/////////////////////////////////////////////////////////////
// Debug stuff
/////////////////////////////////////////////////////////////

#include "VehiclePCH.h"

#include "Vehicle/Livery/Canvas.h"

#if defined(TURN_OFF_VEHICLE_OPTIMISATIONS)
	#pragma optimize("", off)
#endif

DEBUG_ENUM(Vehicle::Livery::E_Canvas)
	DE_OPTION(Vehicle::Livery::E_Canvas_Front,				"Front")
	DE_OPTION(Vehicle::Livery::E_Canvas_Rear,				"Rear")
	DE_OPTION(Vehicle::Livery::E_Canvas_Left,				"Left")
	DE_OPTION(Vehicle::Livery::E_Canvas_Right,				"Right")	
	DE_OPTION(Vehicle::Livery::E_Canvas_Top,				"Top")
END_DEBUG_ENUM

namespace Vehicle
{

namespace Livery
{

//LiveryEditor
Debug::Float g_debugAmaxLiveryFlashMinTime("Vehicle.Livery.Edit.FlashMinTime", 45.0f, Debug::Range(0.0f, 240.0f), Debug::Step(1.0f));
Debug::Float g_debugAmaxLiveryFlashMaxTime("Vehicle.Livery.Edit.FlashMaxTime", 53.0f, Debug::Range(0.0f, 240.0f), Debug::Step(1.0f));

Debug::Enum<E_Canvas> g_debugAmaxLiveryCurrentCanvas("Vehicle.Livery.Edit.CurrentCanvas", E_Canvas_Front);

Debug::Bool g_debugAmaxLiverySave("Vehicle.Livery.FileManagement.Save", false);
Debug::Bool g_debugAmaxLiveryLoad("Vehicle.Livery.FileManagement.Load", false);
Debug::Bool g_debugLiveryForceLookAt("Vehicle.Livery.Edit.ForceLookAt", false);
Debug::Bool g_debugAmaxLiveryClearAllCanvases("Vehicle.Livery.Edit.ClearAll", false);

Debug::Bool g_debugAmaxLiveryRenderCanvas("Vehicle.Livery.Edit.RenderCanvas", false);

Debug::Bool g_debugAmaxLiveryAddLayer("Vehicle.Livery.Edit.AddLayer", false);
Debug::Bool g_debugAmaxLiveryRemoveLayer("Vehicle.Livery.Edit.RemoveLayer", false);
Debug::Bool g_debugAmaxLiveryNextLayer("Vehicle.Livery.Edit.NextLayer", false);
Debug::Bool g_debugAmaxLiveryPrevLayer("Vehicle.Livery.Edit.PreviousLayer", false);
Debug::Bool g_debugAmaxLiveryCopyLayer("Vehicle.Livery.Edit.CopyLayer", false);
Debug::Bool g_debugAmaxLiveryPasteLayer("Vehicle.Livery.Edit.PasteLayer", false);

Debug::Bool g_debugAmaxLiveryFlipHoriz("Vehicle.Livery.Edit.FlipHorizontal", false);
Debug::Bool g_debugAmaxLiveryFlipVert("Vehicle.Livery.Edit.FlipVertical", false);
Debug::Bool g_debugAmaxLiveryMirrorHoriz("Vehicle.Livery.Edit.MirrorHorizontal", false);
Debug::Bool g_debugAmaxLiveryMirrorVert("Vehicle.Livery.Edit.MirrorVertical", false);
	
Debug::Bool g_debugAmaxLiveryClearCanvas("Vehicle.Livery.Edit.ClearCanvas", false);

Debug::Int g_debugAmaxLiveryPosX("Vehicle.Livery.Debug.PosX", 345, Debug::Range(0, 1280), Debug::Step(1));
Debug::Int g_debugAmaxLiveryPosY("Vehicle.Livery.Debug.PosY", 72, Debug::Range(0, 1280), Debug::Step(1));
Debug::Bool g_debugDisplayCanvasTexture("Vehicle.Livery.Debug.DisplayCanvasTexture", false);

Debug::String g_debugLiverySaveName("Vehicle.Livery.FileManagement.SaveName", "my_livery");
Debug::String g_debugLiveryLoadName("Vehicle.Livery.FileManagement.LoadName", "my_livery");

//Canvas

Debug::Bool g_debugAmaxLiveryPromoteLayer("Vehicle.Livery.Edit.PromoteLayer", false);
Debug::Bool g_debugAmaxLiveryDemoteLayer("Vehicle.Livery.Edit.DemoteLayer", false);

Debug::Float g_debugAmaxLiveryFlashScaleR("Vehicle.Livery.FlashR", 0.0f, Debug::Range(0.0f, 5.0f), Debug::Step(0.001f));
Debug::Float g_debugAmaxLiveryFlashScaleG("Vehicle.Livery.FlashG", 5.0f, Debug::Range(0.0f, 5.0f), Debug::Step(0.001f));
Debug::Float g_debugAmaxLiveryFlashScaleB("Vehicle.Livery.FlashB", 0.0f, Debug::Range(0.0f, 5.0f), Debug::Step(0.001f));

//Layer

Debug::Bool g_debugAmaxLiveryUpdateCanvas("Vehicle.Livery.Edit.UpdateCanvas", false);

Debug::Float g_debugAmaxLiveryLiveryPositionX("Vehicle.Livery.Edit.LiveryPositionX", 0.0f, Debug::Range(-2.0f, 2.0f), Debug::Step(0.001f));
Debug::Float g_debugAmaxLiveryLiveryPositionY("Vehicle.Livery.Edit.LiveryPositionY", 0.0f, Debug::Range(-2.0f, 2.0f), Debug::Step(0.001f));
Debug::Float g_debugAmaxLiveryLiverySizeX("Vehicle.Livery.Edit.LiverySizeX", 2.0f, Debug::Range(-10.0f, 10.0f), Debug::Step(0.001f));
Debug::Float g_debugAmaxLiveryLiverySizeY("Vehicle.Livery.Edit.LiverySizeY", 2.0f, Debug::Range(-10.0f, 10.0f), Debug::Step(0.001f));
Debug::Int g_debugAmaxLiveryLiveryRotationZ("Vehicle.Livery.Edit.LiveryRotationZ", 0, Debug::Range(0, 360), Debug::Step(1));
Debug::Int g_debugAmaxLiveryLiveryTextureIndex("Vehicle.Livery.Edit.TextureIndex", 0, Debug::Range(0, 64), Debug::Step(1));
Debug::Colour g_debugAmaxLiveryLiveryColour("Vehicle.Livery.Edit.LiveryColour", 1.0f, 1.0f, 1.0f, 1.0f);

} //namespace Livery

} //namespace Vehicle
