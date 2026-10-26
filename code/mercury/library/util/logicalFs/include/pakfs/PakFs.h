#ifndef LOGICALFS_PAKFS_H
#define LOGICALFS_PAKFS_H

#ifdef _MSC_VER
#pragma once
#endif

#include <PakFs/PakGlobals.h>

namespace Sys
{
class I_File;
struct C_FileInfo;
}

namespace Security
{
	class I_Security;
}

namespace Pak
{
	class C_PakFile;
	struct C_PakFatEntry;
}

namespace LogicalFs
{

class C_PakFs : public Sys::I_FileSystem
{
public:
	enum E_Security
	{
		E_Security_Enabled,
		E_Security_Disabled
	};

	virtual ~C_PakFs();

	virtual Sys::I_File*	Open(const Sys::C_FilePath& fileName, 
		const Sys::E_FileAccess fileAccess = Sys::E_FileAccess_Read,
		const Sys::C_OpenParams& params = Sys::C_OpenParams());

	/// returns true if the file exists, or otherwise false
	virtual bool			FileExists( const Sys::C_FilePath &fileName );
 
	/// returns a C_FileInfo structure for a given file
	/// returns false if the file doesn't exist
	virtual bool			FileInfo( const Sys::C_FilePath &fileName, Sys::C_FileInfo *p_FileInfo );

	/// set the access to the given file
	/// returns false if we don't have the required access or the file does not exist
	virtual bool			FileAccess( const Sys::C_FilePath &fileName, U32 fileAccess );

	/// sets the file time for a given file
	/// returns false if an error occurred
	virtual bool			FileTime( const Sys::C_FilePath& fileName, const Time::C_Time &);
 
	/// delete a file or folder with a given name
	/// returns false if an error occurred
	virtual bool			DeleteFile( const Sys::C_FilePath& fileName );

	/// returns directory info in DirectoryInfo
	/// returns false if an error occurred
	virtual bool			DirectoryInfo( const Sys::C_FilePath& filePath, Sys::C_DirectoryInfo * );

	/// create a directory with a given name/path
	/// returns false if the operation failed
	virtual bool			CreateSingleDirectory( const Sys::C_FilePath& filePath );

	/// changes the current working directory
	/// subsequent use of "./filename.ext" will be relative to this directory
	/// returns false if the operation failed
	virtual bool			SetWorkingDirectory( const Sys::C_FilePath& directoryPath );

	/// returns the path of the current working directory
	virtual Sys::C_FilePath	GetWorkingDirectory() const;

	virtual Sys::C_FilePath GetSystemFilePath( const Sys::C_FilePath& filePath ) const;


//From C_PakFs
public:
	C_PakFs();

	bool					OpenPak(
		Sys::I_FileSystem* pFs,
		const Sys::C_FilePath& filePath,
		const Security::I_Security* pSecurity = NULL,
		const Sys::C_OpenParams& params = Sys::C_OpenParams());

	bool					IsOpen();

	void					ClosePak();

	const Pak::C_PakFile*	GetPak() const { return m_pPak; }

	Pak::C_PakFile*			GetPak()  { return m_pPak; }

	const Sys::C_FilePath&	GetPakFilePath() const;

	bool					SwitchPakFile(
		Sys::I_FileSystem* pFs,
		const Sys::C_FilePath& filePath);

	bool					Resume();

	void					Suspend();
	
	void					StopReads();

	bool					ReopenPakFile();

	Pak::C_PakFatEntry*		GetPakFatEntryAccountingForSerialAccess(const Sys::C_FilePath& fileName);

//From C_PakFs
private:
	union C_Flags
	{
		U32 data;
		struct
		{
			bool isOpen : 1;
			bool isSecure : 1;
			bool isModified : 1;
		};
	};

	Pak::C_PakFile*				m_pPak;
	C_Flags						m_flags;
};

}

#endif
