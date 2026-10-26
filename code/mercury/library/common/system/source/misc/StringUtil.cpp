#include "SystemPch.h"

#include <system/misc/StringUtil.h>

#ifdef TOOL

namespace Util
{

/// returns the contents of the string up to and including the last slash
/// returns the empty string if there is no slash
C_String ExtractDirectory(const C_String& inString)
{
    Util::C_String testString = inString.Replace("/", "\\");

    S32 slashPos = testString.RevSearch('\\');
    if(slashPos < 0)
    {
        return Util::C_String("");
    }
    else
    {
        return inString(0, slashPos + 1);
    }
}

/// returns the contents of the string after the last slash
/// returns the entire string if there is no slash
C_String ExtractFilename(const C_String& inString)
{
    Util::C_String testString = inString.Replace("/", "\\");

    S32 startPos;

    S32 slashPos = testString.RevSearch('\\');
    if(slashPos < 0)
    {
        startPos = 0;
    }
    else
    {
        startPos = slashPos + 1;
    }

	return inString(startPos, testString.Length());
}

/// returns the contents of the string without the last '.' and any characters after it
/// returns the entire string if there is no '.'
C_String RemoveFileExtension(const C_String& inString)
{
    Util::C_String testString = inString.Replace("/", "\\");

    // extension may have more than one dot
    // reverse search for slash, this is where the filename starts
    S32 slashPos = testString.RevSearch('\\');
    if(slashPos < 0)
    {
        slashPos = 0;
    }

    // then forward search for dot from start of filename
    S32 dotPos = testString.Search( '.', slashPos );
    if(dotPos <= 0)
    {
        return inString;
    }
    else
    {
        return inString(0, dotPos);
    }
}

/// returns the contents of the string after the last '.'
/// returns an empty string if there is no '.'
C_String ExtractFileExtension(const C_String& inString)
{
    Util::C_String testString = inString.Replace("/", "\\");

    // extension may have more than one dot
    // reverse search for slash, this is where the filename starts
    S32 slashPos = testString.RevSearch('\\');
    if(slashPos < 0)
    {
        slashPos = 0;
    }

    // then forward search for dot from start of filename
    S32 dotPos = testString.Search( '.', slashPos );
    if(dotPos < 0)
    {
        return Util::C_String("");
    }
    else
    {
        return inString(dotPos + 1, testString.Length());
    }
}

/// replace the contents of the string after but excluding the last '.' with the given extension
/// appens to the end if there is no '.'
C_String ReplaceFileExtension(const C_String& inString, const C_String& extension)
{
	return RemoveFileExtension(inString) + C_String(".") + extension;
}

/// concatenates the strings as follows: directory + filename + "." + extension
C_String ConstructFileString( const C_String& dir, const C_String& filename, const C_String& extension)
{
	Util::C_String result;
    if(extension == Util::C_String(""))
    {
        result = dir + filename;
    }
    else
    {
        result = dir + filename + Util::C_String(".") + extension;
    }

	return result;
}
/// de-concatenates? the strings as follows: directory + filename + "." + extension
void DeconstructFileString( const C_String& input, C_String& path, C_String& filename, C_String& extension)
{
	path = ExtractDirectory(input);
	filename = RemoveFileExtension(	ExtractFilename(input) );
	extension = ExtractFileExtension(input);
}



/// flip slashes from windows ('\\') to mercury file system format ('/')
C_String FormatFileString(const C_String &inString)
{
   Util::C_String rtn = inString.Replace("\\", "/");
   return rtn;
}

/// flip slashes from windows ('\\') to unix format ('/')
C_String FormatUnixFileString(const C_String &inString)
{
   Util::C_String rtn = inString.Replace("\\", "/");
#ifdef PLATFORM_PS3
	char searchStr[] = "/USRDIR/";
	char *lower = (char *)strstr(rtn.Chars(), searchStr);
	if (lower)
	{
		U32 start = strlen(searchStr);
		U32 end = strlen(lower);
		for (U32 i = start ; i < end ; i++)
		{
			if (lower[i] >= 'A' && lower[i] <= 'Z') lower[i] += 0x20;
		}
	}
#else
   rtn.ConvertToLower();
#endif

   return rtn;
}

/// flip slashes from unix format ('/') to windows ('\\')
//lint -save -e111
C_String FormatWindowsFileString(const C_String& inString)
{
   Util::C_String rtn = inString;//.Replace("/", "\\");
   for (U32 i = 0 ; i < (U32)inString.Length() ; i++)
   {
		if ( rtn[i] == '/') rtn[i] = '\\';
   }
   rtn.ConvertToLower();
   return rtn;
}
//lint -restore

C_String FormatWindowsFileString256(const C_String256& inString)
{
   Util::C_String256 rtn = inString;//.Replace("/", "\\");
   for (U32 i = 0 ; i < (U32)inString.Length() ; i++)
   {
		if ( rtn.NonConstChars()[i] == '/') rtn.NonConstChars()[i] = '\\';
		if ( rtn.NonConstChars()[i] >= 'A' && rtn.NonConstChars()[i] <= 'Z') rtn.NonConstChars()[i] += 0x20;
   }
   return rtn;
}

C_String AddTrailingSlash(const C_String& inString, char slash)
{
	if (inString.Length() > 0)
	{
		char c = inString[inString.Length()-1];
		if (c == '\\' || c == '/')
			return inString;
	}

	return inString + Util::C_String(slash);
}

C_String RemoveTrailingSlash(const C_String& inString)
{
	if (inString.Length() > 0)
	{
		char c = inString[inString.Length()-1];
		if (c == '\\' || c == '/')
			return inString(0, inString.Length()-1);
	}

	return inString;
}

C_String RemoveLeadingSlash(const C_String& inString)
{
	if (inString.Length() > 0)
	{
		char c = inString[0];
		if (c == '\\' || c == '/')
			return inString(1, inString.Length());
	}

	return inString;
}

C_String RemoveTrailingSpaces(const C_String& inString)
{
	if (inString.Length() > 0)
	{
		int i=0;
		while (inString[inString.Length()-i-1]==' ' && i<(inString.Length()-1))
			i++;
		
		return inString(0, inString.Length()-i);
	}

	return inString;
}


/// flip slashes to local format (format for target platform)
/// note, the mercury file system always uses '/' regardless of the platform
C_String FormatLocalFileString(const C_String& inString)
{
	#if defined(PLATFORM_PC)
	return FormatWindowsFileString(inString);
	#elif defined(PLATFORM_PS3)
	return FormatUnixFileString(inString);
	#elif defined(PLATFORM_LINUX)
	return FormatUnixFileString(inString);
	#else
	#error ("*** ERROR: Unsupported platform")
	#endif
}

/// Escape escape chars.
C_String EscapeEscapeCharacters(const C_String& inString)
{
	return inString.Replace("\\", "\\\\");
}

bool IsRelativePath(const Util::C_String &path)
{
	if (path.Length() == 0)
		return false;

	if (path.Search(':') != -1)
		return false;

	const char *p = path.Chars();
	if (p[0] == '\\' || p[0] == '/')
		return false;

	return true;
}


const Util::C_String ConvertFilenameToCurrentDirectory(const Util::C_String &currentDirectory, const Util::C_String &filename)
{
	// if the filename isn't a relative path then return it as is
	if (!IsRelativePath(filename))
		return filename;

	if (currentDirectory.Length() == 0)
		return filename;

	// Make sure that the join between the current working directory and the filename are
	// going to marry up correctly i.e. Remove "\" ".\" 
	int stripPos = 0;
	char *p = (char *)filename.Chars();
	if ((p[0] == '\\') || (p[0] == '\\')) stripPos = 1;
	if ((p[0] == '.') && ((p[1] == '\\') || (p[1] == '/'))) stripPos = 2;
//	Util::C_String fname = &p[stripPos];
	// prepend with the current directory and return that
//	return Util::C_String(currentDirectory + "\\" + fname);
	Util::C_String fname ( currentDirectory + "\\" + filename( stripPos, filename.Length() ) );
	return fname;
}


#ifdef PLATFORM_PC

// [Blur2PC] The Alienbrain workspace drive (retail hard-coded z:) comes from BIZARRE_ROOT, the
// variable the build scripts use for that share; unset, paths pass through unchanged.
static C_String AlienBrainDrive()
{
	const char* root = getenv("BIZARRE_ROOT");
	return (root && *root) ? C_String(root) + "\\" : C_String("");
}
#define ALIEN_BRAIN_DRIVE	AlienBrainDrive()
#define ALIEN_BRAIN_FOLDER	"alienbrainwork\\"

C_String ConvertABNetworkPathToLocalPath(const C_String& assetPath)
{
	Util::C_String result(assetPath);
	result = FormatWindowsFileString(result);
	// Look for /<machine_name>/alienBrainWork and convert back to Z:
	S32 pos = result.ISearch(ALIEN_BRAIN_FOLDER);
	if (pos == -1)
		return result;

	result = ALIEN_BRAIN_DRIVE + result(pos + (S32)strlen(ALIEN_BRAIN_FOLDER), result.Length());
	return result;
}

C_String ConvertLocalPathToABNetworkPath(
	const C_String& assetPath,
	const C_String& computerName)
{
	Util::C_String result(assetPath);
	result = FormatWindowsFileString(result);

	S32 pos = ALIEN_BRAIN_DRIVE.Length() ? result.ISearch(ALIEN_BRAIN_DRIVE) : -1;
	if (pos != 0)
	{
		return result;
	}

	result = C_String("\\\\") + computerName + C_String("\\") + C_String(ALIEN_BRAIN_FOLDER) + result(pos + ALIEN_BRAIN_DRIVE.Length(), result.Length());
	return result;
}

#endif

}

#endif
