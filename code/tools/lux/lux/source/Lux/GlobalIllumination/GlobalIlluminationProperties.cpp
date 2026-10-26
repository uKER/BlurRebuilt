#include "stdafx.h"

#include "Lux/Lights/Light.h"
#include "Lux/Lights/DirectionalLight.h"
#include "Lux/RenderChannel.h"
#include "Lux/LightingModelProperties.h"
#include "Lux/GlobalIllumination/GlobalIlluminationProperties.h"
#include "Lux/DomErrorHandler.h"

using namespace LUX;
using namespace XERCES_CPP_NAMESPACE;

extern bool gbCarMode;
bool gbDebugMode;

GlobalIlluminationProperties::GlobalIlluminationProperties() : LightingModelProperties()
{
	Reset();
}

GlobalIlluminationProperties::GlobalIlluminationProperties(const GlobalIlluminationProperties& rhs)
{
	*this = rhs;
}

GlobalIlluminationProperties::~GlobalIlluminationProperties()
{
	Clear();
}

void GlobalIlluminationProperties::ReadXMLNode(DOMNode *pNode)
{
	DOMNode *child;

    if (pNode) 
	{
        if (pNode->getNodeType() == DOMNode::ELEMENT_NODE)
		{
            char *pElementName = XMLString::transcode(pNode->getNodeName());
			if (stricmp(pElementName, "ModelProperties") == 0)
			{
				if(pNode->hasAttributes())
				{
					// get all the attributes of the node
					DOMNamedNodeMap *pAttributes = pNode->getAttributes();
					int nSize = pAttributes->getLength();

					for(int i = 0; i < nSize; ++i) 
					{
						DOMAttr *pAttributeNode = (DOMAttr*) pAttributes->item(i);
						// get attribute name
						char *pName = XMLString::transcode(pAttributeNode->getName());

						// get car mode on/off
						if (stricmp(pName, "car") == 0)
						{
							char* pValue = XMLString::transcode(pAttributeNode->getValue());
							if (stricmp(pValue, "on") == 0)
							{
								gbCarMode = true;
							}
							XMLString::release(&pValue);
						}
						// get debug mode on/off
						else if (stricmp(pName, "debug") == 0)
						{
							char* pValue = XMLString::transcode(pAttributeNode->getValue());
							if (stricmp(pValue, "on") == 0)
							{
								gbDebugMode = true;
							}
							XMLString::release(&pValue);
						}
						// get gotham channels on/off
						else if (stricmp(pName, "forcegothamchannels") == 0)
						{
							char* pValue = XMLString::transcode(pAttributeNode->getValue());
							if (stricmp(pValue, "on") == 0)
							{
								m_iForceGothamChannels = 1;
							}
							else if (stricmp(pValue, "version_two") == 0)
							{
								m_iForceGothamChannels = 2;
							}
							else if (stricmp(pValue, "off") == 0)
							{
								m_iForceGothamChannels = 0;
							}
							XMLString::release(&pValue);
						}
						// allow vertex colour optimization
						else if (stricmp(pName, "optimize") == 0)
						{
							char* pValue = XMLString::transcode(pAttributeNode->getValue());
							if (stricmp(pValue, "on") == 0)
							{
								m_bOptimize = true;
							}
							else if (stricmp(pValue, "off") == 0)
							{
								m_bOptimize = false;
							}
							XMLString::release(&pValue);
						}
						// get shooter channels on/off
						else if (stricmp(pName, "forceshooterchannels") == 0)
						{
							char* pValue = XMLString::transcode(pAttributeNode->getValue());
							if (stricmp(pValue, "on") == 0)
							{
								m_bForceShooterChannels = true;
							}
							else if (stricmp(pValue, "off") == 0)
							{
								m_bForceShooterChannels = false;
							}
							XMLString::release(&pValue);
						}
						// get bounce lighting on/off
						else if (stricmp(pName, "bouncelighting") == 0)
						{
							char* pValue = XMLString::transcode(pAttributeNode->getValue());
							if (stricmp(pValue, "on") == 0)
							{
								m_bBounceLighting = true;
							}
							else if (stricmp(pValue, "off") == 0)
							{
								m_bBounceLighting = false;
							}
							XMLString::release(&pValue);
						}
						// get bounce paths multiplier
						else if (stricmp(pName, "bouncepaths") == 0)
						{
							char* pValue = XMLString::transcode(pAttributeNode->getValue());
							std::stringstream s;
							float fl = 0;
							s << pValue;
							s >> fl;
							m_fBouncePathsPerLight = fl;
							XMLString::release(&pValue);
						}
						// get instancing mode
						else if (stricmp(pName, "instancemode") == 0)
						{
							char* pValue = XMLString::transcode(pAttributeNode->getValue());
							if (stricmp(pValue, "delete") == 0)
							{
								m_iInstanceMode = INSTANCE_MODE_DELETE;
							}
							else if (stricmp(pValue, "instance") == 0)
							{
								m_iInstanceMode = INSTANCE_MODE_UV_OFFSET;
							}
							else if (stricmp(pValue, "instantiate") == 0)
							{
								m_iInstanceMode = INSTANCE_MODE_INSTANTIATE;
							}
							XMLString::release(&pValue);
						}
						// make instances default to use SH lighting?
						else if (stricmp(pName, "instancesdefaulttoshlighting") == 0)
						{
							char* pValue = XMLString::transcode(pAttributeNode->getValue());
							if (stricmp(pValue, "true") == 0)
							{
								m_bInstancesDefaultToSHLighting = true;
							}
							else
							{
								m_bInstancesDefaultToSHLighting = false;
							}
						}
						// get bounce lighting channel
						else if (stricmp(pName, "bouncechannel") == 0)
						{
							char* pValue = XMLString::transcode(pAttributeNode->getValue());
							std::stringstream s;
							std::string	bounceChannel;
							s << pValue;
							s >> bounceChannel;
							XMLString::release(&pValue);
							// 
							m_iBounceChannel = GetRenderChannel(bounceChannel.c_str());
						}							
						// get anti chart seams
						else if (stricmp(pName, "fixseams") == 0)
						{
							char* pValue = XMLString::transcode(pAttributeNode->getValue());
							m_bFixAllSeams = false;
							m_bFixSeams = false;
							if (stricmp(pValue, "on") == 0)
							{
								m_bFixSeams = true;
							}
							else if (stricmp(pValue, "yes") == 0)
							{
								m_bFixSeams = true;
							}
							else if (stricmp(pValue, "all") == 0)
							{
								m_bFixSeams = true;
								m_bFixAllSeams = true;
							}
							XMLString::release(&pValue);
						}

						XMLString::release(&pName);
					}
				}
			}
            XMLString::release(&pElementName);
		}
        for (child = pNode->getFirstChild(); child != 0; child = child->getNextSibling())
		{
			ReadXMLNode(child);
		}
    }
}

bool GlobalIlluminationProperties::Write(const std::string& fileName) const
{
	return true;
}

bool GlobalIlluminationProperties::Read(const std::string& fileName)
{
	AbstractDOMParser::ValSchemes valScheme			= AbstractDOMParser::Val_Auto;
    bool doNamespaces								= false;
    bool doSchema									= false;
    bool schemaFullChecking							= false;

    // Initialize the XML4C system
	try
	{
		XMLPlatformUtils::Initialize();
	}
	catch (const XMLException& toCatch)
    {
         std::cerr << "Error during initialization! :\n"
              << toCatch.getMessage() << std::endl;
         return 1;
    }

    // Instantiate the DOM parser.
    static const XMLCh gLS[] = { chLatin_L, chLatin_S, chNull };
    DOMImplementation *impl = 
		DOMImplementationRegistry::getDOMImplementation(gLS);
    DOMBuilder        *pParser = 
		((DOMImplementationLS*)impl)->createDOMBuilder(DOMImplementationLS::MODE_SYNCHRONOUS, 0);

//	LuxDOMErrorHandler myErrorHandler(fileName);
//	pParser->setErrorHandler(&myErrorHandler);
    pParser->setFeature(XMLUni::fgDOMNamespaces, doNamespaces);
    pParser->setFeature(XMLUni::fgXercesSchema, doSchema);
    pParser->setFeature(XMLUni::fgXercesSchemaFullChecking, schemaFullChecking);

    if (valScheme == AbstractDOMParser::Val_Auto)
    {
        pParser->setFeature(XMLUni::fgDOMValidateIfSchema, true);
    }
    else if (valScheme == AbstractDOMParser::Val_Never)
    {
        pParser->setFeature(XMLUni::fgDOMValidation, false);
    }
    else if (valScheme == AbstractDOMParser::Val_Always)
    {
        pParser->setFeature(XMLUni::fgDOMValidation, true);
    }

	XERCES_CPP_NAMESPACE::DOMDocument *pDOMDocument = 0;

    // reset document pool
    pParser->resetDocumentPool();
	pDOMDocument = pParser->parseURI(fileName.c_str());

	//Okay, we've setup the XML parser, now let's read in the data.
	if (pDOMDocument != 0) 
	{
		this->LightingModelProperties::ReadXMLNode( (DOMNode*)pDOMDocument->getDocumentElement() );
		ReadXMLNode( (DOMNode*)pDOMDocument->getDocumentElement() );
	}

    pParser->release();
//	if (myErrorHandler.bError)
//		return false;

    // And call the termination method
    XMLPlatformUtils::Terminate();

	return true;
}

LuxModelTypeEnum GlobalIlluminationProperties::GetType() const
{
	return LUX_MODEL_TYPE_GLOBALILLUMINATION;
}

GlobalIlluminationProperties* GlobalIlluminationProperties::GetClonePtr() const
{
	return new GlobalIlluminationProperties(*this);
}

LuxModelSimpleMethodEnum GlobalIlluminationProperties::GetMethod() const
{
	return m_method;
}

void GlobalIlluminationProperties::SetMethod(LuxModelSimpleMethodEnum method)
{
	m_method = method;
}

void GlobalIlluminationProperties::Clear()
{
	this->LightingModelProperties::Clear();
	Reset();
}

void GlobalIlluminationProperties::Reset()
{
	this->LightingModelProperties::Reset();

	m_method = LUX_MODEL_SIMPLE_METHOD_HEMISPHERE;
	m_iForceGothamChannels				= 0;
	m_bForceShooterChannels				= false;
	m_bBounceLighting					= false;
	m_fInputGamma						= 1.0f;
	m_fOutputGamma						= 1.0f;
	m_fBouncePathsPerLight				= 32;
	m_fBrightness						= 1.0f;
	m_iInstanceMode						= INSTANCE_MODE_UV_OFFSET;
	m_bFixSeams							= false;
	m_iBounceChannel					= 0;
	m_bWhiteAlpha						= true;
	m_bOptimize							= false;
	m_bPrescaleSH						= false;
	m_bInstancesDefaultToSHLighting		= true;
}

