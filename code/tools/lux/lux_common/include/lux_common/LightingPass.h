#pragma once

#include "Aftk/Aftk.h"

class LightingPass
{
public:

	struct Element
	{
		std::string		name;
		bool			bOn;
	};

					LightingPass() : m_bLightAllLayersByDefault(true), m_bCastShadowsOnAllLayersByDefault(true) {};

	void			ReadXMLNode(XERCES_CPP_NAMESPACE::DOMNode *pNode);

	bool			DoesLayerNeedLighting(const std::string &layerName);
	bool			DoesLayerCastShadows(const std::string &layerName);

	bool			DoesNodeNeedLighting(Aftk::C_SceneNode *pNode);

	Ut::Vector<Element>	m_LayersToLight;
	Ut::Vector<Element>	m_LayersThatCastShadows;
	Util::C_String		m_name;

	bool			m_bLightAllLayersByDefault;
	bool			m_bCastShadowsOnAllLayersByDefault;
};

