//*****************************************************************************
/*!	\file helper.h
	\brief Helper classes for reading and writing mesh data.
	
	(c) Copyright 1996 - 2005 Softimage/Avid Corporation
	
*/
//*****************************************************************************

#ifndef _HELPER_H_
#define _HELPER_H_

#include <stdio.h>

class XSI::CString;
class XSI::CValue;
class XSI::CStatus;

//*****************************************************************************
/*! \class CMeshFileWriter helper.h
	Helper class for writing mesh data to a text file.
	A mesh exported with CMeshFileWriter can be imported with CMeshFileReader.
 */
//*****************************************************************************

class CMeshFileWriter
{
	public:

	CMeshFileWriter();
	~CMeshFileWriter();

	XSI::CStatus Init(const XSI::CString& in_outFile);

	void Header( const XSI::CString& in_str );
	void Footer();
	void Write( const XSI::CString& in_str );
	void Write( const wchar_t* in_pstr );
	void Write( XSI::CValue in_val );	
	void EOL();
		
	private:
	FILE* p;
};

//*****************************************************************************
/*! \class CMeshFileReader helper.h
	Helper class for importing a mesh into XSI from a text file generated 
	by CMeshFileWriter.
 */
//*****************************************************************************
class CMeshFileReader
{
	public:
	CMeshFileReader();
	~CMeshFileReader();

	XSI::CStatus Init(const XSI::CString& in_inFile);
	
	XSI::CStatus GotoSection(const XSI::CString& in_header);
	bool EndSection();
	bool IsEOF();

	LONG GetLongValues(LONG** out_pVals);
	LONG GetDoubleValues(double** out_pVals);
	
	private:
	const char* GetStringValues();
	const char* GetRow();

	char* m_pszRow;
	LONG* m_pLongs;
	double* m_pDoubles;
	FILE* p;
};

#endif