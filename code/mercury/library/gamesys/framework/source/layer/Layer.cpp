#include "FrameworkPch.h"
#include "Framework/Layer/Layer.h"

namespace Framework
{

// C_Layer

C_Layer::C_Layer(const char* name)
	: m_LayerState( E_LayerState_Constructed )
	, m_Enabled( true )
	, m_Name( name )
{
	LOGINFO( g_Log, "Constructed layer %s", m_Name);
}

C_Layer::~C_Layer()
{
	LOGINFO( g_Log, "Destroyed layer %s", m_Name);
}

const char* C_Layer::GetName() const
{
	return m_Name;
}

E_LayerState C_Layer::GetLayerState() const
{
	return m_LayerState;
}

void C_Layer::SetLayerState( const E_LayerState layerState )
{
	LOGINFO( g_Log, "Setting layer state %s %i", m_Name, layerState);
	m_LayerState = layerState;
}

S32 C_Layer::GetLayerPriority() const
{
	return E_LayerPriority_Default;
}

void C_Layer::EnableLayer()
{
	m_Enabled = true;
}

void C_Layer::DisableLayer()
{
	m_Enabled = false;
}


}; // namespace Framework
