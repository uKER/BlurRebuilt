#ifndef C_MAKEPAK_H
#define C_MAKEPAK_H


#ifdef _MSC_VER
#pragma once
#endif


#include "PakFileSystem/PAKFileSystem.h"
#include "PakFileSystem/PAKLoadManager.h"

//-------------------------------------------------------------------
// Class : C_MakePAK
//-------------------------------------------------------------------
class C_MakePAK
{
public:
	C_MakePAK( Debug::Logger& log );

	~C_MakePAK();

	void SetEndianSwap( bool swap );
	void CreatePAKFileList();
	void SetPakRoot( Util::C_String root );
	void AddPakToList( Util::C_String filename );

	bool CreatePAKFromFolder( 
		Util::C_String filename,
		Util::C_String pakMountPoint,
		bool compress, 
		U32 sectorSize,
		Util::C_String excludeList );

	void XML_ReadEndian();
	void XML_ReadRootFolder();
	void XML_ReadFiles(U32 sectorSize);
	bool XML_Read( const char *xml_file, U32 sectorSize );

private:
	Debug::Logger&	m_Log;
	Ut::Vector<Util::C_String> mOrderedFiles;

	bool			mEndianSwap;
	FILE*			fp;
	Util::C_String	mPakRoot;
	Sys::C_XmlDom	dom;
};

#endif // C_MAKEPAK_H