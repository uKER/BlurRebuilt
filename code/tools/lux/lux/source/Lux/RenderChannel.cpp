#include "stdafx.h"

#include "Lux/RenderChannel.h"

using namespace LUX;
using namespace XERCES_CPP_NAMESPACE;

IMPLEMENT_PERSISTENCE(RenderChannel, "RenderCh");

// create default renderchannel
RenderChannel::RenderChannel(bool bDefault)
{
	assert(bDefault);

	m_target = RGB;
	m_name   = "Default";
}

// copy constructor
RenderChannel::RenderChannel(RenderChannel &ref)
{
	m_target	= ref.m_target;
	m_name		= m_name;
}

// read channel data from XML file
void RenderChannel::ReadXMLNode(DOMNode *pNode)
{
    if (!pNode) return;
    if (pNode->getNodeType() != DOMNode::ELEMENT_NODE) return;

	char *pElementName = XMLString::transcode(pNode->getNodeName());
	if (stricmp(pElementName, "channel") == 0 && pNode->hasAttributes())
	{
		// get all the attributes of the node
		DOMNamedNodeMap *pAttributes = pNode->getAttributes();
		int nSize = pAttributes->getLength();

		for(int i = 0; i < nSize; ++i) 
		{
			DOMAttr *pAttributeNode = (DOMAttr*) pAttributes->item(i);
			char *pName = XMLString::transcode(pAttributeNode->getName());

			// get channel name
			if (stricmp(pName, "Name") == 0)
			{
				// get channel name
				char* pValue = XMLString::transcode(pAttributeNode->getValue());
				m_name = pValue;
				XMLString::release(&pValue);
			}

			// get target type
			if (stricmp(pName, "Target") == 0)
			{
				char *pValue = XMLString::transcode(pAttributeNode->getValue());
				if (stricmp(pValue, "Red") == 0 || stricmp(pValue, "1") == 0)
					m_target = Red;
				else if (stricmp(pValue, "Green") == 0 || stricmp(pValue, "2") == 0)
					m_target = Green;
				else if (stricmp(pValue, "Blue") == 0 || stricmp(pValue, "3") == 0)
					m_target = Blue;
				else if (stricmp(pValue, "RGB") == 0 || stricmp(pValue, "all") == 0)
					m_target = RGB;
				else if (stricmp(pValue, "Alpha") == 0 || stricmp(pValue, "4") == 0)
					m_target = Alpha;
				else
					m_target = None;

				XMLString::release(&pValue);
			}
		}
	}
    XMLString::release(&pElementName);
}

