#include "AudioPch.h"
#include "xmlNode.h"


//------------------------------------------------------------------------------
//  SetData
//------------------------------------------------------------------------------
void xml::Node::SetData(const char* pchData)
{
	if (m_pchData)
		delete[] m_pchData;

	m_pchData = new char[strlen(pchData)+1];

	strcpy(m_pchData, pchData);
}

//------------------------------------------------------------------------------
//  Release
//------------------------------------------------------------------------------
void xml::Node::Release()
{
	if (m_pchData)
    {
		delete[] m_pchData;
        m_pchData = NULL;
    }
}

//------------------------------------------------------------------------------
//  SetType
//------------------------------------------------------------------------------
void xml::TagNode::SetType(const char* pchType)
{
	if (m_pchType)
		delete[] m_pchType;

	m_pchType = new char[strlen(pchType)+1];

	strcpy(m_pchType, pchType);
}

//------------------------------------------------------------------------------
//  SetAttributes
//------------------------------------------------------------------------------
void xml::TagNode::SetAttributes(const char* pchAttributes)
{
	if (m_pchAttributes)
		delete[] m_pchAttributes;

	m_pchAttributes = new char[strlen(pchAttributes)+1];

	strcpy(m_pchAttributes, pchAttributes);
}

//------------------------------------------------------------------------------
//  GetAttributeValue
//------------------------------------------------------------------------------
const char* xml::TagNode::GetAttributeValue(const char* pchKey, std::string& strValue)
{
	char* pchCurr = (char*)m_pchAttributes;
    strValue = "";
    char* pchCloseQuote;

	char* pchKeyStart = strstr(pchCurr, pchKey);

    //  if a match is found
    while (pchKeyStart)
    {
		//	make sure its the whole word
		char* pchKeyEnd = pchKeyStart + strlen(pchKey);

		//	skip whitespace
		while ((*pchKeyEnd == ' ') || (*pchKeyEnd == '\t') || (*pchKeyEnd == '\n') || (*pchKeyEnd == '\r'))
			pchKeyEnd++;

		//	next char must be '='
		char* pchEquals = pchKeyEnd;
		if (*pchEquals == '=')
        {
		    //	get value
		    char* pchValueStart = pchEquals + 1;;

		    //	skip whitespace
		    while ((*pchValueStart == ' ') || (*pchValueStart == '\t') || (*pchValueStart == '\n') || (*pchValueStart == '\r'))
			    pchValueStart++;

		    //	must be a quote (" or ')
		    char* pchOpenQuote = pchValueStart;
		    if ((*pchOpenQuote == '\'') || (*pchOpenQuote == '\"'))
            {
 		        //	get close quote
		        pchCloseQuote = strchr(pchOpenQuote + 1, *pchOpenQuote);

                if (pchCloseQuote)
                {
		            //	length of string to copy
		            int iStart = (int)(pchOpenQuote - m_pchAttributes) + 1;
		            int iEnd = (int)(pchCloseQuote - m_pchAttributes);

		            std::string strData(m_pchAttributes);

		            strValue = strData.substr(iStart, iEnd - iStart);

                    return pchCloseQuote+1;
                }
                else
                    pchCurr = pchOpenQuote;
            }
            else
                pchCurr = pchOpenQuote;
        }
        else
            pchCurr = pchEquals;

        pchKeyStart = strstr(pchCurr, pchKey);
    }

    return NULL;
}

//------------------------------------------------------------------------------
//  GetAttributeValueInt
//------------------------------------------------------------------------------
bool xml::TagNode::GetAttributeValueInt(const char* pchKey, int& iValue)
{
	int i = 0;
	std::string strValue;
	
	if (GetAttributeValue(pchKey, strValue)==0)
		return false;

	if (sscanf(strValue.data(), "%d", &i) == 0)
		return false;

	iValue =  i;

	return true;
}

//------------------------------------------------------------------------------
//  GetAttributeValueFloat
//------------------------------------------------------------------------------
bool xml::TagNode::GetAttributeValueFloat(const char* pchKey, float& fValue)
{
	float f = 0.f;
	std::string strValue;
	
	if (GetAttributeValue(pchKey, strValue)==0)
		return false;

	if (sscanf(strValue.data(), "%f", &f) == 0)
		return false;

	fValue =  f;

	return true;
}

//------------------------------------------------------------------------------
//  Make
//------------------------------------------------------------------------------
void xml::TagNode::AddChild(Node* pNode)
{
//    GetChildren().insert(pNode, GetChildren().zeroth());
    GetChildren().insert(pNode, GetChildren().last());
}

//------------------------------------------------------------------------------
//  Release
//------------------------------------------------------------------------------
void xml::TagNode::Release()
{
    Audio::Iterator<Node*> i = m_lChildren.first();
    while(!i.isPastEnd())
    {
        delete i.retrieve();
        i.advance();
    }
    m_lChildren.makeEmpty();

	if (m_pchType)
    {
		delete[] m_pchType;
        m_pchType = NULL;
    }

	if (m_pchAttributes)
    {
		delete[] m_pchAttributes;
        m_pchAttributes = NULL;
    }

    Node::Release();
}

//------------------------------------------------------------------------------
//  SetText
//------------------------------------------------------------------------------
void xml::TextNode::SetText(const char* pchText)
{
	if (m_pchText)
		delete[] m_pchText;

	m_pchText = new char[strlen(pchText)+1];

	strcpy(m_pchText, pchText);
}

//------------------------------------------------------------------------------
//  Release
//------------------------------------------------------------------------------
void xml::TextNode::Release()
{
	if (m_pchText)
    {
		delete[] m_pchText;
        m_pchText = NULL;
    }

    Node::Release();
}

//------------------------------------------------------------------------------
//  SetComment
//------------------------------------------------------------------------------
void xml::CommentNode::SetComment(const char* pchComment)
{
	if (m_pchComment)
		delete[] m_pchComment;

	m_pchComment = new char[strlen(pchComment)+1];

	strcpy(m_pchComment, pchComment);
}

//------------------------------------------------------------------------------
//  Release
//------------------------------------------------------------------------------
void xml::CommentNode::Release()
{
	if (m_pchComment)
    {
		delete[] m_pchComment;
        m_pchComment = NULL;
    }

    Node::Release();
}


