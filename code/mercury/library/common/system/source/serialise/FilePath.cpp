#include "SystemPch.h"

#include <system/serialise/FilePath.h>

/*360 XDK notes
ms-help://MS.VSCC.v90/MS.VSIPCC.v90/MS.Xbox360SDK/Xbox360SDK/dev_overview_filenames.htm

Directory, File, and Processing Names
General rules for applications creating names for directories and files or processing names supplied by the user include the following: 

In general, use only printable characters in a file name. Do not use a path separator, a character in the range 0 through 31, or any character explicitly disallowed by the file system. A specific file system may allow only a certain set of characters in a file name. The FATX file system used for the console hard disk supports the following characters in file names: 

! # $ % & ' ( ) - . @ [ ] ^ _ ` { } ~ 
Numerals 0 through 9 
Uppercase and lowercase letters A through Z 
Use the backslash (\) to separate components in a path. No other character is acceptable as a path separator. 
UNC names are not supported. UNC names are of the form \\server\share. 
Neither the use of a dot (.) as a directory component in a path to represent the current directory, nor two consecutive dots (..) to represent the parent of the current directory, are supported. 
Use a dot (.) to separate the base file name from the extension in a directory name or file name. 
Do not use the following characters in directory names or file names, because they are reserved: 

>< = ? : ; " * + , / \ |

Do not use characters values 128 through 255. 
All paths must be specified as fully-qualified path names beginning at the root of a drive and including the drive letter, directories, and file name. For example, "Z:\ai\search.ait". There are no current directory or current drive defaults. 
Process a path as a null-terminated string. The maximum length for a FATX full path (directories and file name), including a trailing backslash, is given by MAX_PATH. A UDFX full path can be greater than MAX_PATH, but each component (directory or file name) will be MAX_PATH characters or less. 
Do not assume case sensitivity. Consider names such as OSCAR, Oscar, and oscar to be the same. 
By following the rules listed in this section, an application can create valid names for files and directories regardless of the file system in use (hard disk or DVD).

Backslashes (\) are used as element dividers in paths (dividing the file name from the path to it, or directories from one another in a path). You cannot use them in file or directory names. They may be required 
*/

namespace Sys
{

void C_FilePath::TokeniseByDir(Btl::Vector<C_FilePath>& filePaths) const
{
	const char tok = GetLocalDirToken();
	
	const S32 length = Length();

	Btl::Vector<char> tmp;

	const char* pSrcChars = Chars();
	for (S32 i=0; i<length; i++)
	{
		const char c = pSrcChars[i];
		if (c == tok)
		{
			if (!tmp.Empty())
			{
				tmp.PushBack('\0');
				filePaths.PushBack();
				filePaths.Back().m_formattedFilePath = &tmp[0];
				tmp.Clear();
			}
		}
		else
		{
			tmp.PushBack(c);
		}
	}
	if (!tmp.Empty())
	{
		tmp.PushBack('\0');
		filePaths.PushBack();
		filePaths.Back().m_formattedFilePath = &tmp[0];
	}
}

bool IsRelativePath(const C_FilePath& path)
{
	if (path.Length() == 0)
		return false;

	if (path.String().Search(':') != -1)
		return false;

	const char *p = path.Chars();
	if (p[0] == GetLocalDirToken())
		return false;

	return true;
}

C_FilePath ConvertToCurrentDirectory(
	const C_FilePath& currentDirectory, const C_FilePath& filePath)
{
	// if the filename isn't a relative path then return it as is
	if (!IsRelativePath(filePath))
		return filePath;

	if (currentDirectory.Length() == 0)
		return filePath;

	// Make sure that the join between the current working directory and the filePath are
	// going to marry up correctly i.e. Remove "\" ".\" 
	int stripPos = 0;
	char *p = (char *)filePath.Chars();
	if (p[0] == GetLocalDirToken())
		stripPos = 1;
	if (filePath.Length() == 1 && p[0] == '.')
		stripPos = 1;
	else if ((p[0] == '.') && ((p[1] == '\\') || (p[1] == '/'))) 
		stripPos = 2;

	return currentDirectory + "\\" + filePath.String()(stripPos, filePath.Length());
}

C_FilePath ExtractDirectory(const C_FilePath& filePath)
{
    S32 slashPos = filePath.String().RevSearch(GetLocalDirToken());
    if(slashPos < 0)
    {
        return C_FilePath();
    }
    else
    {
        return C_FilePath(filePath.String()(0, slashPos + 1));
    }
}

C_FilePath ExtractFilename(const C_FilePath& filePath)
{
    S32 startPos;

    S32 slashPos = filePath.String().RevSearch(GetLocalDirToken());
    if(slashPos < 0)
    {
        startPos = 0;
    }
    else
    {
        startPos = slashPos + 1;
    }

	return C_FilePath(filePath.String()(startPos, filePath.Length()));
}

C_FilePath RemoveFileExtension(const C_FilePath& filePath)
{
    // extension may have more than one dot
    // reverse search for slash, this is where the filename starts
    S32 slashPos = filePath.String().RevSearch(GetLocalDirToken());
    if(slashPos < 0)
    {
        slashPos = 0;
    }

    // then forward search for dot from start of filename
    S32 dotPos = filePath.String().RevSearch('.');
    if(dotPos <= 0 || slashPos > dotPos)
        return C_FilePath();
    else
		return C_FilePath(filePath.String()(0, dotPos));
}

C_FilePath ExtractFileExtension(const C_FilePath& filePath)
{
    // extension may have more than one dot
    // reverse search for slash, this is where the filename starts
	S32 slashPos = filePath.String().RevSearch(GetLocalDirToken());
    if(slashPos < 0)
    {
        slashPos = 0;
    }

    // then forward search for dot from start of filename
    S32 dotPos = filePath.String().RevSearch('.');
    if(dotPos < 0 || slashPos > dotPos)
        return C_FilePath();
    else
        return filePath.String()(dotPos + 1, filePath.Length());
}

C_FilePath ReplaceFileExtension(const C_FilePath& filePath, const C_FilePath& extension)
{
	return RemoveFileExtension(filePath) + C_FilePath(".") + extension;
}

C_FilePath AddTrailingSlash(const C_FilePath& filePath)
{
	return filePath + GetLocalDirToken();
}

C_FilePath RemoveTrailingSlash(const C_FilePath& filePath)
{
	if (filePath.Length() > 0)
	{
		char c = filePath[filePath.Length()-1];
		if (c == GetLocalDirToken())
			return filePath.String()(0, filePath.Length()-1);
	}

	return filePath;
}

C_FilePath RemoveLeadingSlash(const C_FilePath& filePath)
{
	if (filePath.Length() > 0)
	{
		char c = filePath[0];
		if (c == GetLocalDirToken())
			return filePath.String()(1, filePath.Length());
	}

	return filePath;
}

/// concatenates the strings as follows: directory + filename + "." + extension
C_FilePath ConstructFilePath( const C_FilePath& dir, const C_FilePath& filename, const C_FilePath& extension)
{
	C_FilePath result;
	if(extension == "")
	{
		result = dir + filename;
	}
	else
	{
		result = dir + filename + C_FilePath(".") + extension;
	}

	return result;
}

/// de-concatenates? the strings as follows: directory + filename + "." + extension
void DeconstructFileString(const C_FilePath& input, C_FilePath& path, C_FilePath& filename, C_FilePath& extension)
{
	path = ExtractDirectory(input);
	filename = RemoveFileExtension(	ExtractFilename(input) );
	extension = ExtractFileExtension(input);
}

}
