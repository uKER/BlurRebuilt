#include "stdafx.h"
#include <tools_common/GenericAttribute.h>
#include <ftm_xml_api/TexDatabase.h>
#ifdef YDL
#include <unistd.h>
#else
#include <io.h>
#endif

namespace FTM
{

U32 TexDatabase::DBRecord::NumAttributes() const
{
	return (U32)m_attributes.Size();
}

const std::string& TexDatabase::DBRecord::FileName() const
{
	return m_fileName;
}

std::string& TexDatabase::DBRecord::FileName()
{
	return m_fileName;
}

U32 TexDatabase::DBRecord::AddAttribute(const GenericAttribute& attr)
{
	m_attributes.PushBack(attr);
	return (U32)(m_attributes.Size()-1);
}

const GenericAttribute& TexDatabase::DBRecord::Attribute(U32 index) const
{
	return m_attributes[index];
}

GenericAttribute& TexDatabase::DBRecord::Attribute(U32 index)
{
	return m_attributes[index];
}

void TexDatabase::DBRecord::ReserveAttributeList(U32 size)
{
	m_attributes.Reserve(size);
}

TexDatabase::TexDatabase()
{
}

TexDatabase::~TexDatabase()
{
}

U32 TexDatabase::NumRecords() const
{
	return (U32)m_records.Size();
}

void TexDatabase::Clear()
{
	m_records.Clear();
}

Tu::Status TexDatabase::Read(const char* pFileName)
{
	using namespace XERCES_CPP_NAMESPACE;
	Tu::Status status;

#ifdef COMPILER_VC
	if (_access(pFileName, 4) == -1)
		return Tu::Status::FAILURE;
#else
	if (access(pFileName, 4) == -1)
		return Tu::Status::FAILURE;
#endif

	Clear();

	AbstractDOMParser::ValSchemes valScheme			= AbstractDOMParser::Val_Auto;
    bool doNamespaces								= false;
    bool doSchema									= false;
    bool schemaFullChecking							= false;

    // Initialize the XML4C system
	XMLPlatformUtils::Initialize();
	
	{
		// Instantiate the DOM parser.
		static const XMLCh gLS[] = { chLatin_L, chLatin_S, chNull };
		DOMImplementation *impl = DOMImplementationRegistry::getDOMImplementation(gLS);
		DOMBuilder *pParser = ((DOMImplementationLS*)impl)->createDOMBuilder(DOMImplementationLS::MODE_SYNCHRONOUS, 0);

		pParser->setFeature(XMLUni::fgDOMNamespaces, doNamespaces);
		pParser->setFeature(XMLUni::fgXercesSchema, doSchema);
		pParser->setFeature(XMLUni::fgXercesSchemaFullChecking, schemaFullChecking);
		pParser->setFeature(XMLUni::fgDOMValidateIfSchema, true);
	   
		// reset document pool
		pParser->resetDocumentPool();
		XERCES_CPP_NAMESPACE::DOMDocument *pDOMDocument = pParser->parseURI(pFileName);
	   
		if (!pDOMDocument)
		{
			pParser->release();
		    XMLPlatformUtils::Terminate();
			return Tu::Status::FAILURE;
		}

		//Parse.
		status = ParseXMLDOM((DOMNode*)pDOMDocument->getDocumentElement());
		if (!status)
		{
			pParser->release();
		    XMLPlatformUtils::Terminate();
			return status;
		}

		pParser->release();
	}

    // And call the termination method
    XMLPlatformUtils::Terminate();
	return Tu::Status::SUCCESS;
}

Tu::Status TexDatabase::ParseXMLDOM(XERCES_CPP_NAMESPACE::DOMNode* pRootNode)
{
	using namespace XERCES_CPP_NAMESPACE;
    DOMNode *child;

	if (pRootNode && pRootNode->getNodeType() == DOMNode::ELEMENT_NODE)
	{
		char *pElementName = XMLString::transcode(pRootNode->getNodeName());
		if (!stricmp(pElementName, "textures"))
		{
			return ParseTextures(pRootNode);
		}
	}

	for (child = pRootNode->getFirstChild(); child != 0; child = child->getNextSibling())
	{
        ParseXMLDOM(child);
	}

	return Tu::Status::SUCCESS;
}

Tu::Status TexDatabase::ParseTextures(XERCES_CPP_NAMESPACE::DOMNode* pTexturesNode)
{
	using namespace XERCES_CPP_NAMESPACE;
	Tu::Status status;
    DOMNode *child;

	U32 numTextures = 0;
	for (child = pTexturesNode->getFirstChild(); child != 0; child = child->getNextSibling())
	{
     	if (child && child->getNodeType() == DOMNode::ELEMENT_NODE)
		{
			char *pElementName = XMLString::transcode(child->getNodeName());
			
			if (child->hasAttributes())
			{
				if (!stricmp(pElementName, "texture"))
				{
					numTextures++;
				}
			}
		} 
	}
	m_records.Reserve(numTextures);

	for (child = pTexturesNode->getFirstChild(); child != 0; child = child->getNextSibling())
	{
		if (child && child->getNodeType() == DOMNode::ELEMENT_NODE)
		{
			char *pElementName = XMLString::transcode(child->getNodeName());
			
			if (child->hasAttributes())
			{
				if (!stricmp(pElementName, "texture"))
				{
					//Add a new texture record to our database.
					m_records.PushBack();
					DBRecord& record = m_records[m_records.Size()-1];

					DOMNamedNodeMap *pAttributes = child->getAttributes();
					
					int nSize = pAttributes->getLength();
					for (int i=0; i < nSize; ++i)
					{
						DOMAttr *pAttributeNode = (DOMAttr*) pAttributes->item(i);
						
						char *pName = XMLString::transcode(pAttributeNode->getName());
						char *pValue = XMLString::transcode(pAttributeNode->getValue());

						if (stricmp(pName, "name") != 0)
						{
							continue;
						}

						std::string& fileName = record.FileName();
						fileName = XMLString::transcode(pAttributeNode->getValue());

						DOMNode *grandChild;
						for (grandChild = child->getFirstChild(); grandChild != 0; grandChild = grandChild->getNextSibling())
						{
							if (grandChild && grandChild->getNodeType() == DOMNode::ELEMENT_NODE)
							{
								char *pElementName = XMLString::transcode(grandChild->getNodeName());
								if (!stricmp(pElementName, "actor"))
								{
									//Parse the parameters of this texture.
									status = ParseTextureParams(record, grandChild);
									if (!status) return status;
									break;
								}
							}
						}
					}
				}
			}
		}
	}

	return Tu::Status::SUCCESS;
}

Tu::Status TexDatabase::ParseTextureParams(DBRecord& record, XERCES_CPP_NAMESPACE::DOMNode* pTextureNode)
{
	using namespace XERCES_CPP_NAMESPACE;
    DOMNode *child;

	U32 numParams = 0;
	for (child = pTextureNode->getFirstChild(); child != 0; child = child->getNextSibling())
	{
		if (child && child->getNodeType() == DOMNode::ELEMENT_NODE)
		{
			char *pElementName = XMLString::transcode(child->getNodeName());
			if (stricmp(pElementName, "param") != 0)
				//Skip nodes not called 'param'.
				continue;

			numParams++;
		}
	}
	record.ReserveAttributeList(numParams);

	for (child = pTextureNode->getFirstChild(); child != 0; child = child->getNextSibling())
	{
		if (child && child->getNodeType() == DOMNode::ELEMENT_NODE)
		{
			char *pElementName = XMLString::transcode(child->getNodeName());
			if (stricmp(pElementName, "param") != 0)
				//Skip nodes not called 'param'.
				continue;
			
			if (!child->hasAttributes())
				//Skip nodes with no attributes.
				continue;

			DOMNamedNodeMap *pAttributes = child->getAttributes();
			int nSize = pAttributes->getLength();

			char* pParamName = 0;
			char* pParamType = 0;
			char* pParamValue = 0;
			for (int i=0; i < nSize; ++i)
			{
				DOMAttr *pAttributeNode = (DOMAttr*) pAttributes->item(i);
				char *pAttrName = XMLString::transcode(pAttributeNode->getName());
				if (stricmp(pAttrName, "name") == 0)
				{
					pParamName = XMLString::transcode(pAttributeNode->getValue());
				}
				else if (stricmp(pAttrName, "type") == 0)
				{
					pParamType = XMLString::transcode(pAttributeNode->getValue());
				}
				else if (stricmp(pAttrName, "value") == 0)
				{
					pParamValue = XMLString::transcode(pAttributeNode->getValue());
				}
			}

			U32 newAttrIndex = record.AddAttribute();
			GenericAttribute& genericAttr = record.Attribute(newAttrIndex);
			std::string& gAttrName = genericAttr.Name();
			gAttrName = pParamName;

			if (stricmp(pParamType, "int") == 0)
			{
				std::stringstream ss;
				ss << pParamValue;
				S32 integer;
				ss >> integer;
				genericAttr.SetValue(integer);
			}
			else if (stricmp(pParamType, "bool") == 0)
			{
				bool boolean = false;
				if (stricmp(pParamValue, "true") == 0)
					boolean = true;
				genericAttr.SetValue(boolean);
			}
			else if (stricmp(pParamType, "enum") == 0)
			{
				//Yep, we store enums as strings because we don't understand them.
				genericAttr.SetValue(pParamValue);
			}
			else if (stricmp(pParamType, "string") == 0)
			{
				genericAttr.SetValue(pParamValue);
			}
		}
	}
	return Tu::Status::SUCCESS;
}

S32 TexDatabase::RecordIndex(const char* pTextureName) const
{
	for (U32 i = 0; i < (U32)m_records.Size(); i++)
	{
		if (stricmp(m_records[i].FileName().c_str(), pTextureName) == 0)
		{
			return i;
		}
	}
	return -1;
}

TexDatabase::DBRecord& TexDatabase::Record(U32 index)
{
	return m_records[index];
}

const TexDatabase::DBRecord& TexDatabase::Record(U32 index) const
{
	return m_records[index];
}

}
