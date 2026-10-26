#include "stdafx.h"

using namespace XERCES_CPP_NAMESPACE;

#include "vis_core/DomErrorHandler.h"
#include "vis_core/ReadXmlConfig.h"

VisConfig::~VisConfig()
{
	for (int i=0; i<m_Passes.Size(); i++)
		delete m_Passes[i];
}


bool VisConfig::Read(const std::string& fileName)
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

	LuxDOMErrorHandler myErrorHandler(fileName);
	pParser->setErrorHandler(&myErrorHandler);
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
		ReadXMLNode( (DOMNode*)pDOMDocument->getDocumentElement() );
	}

    pParser->release();

    // And call the termination method
    XMLPlatformUtils::Terminate();

	return !myErrorHandler.bError;
}

// read xml data
void VisConfig::ReadXMLNode(DOMNode *pNode)
{
    DOMNode *child;

	if (pNode)
	{
        if (pNode->getNodeType() == DOMNode::ELEMENT_NODE)
		{
            char *pElementName = XMLString::transcode(pNode->getNodeName());
			if (stricmp(pElementName, "pass") == 0)
			{
				// read a lighting pass info block
				LightingPass *p = new LightingPass;
				p->m_name.Printf("%d", m_Passes.Size());
				p->ReadXMLNode(pNode);
				m_Passes.PushBack(p);
			}

            XMLString::release(&pElementName);
		}
		
        for (child = pNode->getFirstChild(); child != 0; child = child->getNextSibling())
		{
			ReadXMLNode(child);
		}
    }
}
