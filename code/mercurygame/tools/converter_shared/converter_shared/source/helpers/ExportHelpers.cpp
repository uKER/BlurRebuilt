
#include "stdafx.h"

#include <system/misc/StringUtil.h>
#include <tools_shared/PlatformHelpers.h>
#include <tools_shared/file_copy/FileCopyList.h>

#include <converter_shared/helpers/ExportHelpers.h>

#include <stdio.h>
#include <windows.h>
#include <ole2.h>
#include <system/Time/Time.h>
#include <system/debug/BuildStamp.h>

// Implicitly link ole32.dll

#pragma comment( lib, "ole32.lib" )

bool SetFileProperties( const Util::C_String& filename, 
					   const Util::C_String& title,
					   const Util::C_String& subject,
					   const Util::C_String& author,
					   const Util::C_String& keywords,
					   const Util::C_String& comments )
{
	WCHAR wcFilename[1024];
	swprintf( wcFilename, filename.Length() + 1, L"%S", filename.Chars() );

	IPropertySetStorage *pPropSetStg = NULL;
	IPropertyStorage *pPropStg = NULL;
	HRESULT hr;

	//Get the IPropertySetStorage interface
	hr = StgOpenStorageEx(wcFilename, STGM_READWRITE|STGM_SHARE_EXCLUSIVE, STGFMT_ANY, 0, NULL, NULL, IID_IPropertySetStorage, (void**)&pPropSetStg );
	if(FAILED(hr))
		return false;

	//Open summary information, getting an IpropertyStorage.
    hr = pPropSetStg->Create( FMTID_SummaryInformation, NULL, 
                            PROPSETFLAG_DEFAULT,
                            STGM_CREATE|STGM_READWRITE|
                            STGM_SHARE_EXCLUSIVE,
                            &pPropStg );

	if(FAILED(hr))
		return false;

	//Tell Windows to set Title,Subject,Author,Keywords and Comments properties
	const S32 propertyCount = 5;
	PROPSPEC ps[propertyCount];
	ps[0].ulKind = PRSPEC_PROPID;
	ps[0].propid = PIDSI_TITLE;
	ps[1].ulKind = PRSPEC_PROPID;
	ps[1].propid = PIDSI_SUBJECT;
	ps[2].ulKind = PRSPEC_PROPID;
	ps[2].propid = PIDSI_AUTHOR;
	ps[3].ulKind = PRSPEC_PROPID;
	ps[3].propid = PIDSI_KEYWORDS;
	ps[4].ulKind = PRSPEC_PROPID;
	ps[4].propid = PIDSI_COMMENTS;

	//Properties values
	PROPVARIANT pv[propertyCount];
	pv[0].vt = VT_LPSTR;
	pv[0].pszVal = (LPSTR)title.Chars();
	pv[1].vt = VT_LPSTR;
	pv[1].pszVal = (LPSTR)subject.Chars();
	pv[2].vt = VT_LPSTR;
	pv[2].pszVal = (LPSTR)author.Chars();
	pv[3].vt = VT_LPSTR;
	pv[3].pszVal = (LPSTR)keywords.Chars();
	pv[4].vt = VT_LPSTR;
	pv[4].pszVal = (LPSTR)comments.Chars();

	//Write to file
	pPropStg->WriteMultiple(propertyCount, ps, pv, 0);

	//Release interfaces in the reverse order in which they where obtained
	pPropStg->Release();
	pPropStg = NULL;

	pPropSetStg->Release();
	pPropSetStg = NULL;

	return true;
}

namespace ExportHelpers
{

void SetExportFileProperties(const Util::C_String& filename, const Util::E_Platform platform)
{
	Util::C_String title = Util::RemoveFileExtension( Util::ExtractFilename( filename ) ) + " " + Util::ExtractFileExtension( filename );

	Util::C_String subject = 
		Util::C_String(Util::GetPlatformString(platform)).Upper()
		+ " Version";

	char computerName[MAX_COMPUTERNAME_LENGTH + 1];
	DWORD computerNameBufferSize = MAX_COMPUTERNAME_LENGTH;
	BOOL success;
	
	success = GetComputerName(computerName, &computerNameBufferSize);

	char userName[MAX_COMPUTERNAME_LENGTH + 1];
	DWORD userNameBufferSize = MAX_COMPUTERNAME_LENGTH;
	success = GetUserName(userName, &userNameBufferSize);

	Util::C_String author = Util::C_String(userName) + " on " + Util::C_String(computerName);
	Util::C_String keywords;

	Util::C_String comments;

	Time::C_DecomposedTime decomposedTime = Time::GetCurrentTime();
	Util::C_String date = Time::FormatDateString(decomposedTime);
	Util::C_String time = Time::FormatTimeString(decomposedTime);
	
	Util::C_String appName;
	char fullAppName[MAX_PATH + 1];
	DWORD appNameBufferSize = MAX_PATH;
	GetModuleFileName( GetModuleHandle( 0 ), fullAppName, appNameBufferSize);
	appName = Util::ExtractFilename( Util::RemoveFileExtension( fullAppName ) );

	comments.Printf( "Exported %s(%s).\r\n%s built %s(%s) on machine %s.\r\nOriginal export path '%s'", date.Chars(), time.Chars(), appName.Chars(), Debug::GetBuildDate(), Debug::GetBuildTime(), Debug::GetBuildPc(), filename.Chars() );

	SetFileProperties( filename, title, subject, author, keywords, comments );
}

Sys::I_File* WriteExportFile( const Util::C_String& outputFilename, const Util::E_Platform platform, Tools::FileCopy::C_FileCopyList* pFileCopyList )
{
	bool success = true;

	LOGDEBUG( Tools::g_log, "Creating file '%s'", outputFilename.Chars());

	//Create output folder.
	const Util::C_String outputDirectory = Util::ExtractDirectory( outputFilename );
	success = Sys::C_FileSystem::Instance().CreateDirectory( outputDirectory );
	if (!success)
	{
		LOGWARN( Tools::g_log, "Failed to create output directory '%s'", outputDirectory.Chars());
		return NULL;
	}

	Sys::I_File* pFile = Sys::C_FileSystem::Instance().Open(
		outputFilename.Chars(), 
		Sys::E_FileAccess_Write);

	SetExportFileProperties( outputFilename, platform );

	if (!pFile)
	{
		LOGWARN( Tools::g_log, "Failed to create output file '%s'", outputFilename.Chars());
		return NULL;
	}

	const bool endianSwap = Tools::ShouldEndianSwap( platform );
	pFile->SetEndianSwapFlag(endianSwap);

	pFileCopyList->AddFileCopyItem( platform, Tools::FileCopy::C_FileCopyItem((const char*)outputFilename.Chars(), "") );

	return pFile;
}


} // namespace ExportHelpers