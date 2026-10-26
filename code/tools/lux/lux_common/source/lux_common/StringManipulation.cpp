#include "stdafx.h"
#include <lux_common/LpvsUtility.h>

using namespace LPVS;
/*
bool PathGetDir(const CString& srcPath, CString& dstDirectory)
{
	for (long i=srcPath.GetLength() - 1; (i > -1) && (srcPath.GetAt(i) != '\\'); i--);
	
	dstDirectory = srcPath.Left(i + 1);

	return true;
}

bool PathGetParentFolder(const CString& srcFolder, CString& dstFolder)
{
	long i;

	if (srcFolder.Right(1) == "\\")
	{
		i = srcFolder.GetLength() - 2;
	}
	else
	{
		i = srcFolder.GetLength() - 1;
	}

	for (; (i > -1) && (srcFolder.GetAt(i) != '\\'); i--);
	
	dstFolder = srcFolder.Left(i + 1);

	return true;
}

bool PathGetFilename(const CString& srcPathName, CString& dstFilename)
{
	long i;
	CString t;

	for (i=srcPathName.GetLength()-1; (i > -1) && (srcPathName.GetAt(i) != '\\') && (srcPathName.GetAt(i) != '/'); i --);
	t = srcPathName.Right(srcPathName.GetLength () - i - 1);

	for (i = t.GetLength () - 1; i > -1 && t.GetAt (i) != '.'; i --);
	
	if (i == -1)	dstFilename = t;
	else			dstFilename = t.Left(i);

	return true;
}

bool PathGetFileNameExt(const CString& Path, CString &Dest, long nSubFolders)
{
	long i=Path.GetLength()-1;

	for (long j=0; j <= nSubFolders; j++)
	{
		for (; (i > -1) && (Path.GetAt(i) != '\\'); i--);

		if ((j < nSubFolders) && (i > -1) && (Path.GetAt(i) == '\\'))
			i--;
	}

	Dest = Path.Right(Path.GetLength()-i-1);

	return true;
}

bool PathGetExtension(const CString& sPath, CString &sExt)
{
	long i;

	for (i = sPath.GetLength () - 1; i > -1 && sPath.GetAt (i) != '.'; i --);
	sExt = sPath.Right (sPath.GetLength () - i - 1);

	return true;
}
*/
