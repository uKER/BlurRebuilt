#include "stdafx.h"

#include <core/containers/string.h>
#include "vis_core/LightingPass.h"

using namespace XERCES_CPP_NAMESPACE;

// read lighting pass information
void LightingPass::ReadXMLNode(XERCES_CPP_NAMESPACE::DOMNode *pNode)
{
    if (!pNode) return;
    if (pNode->getNodeType() != DOMNode::ELEMENT_NODE) return;

	char *pElementName = XMLString::transcode(pNode->getNodeName());
	if (stricmp(pElementName, "pass") == 0)
	{
		// does this pass have a name?
		DOMNamedNodeMap *pAttributes = pNode->getAttributes();
		int nSize = pAttributes->getLength();

		// if there is an attribute, assume that that's the name
		if (nSize)
		{
			DOMAttr *pAttributeNode = (DOMAttr*) pAttributes->item(0);
			char *pName = XMLString::transcode(pAttributeNode->getName());
			if (stricmp(pName, "name") == 0)
			{
				// get value of name attribute
				char *data = XMLString::transcode(pAttributeNode->getValue());
				m_name = data;
			    XMLString::release(&data);
			}
			XMLString::release(&pName);
		}

		// iterate through children - these should be the sampling points
        for (DOMNode *child = pNode->getFirstChild(); child != 0; child = child->getNextSibling())
		{
			char *pElementName = XMLString::transcode(child->getNodeName());
		    if (child->getNodeType() != DOMNode::ELEMENT_NODE) 
			{
			    XMLString::release(&pElementName);
				continue;
			}

			// we don't care about the node name
			// get all the attributes of the node
			DOMNamedNodeMap *pAttributes = child->getAttributes();
			int nSize = pAttributes->getLength();

			// if there is an attribute, assume that it's the name
			if (nSize)
			{
				DOMAttr *pAttributeNode = (DOMAttr*) pAttributes->item(0);
				char *pName = XMLString::transcode(pAttributeNode->getName());

				Element newElement;

				char *data = XMLString::transcode(pAttributeNode->getValue());
				newElement.name = data;
				if (stricmp(pName, "PVS") == 0)
				{
					if (stricmp(data, "all") == 0)
					{
						m_bLightAllLayersByDefault = true;
					}
					else
					{
						newElement.bOn = true;
						m_LayersToLight.PushBack(newElement);
					}
				}
				else if (stricmp(pName, "DontPVS") == 0)
				{
					if (stricmp(data,"all") == 0)
					{
						m_bLightAllLayersByDefault = false;
					}
					else
					{
						newElement.bOn = false;
						m_LayersToLight.PushBack(newElement);
					}
				}
				else if (stricmp(pName, "Occlude") == 0)
				{
					if (stricmp(data,"all") == 0)
					{
						m_bCastShadowsOnAllLayersByDefault = true;
					}
					else
					{
						newElement.bOn = true;
						m_LayersThatCastShadows.PushBack(newElement);
					}
				}
				else if (stricmp(pName, "DontOcclude") == 0)
				{
					if (stricmp(data,"all") == 0)
					{
						m_bCastShadowsOnAllLayersByDefault = false;
					}
					else
					{
						newElement.bOn = false;
						m_LayersThatCastShadows.PushBack(newElement);
					}
				}
				XMLString::release(&pName);
			    XMLString::release(&data);
			}
		    XMLString::release(&pElementName);
		}
	}
    XMLString::release(&pElementName);
}

const char *StripReferenceNameFromLayerString(const char *pszLayer)
{
	const char *p = strrchr(pszLayer, ':');

	if (p)
		return p+1;
	else
		return pszLayer;
}

// returns true if a particular layer needs PVS'ing
bool LightingPass::DoesLayerPVS(const std::string &layerName)
{
	const char *p = StripReferenceNameFromLayerString(layerName.c_str());

	// check all explicitly specified layers
	for (int i=0; i<m_LayersToLight.Size(); i++)
	{
		if (stricmp(m_LayersToLight[i].name.c_str(), p) == 0)
			return m_LayersToLight[i].bOn;
	}

	// not found - use the default
    return m_bLightAllLayersByDefault;
}

// returns true if a particular layer needs to occlude
bool LightingPass::DoesLayerOcclude(const std::string &layerName)
{
	const char *p = StripReferenceNameFromLayerString(layerName.c_str());

	// check all explicitly specified layers
	for (int i=0; i<m_LayersThatCastShadows.Size(); i++)
	{
		if (stricmp(m_LayersThatCastShadows[i].name.c_str(), p) == 0)
			return m_LayersThatCastShadows[i].bOn;
	}

	// not found - use the default
    return m_bCastShadowsOnAllLayersByDefault;
}

