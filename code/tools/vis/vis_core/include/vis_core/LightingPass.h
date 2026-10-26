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

	bool			DoesLayerPVS(const std::string &layerName);
	bool			DoesLayerOcclude(const std::string &layerName);

	Ut::Vector<Element>	m_LayersToLight;
	Ut::Vector<Element>	m_LayersThatCastShadows;
	Util::C_String		m_name;

	bool			m_bLightAllLayersByDefault;
	bool			m_bCastShadowsOnAllLayersByDefault;
};

