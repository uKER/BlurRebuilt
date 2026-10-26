#ifndef XMLPARSER_H
#define XMLPARSER_H

#ifdef _MSC_VER
#pragma once
#endif

#include <list>
#include "xmlNode.h"

#define USE_XML_FILE_SYSTEM

typedef bool (*ERROR_CALLBACK) ( unsigned int u32LineNumber, char* strError, char* strFile );

//  TODO:   ignore leading whitespace on xml files (currently will miss includes..)
//          
namespace xml
{
//------------------------------------------------------------------------------
//  Parser
//------------------------------------------------------------------------------
	class Parser
	{
	public:
		Parser() 
        {
#ifdef USE_XML_FILE_SYSTEM
            m_pFileIn = NULL;
#endif // USE_XML_FILE_SYSTEM
            m_chNext = -1;
            m_iCurrLine = 0;
			m_pErrorCallback = 0;
        }
		virtual ~Parser() {}

		bool Load(const char* pchFilename, xml::Node** ppNode, bool bStrict = false);

        void Show(Node* pNode);

        const std::string& GetFilename() { return m_strFilename; }

		void SetErrorCallback(ERROR_CALLBACK pCallback) { m_pErrorCallback = pCallback; }

    protected:
#ifdef USE_XML_FILE_SYSTEM
        virtual bool Open(const char* pchFilename)
        {
            Close();
            m_pFileIn = fopen(pchFilename, "rb");
            return (m_pFileIn != NULL);
        }
        virtual char ReadChar()
        {
            return getc(m_pFileIn);
        }
        virtual bool Close()
        {
            int iRet = 0;
            if (m_pFileIn)
            {
		        int iRet = fclose(m_pFileIn);
		        m_pFileIn = NULL;
            }
            return (iRet != EOF);
        }
#else
        virtual bool Open(const char* pchFilename) = 0;
        virtual char ReadChar() = 0;
        virtual bool Close() = 0;
#endif // USE_XML_FILE_SYSTEM

	protected:
		//	read data from current file pos and fill this block
		//	(also does children recursively, unless instructed to skip them)
		Node* ReadNode(TagNode* pParentNode = NULL);

		char ReadNextChar();
		char ReadNextCharNoWS();
		char PeekNextChar();
		char PeekNextCharNoWS();
		std::string ReadWord();
		bool Quoted(std::string& strWord);

	protected:
        std::string m_strFilename;
		ERROR_CALLBACK m_pErrorCallback;

#ifdef USE_XML_FILE_SYSTEM
	    FILE* m_pFileIn;
#endif // USE_XML_FILE_SYSTEM

		char m_chNext;

		int m_iCurrLine;
	};
}

#endif
