#include "stdafx.h"
#include "block.h"
#include "xmldatafile.h"
#include "xmlparser.h"
#include "xmlnode.h"

//------------------------------------------------------------------------------
//  Make
//------------------------------------------------------------------------------
void XmlDataFile::Make(xml::Node* pNode, Block** ppBlock, Block* pParentBlock)
{
    xml::TagNode* pTag = dynamic_cast<xml::TagNode*>(pNode);
    xml::TextNode* pText = dynamic_cast<xml::TextNode*>(pNode);
    xml::CommentNode* pComment = dynamic_cast<xml::CommentNode*>(pNode);

	*ppBlock = NULL;

	if (pTag)
	{
		const char* pchName = pTag->GetType();
		const char* pchAttributes = pTag->GetAttributes();

		//	process tag
		*ppBlock = FoundTag(pTag, pParentBlock);

		//	if including this tag
		if (*ppBlock)
		{
			//	process attributes
			if (pchAttributes)
			{
				Block* pAttributesBlock = FoundTagAttributes(pTag, pchName, pchAttributes, pParentBlock);

				if (pAttributesBlock)
					(*ppBlock)->AddChild(pAttributesBlock);
			}

            for (Audio::Iterator<xml::Node*> i = pTag->GetChildren().first(); !i.isPastEnd(); i.advance())
			{
                Block* pChildBlock = NULL;

				//	recurse
				Make(i.retrieve(), &pChildBlock, *ppBlock);

				if (pChildBlock)
					(*ppBlock)->AddChild(pChildBlock);
			}
		}

        EndNode(pTag);
	}

	else if (pText)
		*ppBlock = FoundText(pText->GetParent(), pText->GetText(), pParentBlock);

	else if (pComment)
	{
		if (pComment->GetComment())
			*ppBlock = FoundComment(pComment->GetParent(), pComment->GetComment(), pParentBlock);
	}
}

//------------------------------------------------------------------------------
//  FoundTag
//------------------------------------------------------------------------------
Block* XmlDataFile::FoundTag(xml::TagNode* pTag, Block* pParentBlock)
{
	Block* pTagBlock = new Block(pParentBlock);
	pTagBlock->SetID(DoEndian(AddString(pTag->GetType())));

//    printf("Using default FoundTag\n");

    return pTagBlock;
}

//------------------------------------------------------------------------------
//  FoundAttributes
//------------------------------------------------------------------------------
Block* XmlDataFile::FoundTagAttributes(xml::TagNode* pParentNode, const char* pchName, const char* pchAttributes, Block* pParentBlock)
{
	Block* pAttributesBlock = new Block(pParentBlock);
	pAttributesBlock->SetData(pchAttributes, (int)strlen(pchAttributes));
	pAttributesBlock->SetID(DoEndian(AddString("Attributes")));

    printf("Using default FoundTagAttributes\n");

	return pAttributesBlock;
}

//------------------------------------------------------------------------------
//  FoundText
//------------------------------------------------------------------------------
Block* XmlDataFile::FoundText(xml::TagNode* pParentNode, const char* pchText, Block* pParentBlock)
{
	Block* pTextBlock = new Block(pParentBlock);
	pTextBlock->SetData(pchText, (int)strlen(pchText));
	pTextBlock->SetID(DoEndian(AddString("Text")));

    printf("Using default FoundText\n");

	return pTextBlock;
}

//------------------------------------------------------------------------------
//  FoundComment
//------------------------------------------------------------------------------
Block* XmlDataFile::FoundComment(xml::TagNode* pParentNode, const char* pchComment, Block* pParentBlock)
{
	Block* pCommentBlock = new Block(pParentBlock);
	pCommentBlock->SetData(pchComment, (int)strlen(pchComment));
	pCommentBlock->SetID(DoEndian(AddString("Comment")));

    printf("Using default FoundComment\n");

	return pCommentBlock;
}

//------------------------------------------------------------------------------
//  IsData
//      used to specify whether the block contents are more blocks or data
//------------------------------------------------------------------------------
bool XmlDataFile::IsData(DWORD dwBlockID)
{
    if ((strcmp(GetString(dwBlockID), "Attributes") == 0) ||
        (strcmp(GetString(dwBlockID), "Text") == 0) ||
        (strcmp(GetString(dwBlockID), "Comment") == 0))
        return true;
    else
        return false;
}

//------------------------------------------------------------------------------
//  GetValueFromKey
//------------------------------------------------------------------------------
char* XmlDataFile::GetValueFromKey(const char* pchData, const char* pchKey, std::string& strValue)
{
    char* pchCurr = (char*)pchData;
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
		            int iStart = (int)(pchOpenQuote - pchData) + 1;
		            int iEnd = (int)(pchCloseQuote - pchData);

		            std::string strData(pchData);

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
//  GetFloatValue
//------------------------------------------------------------------------------
float XmlDataFile::GetFloatValue(const char* pchValue)
{
	float fValue = 0.f;
	sscanf(pchValue, "%f", &fValue);
	return fValue;
}

//------------------------------------------------------------------------------
//  GetIntValue
//------------------------------------------------------------------------------
S32 XmlDataFile::GetIntValue(const char* pchValue)
{
	int iValue = 0;
	sscanf(pchValue, "%d", &iValue);
	return iValue;
}

//------------------------------------------------------------------------------
//  GetIntValue
//------------------------------------------------------------------------------
const char* XmlDataFile::GetFirstNonWhitespace(const char* pchString)
{
	const char* pchChar = pchString;

	if (pchChar)
	{
		while (*pchChar)
		{
			if ((*pchChar != ' ') && (*pchChar != '\t') && (*pchChar != '\r') && (*pchChar != '\n'))
				return pchChar;

			pchChar++;
		}
	}

	return NULL;
}

