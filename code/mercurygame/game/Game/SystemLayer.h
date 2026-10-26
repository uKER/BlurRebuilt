#ifndef SYSTEM_LAYER_H
#define SYSTEM_LAYER_H

#ifdef _MSC_VER
#pragma once
#endif

#include <Framework/Layer/Layer.h>
#include <Game/Gamedef/GameData.h>

namespace DebugSystems
{
	class C_ProfilerView;
}

namespace Game
{

class C_SystemLayer : public Framework::C_Layer
{
public:

	C_SystemLayer();
	virtual ~C_SystemLayer();

	// For layer messages
	HRESULT ProcessMessage(const Framework::E_LayerMessage message);

	HRESULT Create();
	HRESULT Destroy();
	HRESULT Update();
	HRESULT GlobalUpdate();
	HRESULT RenderSplitscreen();
	HRESULT RenderFullscreen();


private:
	C_GameData m_GameData;
	DebugSystems::C_ProfilerView* m_ProfilerView;
};

} // namespace Game

#endif // SYSTEM_LAYER_H
