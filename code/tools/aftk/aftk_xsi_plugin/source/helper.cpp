#include "XSIA3DExportPCH.h"

//*****************************************************************************
/*!	\file helper.cpp
	\brief Helper classes for reading and writing mesh data.
	
	(c) Copyright 1996 - 2005 Softimage/Avid Corporation	
*/
//*****************************************************************************

#include <xsi_string.h>
#include <xsi_value.h>
#include <xsi_status.h>

#include "helper.h"

#include <wchar.h>
#include <string.h>

#define MAX_CHARS 200

// logger
CMeshFileWriter::CMeshFileWriter() : p (NULL) {}
CMeshFileWriter::~CMeshFileWriter()
{
	if (p)
	{
		fclose(p);
	}
}

XSI::CStatus CMeshFileWriter::Init(const XSI::CString& in_outFile )
{
	p = fopen( in_outFile.GetAsciiString(), "wt" );
	
	if (!p) return XSI::CStatus::InvalidArgument;
	
	return XSI::CStatus::OK;	
}

void CMeshFileWriter::Header( const XSI::CString& in_strText )
{	
	Write( L"\n" );
	XSI::CString str = in_strText;
	str += L"\n{\n";
	
	Write( str );
}

void CMeshFileWriter::Footer()
{
	Write( L"}\n\n" );
}

void CMeshFileWriter::Write( const XSI::CString& in_str )
{	
	const char* psz = in_str.GetAsciiString();
	size_t nLen = strlen(psz);
	fwrite( psz, sizeof(char), nLen, p );		 
}

void CMeshFileWriter::Write( const wchar_t* in_pstr )
{
	XSI::CString str(in_pstr);
	Write( str );
}

void CMeshFileWriter::Write( XSI::CValue in_val )
{
	Write( in_val.GetAsText().GetWideString() );
}

void CMeshFileWriter::EOL()
{
	Write( L"\n" );
}

CMeshFileReader::CMeshFileReader() : 
	p (NULL)
	,m_pszRow(NULL)
	,m_pLongs(NULL)
	,m_pDoubles(NULL)
{}

CMeshFileReader::~CMeshFileReader()
{
	if (p) fclose(p);

	if (m_pLongs) free(m_pLongs);
	if (m_pDoubles) free(m_pDoubles);
	if (m_pszRow) free(m_pszRow);	
	
}

XSI::CStatus CMeshFileReader::Init(const XSI::CString& in_inFile)
{
	p = fopen( in_inFile.GetAsciiString(), "rt" );	
	if (!p) return XSI::CStatus::InvalidArgument;
	
	m_pszRow = (char*)malloc( MAX_CHARS );
	return XSI::CStatus::OK;	
}

bool CMeshFileReader::IsEOF()
{
	if (!p) return true;
	return feof(p) == EOF;
}

XSI::CStatus CMeshFileReader::GotoSection( const XSI::CString& in_header )
{	
	// go to beginning of file
	fseek(p,0L,SEEK_SET);
	
	while (!feof(p))
	{
		GetRow();
		
		// remove ending CR if necessary
		size_t nLen = strlen(m_pszRow);
		if (m_pszRow[nLen-1] == '\n')		
			m_pszRow[nLen-1] = NULL;
		
		XSI::CString str;
		str.PutAsciiString(m_pszRow);
		bool bFound = in_header.IsEqualNoCase(str);
		if (bFound)
		{
			// go to first row of values
			if ( fgetc(p) == '{' && fgetc(p) == '\n')
			{				
				return XSI::CStatus::OK;
			}
			// wrong format
			return XSI::CStatus::Fail;
		}			
	}	
	return XSI::CStatus::OK;
}

bool CMeshFileReader::EndSection()
{
	int ch = fgetc(p);
	if ( '}' == ch || EOF == ch)
	{
		return true;
	}
	
	ungetc(ch,p);
	return false;
}

const char* CMeshFileReader::GetRow()
{
	return fgets( m_pszRow, MAX_CHARS, p );
}

const char* CMeshFileReader::GetStringValues()
{	
	const char* pszRow = GetRow();
	if (!pszRow) return NULL;
	
	size_t nLen = strlen(pszRow);	
	for (size_t i = 0; i<nLen; i++ )
	{
		if (pszRow[i] == ':') 
			// skip first blank in the row
			// e.g. "1 2 3\n"
			return &pszRow[i+2 < nLen ? i+2 : i];
	}
	
	return NULL;	
}

LONG CMeshFileReader::GetLongValues(LONG** out_pVals)
{		
	if (m_pLongs)
	{	
		free(m_pLongs);
		m_pLongs = NULL;
	}

	*out_pVals = NULL;
	
	const char* pszVals = GetStringValues();
	if (!pszVals) return 0;

	size_t nLen = strlen(pszVals);
	LONG nCount = 0; // n of values
	LONG start = 0;
	for ( size_t i = 0; i<nLen ; i++ )	
	{	
		if (pszVals[i] == ' ' || pszVals[i] == '\n') 
		{
			// add one more value
			m_pLongs = (LONG*)realloc( m_pLongs, sizeof(LONG)*(++nCount) );
			
			// get string value 
			char strChunk[100];
			strncpy(strChunk,&pszVals[start],(i-start)+1);
			strChunk[i-start] = NULL;
			 
			// store in output buffer
			char* pStopScan;
			m_pLongs[nCount-1] = strtol( strChunk, &pStopScan, 10 );
			
			// set new start index by skipping the blank separator
			start = (LONG)(i+1);
		}
	}
	*out_pVals = m_pLongs;
	return nCount;
}

LONG CMeshFileReader::GetDoubleValues(double** out_pVals)
{		
	if (m_pDoubles)
	{	
		free(m_pDoubles);
		m_pDoubles = NULL;
	}

	*out_pVals = NULL;	
	
	const char* pszVals = GetStringValues();
	if (!pszVals) return 0;

	size_t nLen = strlen(pszVals);
	LONG nCount = 0; // n of values
	LONG start = 0;
	for ( size_t i = 0; i<nLen ; i++ )	
	{			
		if (pszVals[i] == ' ' || pszVals[i] == '\n') 
		{
			// add one more value
			m_pDoubles = (double*)realloc( m_pDoubles, sizeof(double)*(++nCount) );
			
			// get string value 
			char strChunk[100];
			strncpy(strChunk,&pszVals[start],(i-start)+1);
			strChunk[i-start] = NULL;
			 
			// store in output buffer
			char* pStopScan;
			m_pDoubles[nCount-1] = strtod( strChunk, &pStopScan );
			
			// set new start index by skipping the blank separator
			start = (LONG)(i+1);
		}
	}

	*out_pVals = m_pDoubles;
	
	return nCount;
}
