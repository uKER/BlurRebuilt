#include "AudioPch.h"
#include "xmlParser.h"
#include "xmlNode.h"
#include "exception.h"

//  TODO: xml parser ignores <? tags

//------------------------------------------------------------------------------
//  Load
//------------------------------------------------------------------------------
bool xml::Parser::Load(const char* pchFilename, xml::Node** ppNode, bool bStrict)
{
    *ppNode = NULL;
	char strError[256];

	try
	{
		m_iCurrLine = 1;

        if (Open(pchFilename))
        {
            m_strFilename = pchFilename;

			if (bStrict &&  getc(m_pFileIn) != '<')
			{
				sprintf(strError, "STRICT Error: first character is not open tag in file %s\n", pchFilename);
				throw Error(strError);
			}
			rewind(m_pFileIn);

            *ppNode = ReadNode();

            if (!Close())
                throw Error("Closing file");
        }
        else
		{
			sprintf(strError, "Opening file '%s'\n", pchFilename);
            throw Error(strError);
		}

        return true;
	}
	catch (Error& err)
	{
		if (m_pErrorCallback)
		{
			return m_pErrorCallback(err.When(), err.What(), err.Where() );
		}
		else
		{
			printf("Error: %s" , err.What());
			if (err.When() != -1)
				printf(" at line %d", err.When());
			printf("\n");

			delete *ppNode;
			*ppNode = NULL;
	//        Release(*ppNode);
			system("pause 0");

			return false;
		}
	}
}

//------------------------------------------------------------------------------
//  ReadNode
//------------------------------------------------------------------------------
xml::Node* xml::Parser::ReadNode(TagNode* pParentNode)
{
	char chCurr = ReadNextChar();

	if (chCurr == '<')
	{
		//	Comment
		if (PeekNextChar() == '!')
		{
			std::string strComment;

			//	skip '!'
			ReadNextChar();

			//	'-'
			if (ReadNextChar() != '-')
				throw Error("Comment badly formed", m_iCurrLine);

			//	'-'
			if (ReadNextChar() != '-')
				throw Error("Comment badly formed", m_iCurrLine);

			char chNext = ReadNextChar();

            bool bEndComment = false;
			while (!bEndComment)
			{
                if (chNext == '-')
                {
				    strComment += chNext;
				    chNext = ReadNextChar();

                    if (chNext == '-')
                    {
				        strComment += chNext;
				        chNext = ReadNextChar();

                        if (chNext == '>')
                        {
                            strComment.resize(strComment.length()-2);
//				            chNext = ReadNextChar();

                            bEndComment = true;
                            break;
                        }
                    }
                }

				strComment += chNext;
				chNext = ReadNextChar();
			}

            CommentNode* pCommentNode = new CommentNode(pParentNode, GetFilename().data());
			pCommentNode->SetLine(m_iCurrLine);

			pCommentNode->SetComment(strComment.data());

			pCommentNode->SetNodeType(eNodeType_Comment);

			return pCommentNode;
		}
		//	Close tag
		else if (PeekNextChar() == '/')
		{
			//	skip '/'
			ReadNextChar();

			std::string strName = ReadWord();

			if (strcmp(strName.data(), pParentNode->GetType()) != 0)
			 	throw Error("Close tag does not match open tag", m_iCurrLine);

			//	skip '>'
			ReadNextChar();

		 	return NULL;
		}
		//	Open Tag
		else
		{
			std::string strName;;
			std::string strAttributes;

			strName = ReadWord();

			char chNext = PeekNextCharNoWS();

			TagNode* pTagNode = new TagNode(pParentNode, GetFilename().data());
			pTagNode->SetLine(m_iCurrLine);

			pTagNode->SetType(strName.data());

			//	no atts, just open tag (has children?)
			if (chNext == '>')
			{
				//	skip '>'
				ReadNextChar();

				Node* pChildNode = ReadNode(pTagNode);

				while (pChildNode != NULL)
				{
					pTagNode->AddChild(pChildNode);
					pChildNode = ReadNode(pTagNode);
				}
			}
			//	open/close tag, no atts
			else if (chNext == '/')
			{
				//	skip '/>'
				ReadNextChar();
				ReadNextChar();
			}
			//	atts
			else
			{
				while ((chNext != '/') && (chNext != '>'))
				{
                    if (PeekNextCharNoWS() == '<')
                        throw Error("Attribute list contains an open tag", m_iCurrLine);

					strAttributes += ReadWord();

					chNext = ReadNextCharNoWS();

					//	=
					if (chNext == '=')
//						strAttributes += " = ";
						strAttributes += "=";
					else
 						throw Error("Attribute pair has no equals sign", m_iCurrLine);

					//	value
					std::string strValue = ReadWord();

                    if ((strValue.compare("\"\"") == 0) ||
                        (strValue.compare("\'\'") == 0))
						throw Error("Attribute has no value", m_iCurrLine);

					if (!Quoted(strValue))
						throw Error("Attribute value not quoted correctly", m_iCurrLine);

					strAttributes += strValue;

					chNext = PeekNextCharNoWS();

					if ((chNext != '/') && (chNext != '>'))
						strAttributes += ' ';
				}

				pTagNode->SetAttributes(strAttributes.data());

				//	open/close (with atts)
				if (chNext == '/')
				{
                    char ch = ReadNextChar();

					//	skip '>'
					if (PeekNextChar() != '>')
                        throw Error("Tag not closed correctly", m_iCurrLine);
				}
				//	do children
				else if (chNext == '>')
				{
					Node* pChildNode = ReadNode(pTagNode);

					while (pChildNode != NULL)
					{
						pTagNode->AddChild(pChildNode);
						pChildNode = ReadNode(pTagNode);
					}
				}
			}

			pTagNode->SetNodeType(eNodeType_Tag);

			return pTagNode;
		}
	}
	//	Text
	else
	{
		std::string strText;

		char chNext = PeekNextChar();

		while (chNext != '<')
		{
			strText += chNext;

			ReadNextChar();

			chNext = PeekNextChar();
		}

		TextNode* pTextNode = new TextNode(pParentNode, GetFilename().data());
		pTextNode->SetLine(m_iCurrLine);

		pTextNode->SetText(strText.data());

		pTextNode->SetNodeType(eNodeType_Text);

		return pTextNode;
	}
}

//------------------------------------------------------------------------------
//  ShowNode
//------------------------------------------------------------------------------
void xml::Parser::Show(Node* pNode)
{
    xml::TagNode* pTag = dynamic_cast<xml::TagNode*>(pNode);
    xml::TextNode* pText = dynamic_cast<xml::TextNode*>(pNode);
    xml::CommentNode* pComment = dynamic_cast<xml::CommentNode*>(pNode);

    if (pTag)
	{
		printf("<%s", pTag->GetType());

		if (pTag->GetAttributes())
			printf(" %s", pTag->GetAttributes());

		//	if has children
		if (pTag->GetChildren().size())
		{
			printf(">");

			//	do children
            for (Audio::Iterator<Node*> i = pTag->GetChildren().first(); !i.isPastEnd(); i.advance())
			{
				Show(i.retrieve());
			}

			printf("</%s>", pTag->GetType());
		}
		//	else no children
		else
			printf("/>");
	}

	else if (pText)
		printf("%s", pText->GetText());

    else if (pComment)
		printf("<!--%s-->", pComment->GetComment());
}
/*
//------------------------------------------------------------------------------
//  Release
//------------------------------------------------------------------------------
void xml::Parser::Release(xml::Node* pNode)
{
    if (pNode)
    {
	    delete pNode;
        pNode = NULL;
    }
}
*/
//------------------------------------------------------------------------------
//  ReadNextChar
//------------------------------------------------------------------------------
char xml::Parser::ReadNextChar()
{
	if (m_chNext == -1)
	{
		// read ahead
		m_chNext = ReadChar();
	}

	char chThis = m_chNext;

	if (chThis == '\n')
		m_iCurrLine++;

	if (chThis == '/0')
		throw Error("End of char stream", m_iCurrLine);

	if (chThis == EOF)
		throw Error("EOF", m_iCurrLine);

	m_chNext = ReadChar();

	return chThis;
}

//------------------------------------------------------------------------------
//  ReadNextChar
//------------------------------------------------------------------------------
char xml::Parser::ReadNextCharNoWS()
{
	char chCurr = ReadNextChar();

	while (chCurr == ' ' || chCurr == '\t' || chCurr == '\n' || chCurr == '\r')
		chCurr = ReadNextChar();

	return chCurr;
}

//------------------------------------------------------------------------------
//  PeekNextChar
//------------------------------------------------------------------------------
char xml::Parser::PeekNextChar()
{
	return m_chNext;
}

//------------------------------------------------------------------------------
//  PeekNextCharNoWS
//------------------------------------------------------------------------------
char xml::Parser::PeekNextCharNoWS()
{
//	char chCurr = ReadNextChar();

	while (m_chNext == ' ' || m_chNext == '\t' || m_chNext == '\n' || m_chNext == '\r')
		ReadNextChar();

	return m_chNext;
}

//------------------------------------------------------------------------------
//  ReadWord
//------------------------------------------------------------------------------
std::string xml::Parser::ReadWord()
{
	char chCurr, chPeek;
	char chQuotes = '\0';

	std::string strWord;

	chCurr = ReadNextChar();

	while (chCurr == ' ' || chCurr == '\t' || chCurr == '\n' || chCurr == '\r')
		chCurr = ReadNextChar();

	if ((chCurr == '\"') || (chCurr == '\''))
		chQuotes = chCurr;

	strWord += chCurr;

	//	'=' is a word when not in quotes
	if (chCurr == '=')
		return strWord;

	chPeek = PeekNextChar();

    if (chPeek != chQuotes)
    {
	    //	scan till ws, end marker, or end quote
	    while ((chPeek != ' ' && chPeek != '\t' && chPeek != '\n' && chPeek != '\r' && chPeek != '/' && chPeek != '>' && chPeek != '=') || chQuotes)
	    {
		    strWord += ReadNextChar();

		    chPeek = PeekNextChar();

		    if (chPeek == chQuotes)
			    chQuotes = '\0';
	    }
    }
    else
	    strWord += chQuotes;



	return strWord;
}

//------------------------------------------------------------------------------
//  ReadWord
//------------------------------------------------------------------------------
bool xml::Parser::Quoted(std::string& strWord)
{
	char chQuote1 = strWord[0];

	//	starts with quote
	if (chQuote1 == '\"' || chQuote1 == '\'')
	{
		char chQuote2 = strWord[strWord.length()-1];

		//	ends with matching quote
		if (chQuote2 == chQuote1)
			return true;
	}

	return false;
}
