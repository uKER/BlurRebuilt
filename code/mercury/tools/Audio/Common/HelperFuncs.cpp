#include "stdafx.h"

//-----------------------------------------------------------------------------
// Name: GetWindowsErrorMessage()
// Desc: Gets the error string associated with an error code
//-----------------------------------------------------------------------------
Util::C_String GetWindowsErrorMessage(int errorCode)
{
	char* message;
	
	Util::C_String str = Util::C_String("Windows error message: ");

	if (FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			0, errorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&message, 0, 0
		) >= 0)
	{
		if (message==NULL)
			return Util::C_String(str + "Unknown");

		for(char* ptr = message; *ptr != '\0'; ptr++)
		{
			if (!isprint(*ptr))
				*ptr = ' ';
		}

		str += "\"";
		str += message;
		str += "\"";
		LocalFree(message);
	}
	else
	{
		str += Util::C_String().Printf("Error code %d", errorCode);
	}

	return str;
}

//-----------------------------------------------------------------------------
// Name: FormatTime()
// Desc: Given a time in secs gives you a formatted string
//-----------------------------------------------------------------------------
Util::C_String FormatTime(F32 fTimeInSecs)
{
	if (fTimeInSecs >= 60.f*60.f)
	{
		U32 uHours = (int)(fTimeInSecs/(60.f*60.f));
		U32 uMins = (int)(fmod(fTimeInSecs, (60.f*60.f))/60.f);
		return Util::C_String().Printf("%d hour%s %d min%s %d secs", uHours, uHours>1?"s":"", uMins, uMins>1?"s":"", (int)fmod(fTimeInSecs, 60.f));
	}

	if (fTimeInSecs >= 60.f)
	{
		U32 uMins = (int)(fTimeInSecs/60.f);
		return Util::C_String().Printf("%d min%s %d secs", uMins, uMins>1?"s":"", (int)fmod(fTimeInSecs, 60.f));
	}

	return Util::C_String().Printf("%.2f secs", fTimeInSecs);
}

//-----------------------------------------------------------------------------
// Name: GetRawFilename()
// Desc: Get the name of a file given the full path name.
//		 note: removes the extension too.
// TODO: replace this with the mercury equivalent in System\Misc
//-----------------------------------------------------------------------------
Util::C_String GetRawFilename(const char* pchPathAndFname)
{
	return Util::RemoveFileExtension(Util::ExtractFilename(Util::C_String(pchPathAndFname)));
	/*static char pchFname[_MAX_FNAME]={0};
	_splitpath(pchPathAndFname, NULL, NULL, pchFname, NULL);
	return pchFname;*/
}


// NOTE: the ones in samplerate.c are probalby better than these:

//-----------------------------------------------------------------------------
// Name: S16ToF32()
// Desc: Converts a block of sample data from S16 to F32.
//		 note: sSrcLength is in bytes
//-----------------------------------------------------------------------------
bool S16ToF32(S16* s16Src, S32 sSrcLength, F32** ppf32Dest, S32* psDestLength)
{
	*psDestLength = sSrcLength*2;
	*ppf32Dest = new F32[*psDestLength];

	if (ppf32Dest==NULL)
	{
		printf("S16ToF32() : Could not allocate memory for f32 wave data");
		return false;
	}

	F32* pfDest=*ppf32Dest;
	U8* pu8Src=(U8*)s16Src;

	//src_short_to_float_array((const short*)pu8Src, pfDest, sSrcLength/2);

	while (pu8Src < ((U8*)s16Src+sSrcLength))
	{
		*pfDest=(float)*((S16*)pu8Src);
		//*pfDest /= 32768.f;
		*pfDest /= (1.0 * 0x8000);
		pu8Src+=2;
		pfDest++;
	}

	return true;
}

//-----------------------------------------------------------------------------
// Name: F32ToS16()
// Desc: Converts a block of sample data from F32 to S16
//-----------------------------------------------------------------------------
bool F32ToS16(F32* pf32Src, S32 sSrcLength, S16** pps16Dst, S32* psDstLength)
{
	*psDstLength = sSrcLength/2;
	*pps16Dst = new S16[*psDstLength];

	if (*pps16Dst==NULL)
	{
		printf("F32ToS16(): Could not allocate memory for S16 wave data");
		return false;
	}

	S16* pS16Dest=*pps16Dst;
	F32* pf32Curr = pf32Src;

	while ((U8*)pf32Curr < ((U8*)pf32Src+sSrcLength))
	{
		*pS16Dest=(S16)((F32)*((F32*)pf32Curr) * 32768.f);
		pf32Curr++;
		pS16Dest++;
	}

	return true;
}
