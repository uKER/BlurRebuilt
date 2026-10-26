#include "stdafx.h"

#include "Lux/Lights/Light.h"
#include "Lux/Lights/DirectionalLight.h"
#include "Lux/RenderChannel.h"
#include "Lux/LightingModelProperties.h"
#include "Lux/GlobalIllumination/GlobalIlluminationProperties.h"
#include "Lux/Lux.h"
#include "Lux/LightingModel.h"
#include "Lux/GlobalIllumination/GlobalIlluminationModel.h"
#include "Lux/SurfaceIterator.h"
#include "Lux/SamplePoints.h"
#include "lux_common/LightingPass.h"
#include "cc++/Thread.h"
#include "Lux/DomErrorHandler.h"

using namespace LUX;
using namespace XERCES_CPP_NAMESPACE;
using namespace XercesUtility;

Lux::Lux()
{
	Reset();
}

Lux::~Lux()
{
	Clear();
}

void Lux::Clear()
{
	delete m_pProperties;
	delete m_pLightingModel;
	if (m_pSamplePoints)
		delete m_pSamplePoints;
	for (int i=0; i<m_LightingPasses.Size(); i++)
		if (m_LightingPasses[i])
			delete m_LightingPasses[i];
	m_LightingPasses.Clear();

	Reset();
}

void Lux::Reset()
{
	Aftk::C_Scene* m_pAftkScene = 0;
	m_pHierarchy = 0;
	m_pProperties = 0;
	m_pLightingModel = 0;
	m_pLogCallbackFunc = 0;
	m_ProgressCallback = 0;
	m_pSamplePoints = NULL;
	m_iAtlasOffset = 0;
}

void Lux::SetAftkScene(Aftk::C_Scene *pAftkScene)
{
	m_pAftkScene = pAftkScene;
}

void Lux::SetSpatialHierarchy(LUX::SpatialHierarchy* pHierachy)
{
	m_pHierarchy = pHierachy;
}

void Lux::SetLightingModelPropertiesPtr(LightingModelProperties* pProperties)
{
	if (m_pProperties != 0)
	{
		delete m_pProperties;
	}
	m_pProperties = pProperties;
}

Aftk::C_Scene* Lux::GetAftkScene()
{
	return m_pAftkScene;
}

const LUX::SpatialHierarchy* Lux::GetSpatialHierarchyPtr() const
{
	return m_pHierarchy;
}

const LightingModelProperties* Lux::GetLightingModelPropertiesPtr() const
{
	return m_pProperties;
}

LightingModelProperties* Lux::GetLightingModelPropertiesPtr()
{
	return m_pProperties;
}

void Lux::SetInputSceneFileName(const std::string& fileName)
{
	m_inputSceneFileName = fileName;
}

void Lux::SetOutputSceneFileName(const std::string& fileName)
{
	m_outputSceneFileName = fileName;
}

void Lux::SetLightingModelFileName(const std::string& fileName)
{
	m_lightingModelFileName = fileName;
}

const std::string& Lux::GetInputSceneFileName() const
{
	return m_inputSceneFileName;
}

const std::string& Lux::GetOutputSceneFileName() const
{
	return m_outputSceneFileName;
}

const std::string& Lux::GetLightingModelFileName() const
{
	return m_lightingModelFileName;
}

LuxLightMapFileTypeEnum Lux::GetLightMapFileType() const
{
	return m_lightMapFileType;
}

SamplePoints *Lux::GetSamplePointsPtr() const
{
	return m_pSamplePoints;
}

const std::string& Lux::GetLightMapFolder() const
{
	return m_lightMapFolder;
}

const std::string& Lux::GetLightMapFilePrefix() const
{
	return m_lightMapFilePrefix;
}

void Lux::SetLightMapFolder(const std::string& folder)
{
	m_lightMapFolder = folder;
}

void Lux::SetLightMapFilePrefix(const std::string& prefix)
{
	m_lightMapFilePrefix = prefix;
}

void Lux::SetLightMapFileType(LUX::LuxLightMapFileTypeEnum type)
{
	m_lightMapFileType = type;
}

bool Lux::Light(Progress::I_ProgressBar *pProgress)
{
	if (m_pProperties == 0)
	{
		return false;
	}
	bool status;
	LuxModelTypeEnum type = m_pProperties->GetType();

	if (m_pLightingModel != 0)
	{
		delete m_pLightingModel;
		m_pLightingModel = NULL;
	}
	switch (type)
	{
		case LUX_MODEL_TYPE_GLOBALILLUMINATION:	// monte-carlo integrator + instant radiosity
			m_pLightingModel = new GlobalIlluminationModel;
			break;
		default:								// unknown lighting model - abort!
			return false;
	}

	status = m_pLightingModel->Light(
		m_pHierarchy, 
		m_pProperties,
		this,
		m_pAftkScene,
		pProgress);

	return status;
}

const LightingModel* Lux::GetLightingModelPtr() const
{
	return m_pLightingModel;
}


void Lux::ReadXMLNode(DOMNode *pNode)
{
    DOMNode *child;

    if (pNode) 
	{
		char *pElementName = XMLString::transcode(pNode->getNodeName());
		if (stricmp(pElementName, "LpvsProject") == 0)
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

					if (stricmp(pName, "InputScene") == 0)
					{
						char* pValue = XMLString::transcode(pAttributeNode->getValue());
						m_inputSceneFileName = pValue;
						XMLString::release(&pValue);
					}
					else if (stricmp(pName, "OutputScene") == 0)
					{
						char* pValue = XMLString::transcode(pAttributeNode->getValue());
						m_outputSceneFileName = pValue;
						XMLString::release(&pValue);
					}
					else if (stricmp(pName, "LightMapOutputFolder") == 0)
					{
						char* pValue = XMLString::transcode(pAttributeNode->getValue());
						m_lightMapFolder = pValue;
						XMLString::release(&pValue);
					}
					else if (stricmp(pName, "LightMapFilePrefix") == 0)
					{
						char* pValue = XMLString::transcode(pAttributeNode->getValue());
						m_lightMapFilePrefix = pValue;
						XMLString::release(&pValue);
					}
					else if (stricmp(pName, "LightMapFileType") == 0)
					{
						char* pValue = XMLString::transcode(pAttributeNode->getValue());
						if (stricmp(pValue, "bmp") == 0)
						{
							m_lightMapFileType = LUX_LIGHT_MAP_FILE_TYPE_BMP;
						}
						else if (stricmp(pValue, "tga") == 0)
						{
							m_lightMapFileType = LUX_LIGHT_MAP_FILE_TYPE_TGA;
						}
						else if (stricmp(pValue, "dds") == 0)
						{
							m_lightMapFileType = LUX_LIGHT_MAP_FILE_TYPE_DDS;
						}
						XMLString::release(&pValue);
					}
					else if (stricmp(pName, "LightingModel") == 0)
					{
						char* pValue = XMLString::transcode(pAttributeNode->getValue());
						m_lightingModelFileName = pValue;
						XMLString::release(&pValue);
					}
					XMLString::release(&pName);
				}
			}
		}
        for (child = pNode->getFirstChild(); child != 0; child = child->getNextSibling())
		{
           ReadXMLNode(child);
		}
    }
}

bool Lux::ReadProjectProperties(const std::string& fileName)
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
         S_ERROR( g_Log, "Error during initialization! :\n"
						<< toCatch.getMessage() << std::endl);
         return 1;
    }

    // Instantiate the DOM parser.
    static const XMLCh gLS[] = { chLatin_L, chLatin_S, chNull };
    DOMImplementation *impl = 
		DOMImplementationRegistry::getDOMImplementation(gLS);
    DOMBuilder        *pParser = 
		((DOMImplementationLS*)impl)->createDOMBuilder(DOMImplementationLS::MODE_SYNCHRONOUS, 0);

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

	LuxDOMErrorHandler myErrorHandler(fileName);
	pParser->setErrorHandler(&myErrorHandler);

	XERCES_CPP_NAMESPACE::DOMDocument *pDOMDocument = 0;

    try
    {
        // reset document pool
        pParser->resetDocumentPool();
		pDOMDocument = pParser->parseURI(fileName.c_str());
    }
    catch (const XMLException& toCatch)
    {
		S_ERROR( g_Log, "\nError during parsing: '" << fileName << "'\n" << "Exception message is:  \n"
               << toCatch.getMessage() << "\n");
    }
    catch (const DOMException& toCatch)
    {
        const unsigned int maxChars = 2047;
        XMLCh errText[maxChars + 1];

        S_ERROR( g_Log, "\nDOM Error during parsing: '" << fileName << "'\n"
                << "DOMException code is:  " << toCatch.code);

        if (DOMImplementation::loadDOMExceptionMsg(toCatch.code, errText, maxChars))
		{
			S_ERROR( g_Log, "Message is: " << errText << "\n");
		}
    }
    catch (...)
    {
		S_ERROR( g_Log, "\nUnexpected exception during parsing: '" << fileName.c_str() << "'\n");
    }

	{
		//Okay, we've setup the XML parser, now let's read in the data.
		if (pDOMDocument != 0) 
		{
			ReadXMLNode( (DOMNode*)pDOMDocument->getDocumentElement() );
		}
	}

    pParser->release();

    // And call the termination method
    XMLPlatformUtils::Terminate();

	return true;
}

bool Lux::WriteProjectProperties(const std::string& fileName) const
{
	// TODO: someone needs to write this!

	return false;
}

void Lux::CreateLightingModelFromXML(DOMNode *pNode)
{
    DOMNode *child;

    if (pNode) 
	{
        if (pNode->getNodeType() == DOMNode::ELEMENT_NODE)
		{
            char *pElementName = XMLString::transcode(pNode->getNodeName());
			if (stricmp(pElementName, "lightingmodel") == 0)
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
						if (stricmp(pName, "type") == 0)
						{
							char* pValue;

							// get attribute type
							pValue = XMLString::transcode(pAttributeNode->getValue());
							if (stricmp(pValue, "globalillumination") == 0)
							{
								m_pProperties = new GlobalIlluminationProperties;
							}
							XMLString::release(&pValue);
							if (m_pProperties != 0)
							{
								XMLString::release(&pElementName);
								XMLString::release(&pName);
								return;
							}
						}
	                    
						XMLString::release(&pName);
					}
				}
			}

            XMLString::release(&pElementName);
		}
        for (child = pNode->getFirstChild(); child != 0; child = child->getNextSibling())
		{
			CreateLightingModelFromXML(child);
		}
    }
}

// find sample point xml node, also read alpha texture filenames and pass info
void Lux::ParseAdditionalXMLData(DOMNode *pNode)
{
    DOMNode *child;

	if (pNode)
	{
        if (pNode->getNodeType() == DOMNode::ELEMENT_NODE)
		{
            char *pElementName = XMLString::transcode(pNode->getNodeName());
			if (stricmp(pElementName, "samplepoints") == 0)
			{
				// read the sample point list info block
				if(pNode->hasAttributes())
				{
					m_pSamplePoints = new SamplePoints;
					m_pSamplePoints->ReadXMLNode(pNode);
				}
			}
			else if (stricmp(pElementName, "pass") == 0)
			{
				// read a lighting pass info block
				LightingPass *p = new LightingPass;
				p->m_name.Printf("%d", m_LightingPasses.Size());
				p->ReadXMLNode(pNode);
				m_LightingPasses.PushBack(p);
			}

            XMLString::release(&pElementName);
		}
		
        for (child = pNode->getFirstChild(); child != 0; child = child->getNextSibling())
		{
			ParseAdditionalXMLData(child);
		}
    }
}


bool Lux::ReadLightingModelProperties()
{
	std::string fileName = m_lightingModelFileName;

	if (m_pProperties != 0)
	{
		delete m_pProperties;
		m_pProperties = 0;
	}
	if (m_pLightingModel != 0)
	{
		delete m_pLightingModel;
		m_pLightingModel = 0;
	}

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
         S_ERROR( g_Log, "Error during initialization! :\n"
              << toCatch.getMessage() << "\n");
         return 1;
    }

    // Instantiate the DOM parser.
    static const XMLCh gLS[] = { chLatin_L, chLatin_S, chNull };
    DOMImplementation *impl = 
		DOMImplementationRegistry::getDOMImplementation(gLS);
    DOMBuilder        *pParser = 
		((DOMImplementationLS*)impl)->createDOMBuilder(DOMImplementationLS::MODE_SYNCHRONOUS, 0);

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

	LuxDOMErrorHandler myErrorHandler(fileName);
	pParser->setErrorHandler(&myErrorHandler);

    try
    {
		// reset document pool
		pParser->resetDocumentPool();

		pDOMDocument = pParser->parseURI(fileName.c_str());
    }
    catch (const XMLException& toCatch)
    {
		S_ERROR( g_Log, "\nError during parsing: '" << fileName << "'\n" << "Exception message is:  \n"
               << toCatch.getMessage() << "\n");
    }
    catch (const DOMException& toCatch)
    {
        const unsigned int maxChars = 2047;
        XMLCh errText[maxChars + 1];

        S_ERROR( g_Log, "\nDOM Error during parsing: '" << fileName << "'\n"
                << "DOMException code is:  " << toCatch.code << "\n");

        if (DOMImplementation::loadDOMExceptionMsg(toCatch.code, errText, maxChars))
		{
			S_ERROR( g_Log, "Message is: " << errText << "\n");
		}

    }
    catch (...)
    {
		S_ERROR( g_Log, "\nUnexpected exception during parsing: '" << fileName.c_str() << "'\n");
    }

	if (!pDOMDocument)
	{
		S_FATAL( g_Log, "\nCould not parse .xml file '" << fileName.c_str() << "'\n");
		return false;
	}

	//Parse the XML file
	//Perform an intial pass to determine what lighting model we are supposed to be reading
	//in.
	CreateLightingModelFromXML( (DOMNode*)pDOMDocument->getDocumentElement() );

	// read sample points
	ParseAdditionalXMLData( (DOMNode*)pDOMDocument->getDocumentElement() );

    pParser->release();

    // And call the termination method
    XMLPlatformUtils::Terminate();

	if (m_pProperties != 0)
	{
		//We succesfully created the correct properties container for our new lighting model.
		//So let's now read it in
		return m_pProperties->Read(fileName);
	}
	else
	{
		return false;
	}
}

bool Lux::WriteLightingModelProperties(const std::string& fileName) const
{
	return true;
}

void Lux::SetProgressCallback(void *pThis, ProgressCallbackProc *p)
{
	m_ProgressContext = pThis;
	m_ProgressCallback = p;
}

bool Lux::DoProgressCallback(float f, char *psz) const
{
	if (m_ProgressCallback)
	{
		return m_ProgressCallback(m_ProgressContext, f, psz);
	}
	return false;
}

const Ut::Vector<LightingPass*> *Lux::GetLightingPassVector() const
{
	return &m_LightingPasses;
}

// collect lighting pass layer settings and convert to bits
Ut::BitArray128	Lux::GetLightingPassBits(int iPass) const
{
	Ut::BitArray128 bits;

	if (m_pAftkScene->GetNumLayers()>=128)
	{
		LOGERROR( LUX::g_Log, "ERROR: Number of layers in the scene is too high. (the maximum allowed is 127).\n"
						 	  "*** THE SCENE WILL NOT BE LIT CORRECTLY ***\n" );
	}

	for (int i=0; i<m_pAftkScene->GetNumLayers(); i++)
	{
		if (m_LightingPasses[iPass]->DoesLayerCastShadows( m_pAftkScene->GetLayerPtr(i)->GetName() ))
		{
			bits[i] = 1;
		}
		else
		{
			bits[i] = 0;
		}
	}

	return bits;
}


// calculate usage of each layer, to pass into the spatial hierarchy builder
void Lux::GetLayerUsage(LUX::SpatialHierarchy::LayerMode *piLayerUsage)
{
	for (int iLayer=0; iLayer<m_pAftkScene->GetNumLayers(); iLayer++)
	{
		const std::string *pLayerName = &m_pAftkScene->GetLayerPtr(iLayer)->GetName();

		// for each layer, check each lighting pass
		bool bAlwaysOn = true;
		bool bAlwaysOff = true;

		for (int iPass=0; iPass<m_LightingPasses.Size(); iPass++)
		{
			if (m_LightingPasses[iPass]->DoesLayerCastShadows(pLayerName->c_str()))
				bAlwaysOff = false;
			else
				bAlwaysOn = false;
		}

		if (bAlwaysOn)
			piLayerUsage[iLayer] = SD::SpatialHierarchy::AlwaysOn;
		else if (bAlwaysOff)
			piLayerUsage[iLayer] = SD::SpatialHierarchy::AlwaysOff;
		else
			piLayerUsage[iLayer] = SD::SpatialHierarchy::Variable;
	}    
}

void Lux::SetCurrentPass(int i)
{
	m_iCurrentPass = i;

	// turn layers on/off in spatial hierarchy
	for (int iLayer=0; iLayer<m_pAftkScene->GetNumLayers(); iLayer++)
	{
		const std::string *pLayerName = &m_pAftkScene->GetLayerPtr(iLayer)->GetName();

		m_pHierarchy->SetLayer(*pLayerName, 		
			m_LightingPasses[m_iCurrentPass]->DoesLayerCastShadows(pLayerName->c_str()) );
	}

}

int Lux::GetNumPasses()
{
	// if we have no passes at all, create a default one
	if ( m_LightingPasses.Size() == 0)
	{
		LightingPass *p = new LightingPass;
		p->m_name = "1";
		m_LightingPasses.PushBack(p);
	}

	return m_LightingPasses.Size();
}

