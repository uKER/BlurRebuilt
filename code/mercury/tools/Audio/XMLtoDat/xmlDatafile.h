#ifndef _XMLDATAFILE_H_
#define _XMLDATAFILE_H_

#include "stringdatafile.h"
#include <string>

namespace xml
{
    class Parser;
    class Node;
    class TagNode;
}

//------------------------------------------------------------------------------
//  XmlDataFile
//------------------------------------------------------------------------------
class XmlDataFile : public StringDataFile
{
public:
    XmlDataFile() : StringDataFile() {}

    //  create blocks from xml tags
	virtual void Make(xml::Node* pNode, Block** ppBlock, Block* pParentBlock = NULL);

    //  support functions
    char* GetValueFromKey(const char* data, const char* pchKey, std::string& strValue);
    F32 GetFloatValue(const char* pchValue);
    S32 GetIntValue(const char* pchValue);

    const char* GetFirstNonWhitespace(const char* pchString);

protected:

    //  overridable to allow data to be packed
	virtual Block* FoundTag(xml::TagNode* pTag, Block* pParentBlock);
	virtual Block* FoundTagAttributes(xml::TagNode* pParentNode, const char* pchType, const char* pchAttributes, Block* pParentBlock);
	virtual Block* FoundText(xml::TagNode* pParentNode, const char* pchText, Block* pParentBlock);
	virtual Block* FoundComment(xml::TagNode* pParentNode, const char* pchComment, Block* pParentBlock);
    virtual void EndNode(xml::TagNode* pTag) { return; }

    //  'Attributes', 'Text', 'Comment'
	virtual bool IsData(DWORD dwBlockID);
};


#endif // _DATAFILE_H_

