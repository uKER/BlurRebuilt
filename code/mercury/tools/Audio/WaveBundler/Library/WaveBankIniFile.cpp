#include "stdafx.h"
#include "AppHeaders.h"

#include "system\DebugLog.h"
#include "samplerate.h"

namespace Audio
{

enum eAttribName
{
	eCommonAttribSampleRate=0,
	eCommonAttribBitDepth,		
	eCommonAttribQuality,		
	eCommonAttribWaveFormat,		
	eCommonAttribPitchShiftUp,	
	eCommonAttribPitchShiftDown,	
	eCommonAttribQuantizeMethod,

	eBankAttribName,				// this must be first of the bank's enums
	eBankAttribEndian,			
	eBankAttribStreaming,		

	eWaveAttribFilename,			// this must be first of the wave's enums
	eWaveAttribInternalName,		
	eWaveAttribDefaultLevel,		
	eWaveAttribLoop,				

	NumAttribNames,
};

const int BankAttribStart = eBankAttribName;				
const int WaveAttribStart = eWaveAttribFilename;

const int NumCommonAttribNames = BankAttribStart;
const int NumBankAttribNames = WaveAttribStart - BankAttribStart;
const int NumWaveAttribNames = NumAttribNames - WaveAttribStart;

Util::C_String strAttribNames[NumAttribNames] =
{
	"samplerate",			// common attributes - can be used for bank or wave
	"bitdepth",
	"quality",
	"format",
	"pitchshiftup",
	"pitchshiftdown",
	"quantizemethod",

	"name",					// bank attributes
	"endian",
	"streaming",

	"filename",				// wave attributes
	"internalname",
	"defaultlevel",
	"loop",
};

Util::C_String& AttribName(eAttribName index) { return strAttribNames[index]; }

Util::C_String& CommonAttribName(int index) { return strAttribNames[index]; }
Util::C_String& BankAttribName(int index) { return strAttribNames[BankAttribStart+index]; }
Util::C_String& WaveAttribName(int index) { return strAttribNames[WaveAttribStart+index]; }


// TODO : all attribute name/value strings
// format strings

Util::C_String strFormats[WaveBankIniFile::NUM_FORMATS] =
{
	"adpcm",
	"xma",
	"mp3",
	"pcm_wav_s16",
	"pcm_wav_s16f32",
	"pcm_wav_s16_to_xma",
	"pcm_wav_s16_to_xma2",
	"pcm_wav_s16_to_adpcm",
};

Util::C_String& GetFormatString(int index) { return strFormats[index]; }

//-----------------------------------------------------------------------------
// Name: LogFormatStrings
// Desc: 
//-----------------------------------------------------------------------------
void LogFormatStrings()
{
	for (int i=0; i<WaveBankIniFile::NUM_FORMATS; i++)
		Print(TXTCOLOUR_YELLOW, "'%s', ", GetFormatString(i).Chars());
}


//-----------------------------------------------------------------------------
// Name: VerifyCommonAttributeName
// Desc: 
//-----------------------------------------------------------------------------
bool VerifyCommonAttributeName(const Util::C_String& strName)
{
	Util::C_String striName = strName.Lower();

	for (int i=0; i<NumCommonAttribNames; i++)
		if (striName == CommonAttribName(i))
			return true;
	return false;
}

//-----------------------------------------------------------------------------
// Name: VerifyCommonAttributeName
// Desc: 
//-----------------------------------------------------------------------------
bool VerifyBankAttributeName(const Util::C_String& strName)
{
	Util::C_String striName = strName.Lower();

	for (int i=0; i<NumBankAttribNames; i++)
		if (striName == BankAttribName(i))
			return true;
	return false;
}

//-----------------------------------------------------------------------------
// Name: VerifyWaveAttributeName
// Desc: 
//-----------------------------------------------------------------------------
bool VerifyWaveAttributeName(const Util::C_String& strName)
{
	Util::C_String striName = strName.Lower();

	for (int i=0; i<NumWaveAttribNames; i++)
		if (striName == WaveAttribName(i))
			return true;
	return false;
}

//-----------------------------------------------------------------------------
// Name: LogCommonAttribNames
// Desc: 
//-----------------------------------------------------------------------------
void LogCommonAttribNames()
{
	for (int i=0; i<NumCommonAttribNames; i++)
		Print(TXTCOLOUR_YELLOW, "'%s', ", CommonAttribName(i).Chars());
}

//-----------------------------------------------------------------------------
// Name: LogBankAttribNames
// Desc: 
//-----------------------------------------------------------------------------
void LogBankAttribNames()
{
	for (int i=0; i<NumBankAttribNames; i++)
		Print(TXTCOLOUR_YELLOW, "'%s', ", BankAttribName(i).Chars());
}

//-----------------------------------------------------------------------------
// Name: LogBankAttribNames
// Desc: 
//-----------------------------------------------------------------------------
void LogWaveAttribNames()
{
	for (int i=0; i<NumWaveAttribNames; i++)
		Print(TXTCOLOUR_YELLOW, "'%s', ", WaveAttribName(i).Chars());
}

//-----------------------------------------------------------------------------
// Name: CheckAttribExists
// Desc: 
//-----------------------------------------------------------------------------
bool CheckAttribExists(const Sys::C_XmlAttributeList& attributes, Util::C_String& strName)
{
	if (attributes.GetAttributeByName(strName) == 0)
	{
		Print(TXTCOLOUR_RED, "\nAttribute '%s' missing\n", strName.Chars());
		return false;
	}
	return true;
}

//-----------------------------------------------------------------------------
// Name: VerifyBankAttributes
// Desc: 
//-----------------------------------------------------------------------------
bool VerifyBankAttributes(const Sys::C_XmlAttributeList& attributes) 
{
	// check essential attribs exists
	if (CheckAttribExists(attributes, AttribName(eBankAttribName))==false) return false;
	if (CheckAttribExists(attributes, AttribName(eBankAttribEndian))==false) return false;

	bool bVerifiedOk = true;

	// check for unknown attributes
	for (int i=0; i<attributes.Elements(); i++)
	{
		if (VerifyCommonAttributeName(attributes[i]->name) == false && VerifyBankAttributeName(attributes[i]->name) == false)
		{
			Print(TXTCOLOUR_RED, "%sUnknown attribute '%s' in 'wavebank' block\n", bVerifiedOk ? "\n" : "", attributes[i]->name.Chars());
			bVerifiedOk = false;
		}
	}

	if (!bVerifiedOk)
	{
		Print(TXTCOLOUR_YELLOW, "Valid attributes are: ");
		LogCommonAttribNames();
		LogBankAttribNames();
		Print(TXTCOLOUR_YELLOW, "\n");
	}
	return bVerifiedOk;
}

//-----------------------------------------------------------------------------
// Name: VerifyWaveAttributes
// Desc: 
//-----------------------------------------------------------------------------
bool VerifyWaveAttributes(const Sys::C_XmlAttributeList& attributes) 
{
	// check essential attribs exists
	if (CheckAttribExists(attributes, AttribName(eWaveAttribFilename))==false) return false;
	if (CheckAttribExists(attributes, AttribName(eCommonAttribWaveFormat))==false) return false;

	bool bVerifiedOk = true;

	for (int i=0; i<attributes.Elements(); i++)
	{
		if (VerifyCommonAttributeName(attributes[i]->name) == false && VerifyWaveAttributeName(attributes[i]->name) == false)
		{
			Print(TXTCOLOUR_RED, "%sUnknown attribute '%s' in 'wave' block\n", bVerifiedOk ? "\n" : "", attributes[i]->name.Chars());
			bVerifiedOk = false;
		}
	}

	if (!bVerifiedOk)
	{
		Print(TXTCOLOUR_YELLOW, "Valid attributes are: ");
		LogCommonAttribNames();
		LogWaveAttribNames();
		bVerifiedOk = false;
	}

	return bVerifiedOk;
}

//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
WaveBankIniFile::~WaveBankIniFile()
{
	for (int i=0; i<m_vpWaves.Size(); i++)
		delete m_vpWaves[i];
}

//-----------------------------------------------------------------------------
// Name: SetFilename
// Desc: 
//-----------------------------------------------------------------------------
void WaveBankIniFile::SetFilename(const Util::C_String& strFilename)
{ 
	m_strFilename = strFilename;
	m_strPath = Util::ExtractDirectory(m_strFilename);
} 

//-----------------------------------------------------------------------------
// Name: WaveBankIniFile::StartElement
// Desc: 
//-----------------------------------------------------------------------------
bool WaveBankIniFile::StartElement(const Util::C_String& _name, const Sys::C_XmlAttributeList& attributes)
{
	const Util::C_String name = _name.Lower();

	if (name == "wavebank")
	{
		// check all attributes names are ok first
		if (VerifyBankAttributes(attributes) == false)
		{
			PrintError("Parse error in 'wavebank' block\n");
			return false;
		}

		// TODO:
		// check for identical internal names

		const Sys::C_XmlAttribute* attrib = attributes.GetAttributeByName(AttribName(eBankAttribName));
		if (attrib)
		{
			if (attrib->value.Length() > 31)
			{
				PrintError("Wavebank name '%s' too long'. Must be 31 characters or less\n", attrib->value.Chars());
				return false;
			}
			m_strBankName = attrib->value;
		}

		attrib = attributes.GetAttributeByName(AttribName(eBankAttribEndian));
		if (attrib)
		{
			if (attrib->value == "little")
				m_iEndian = 0;
			else
			if (attrib->value == "big")
				m_iEndian = 1;
			else
			{
				PrintError("Unknown endian type in 'wavebank' block. Expected 'little' or 'big'.\n");
				return false;
			}
		}

		attrib = attributes.GetAttributeByName(AttribName(eBankAttribStreaming));
		if (attrib)
		{
			if (attrib->value == "yes" || attrib->value == "true" || attrib->value == "1")
				m_bStreaming = 1;
			else
			if (attrib->value == "no" || attrib->value == "false" || attrib->value == "0")
				m_bStreaming = 0;
			else
				PrintError("Unknown value for '%s' attribute in 'wavebank' block. Expected 'yes', 'true', '1' or 'no', 'false', or '0'.\n", AttribName(eBankAttribStreaming).Chars());
		}

		attrib = attributes.GetAttributeByName(AttribName(eCommonAttribSampleRate));
		if (attrib)
		{
			m_iDefaultTargetSampleRate = strtol(attrib->value, 0, 10);
		}
		
		attrib = attributes.GetAttributeByName(AttribName(eCommonAttribBitDepth));
		if (attrib)
		{
			m_iDefaultTargetBitDepth = strtol(attrib->value, 0 , 10);
		}

		attrib = attributes.GetAttributeByName(AttribName(eCommonAttribPitchShiftUp));
		if (attrib)
		{
			m_iDefaultPitchShiftUp = strtol(attrib->value, 0 , 10);
		}

		attrib = attributes.GetAttributeByName(AttribName(eCommonAttribPitchShiftDown));
		if (attrib)
		{
			m_iDefaultPitchShiftDown = strtol(attrib->value, 0 , 10);
		}

		attrib = attributes.GetAttributeByName(AttribName(eCommonAttribQuality));
		if (attrib)
		{
			m_iDefaultCompressionQuality = strtol(attrib->value, 0, 10);
		}

		attrib = attributes.GetAttributeByName(AttribName(eCommonAttribQuantizeMethod));
		if (attrib)
		{
			if (attrib->value == "insertdelete")
				m_eDefaultQuantizeMethod = E_QUANTIZE_INSERT_DELETE;
			else
			if (attrib->value == "resample")
				m_eDefaultQuantizeMethod = E_QUANTIZE_RESAMPLE;
			else
			{
				PrintError("Unknown quantize method in 'wavebank' block. Expected 'insertdelete' or 'resample'.\n");
				return false;
			}
		}

		attrib = attributes.GetAttributeByName(AttribName(eCommonAttribWaveFormat));
		if (attrib)
		{
			eFormat eFormat;
			if (GetFormatEnum(Util::C_String(attrib->value), eFormat))
			{
				m_eDefaultFormat = eFormat;
			}
			else
			{
				Print(TXTCOLOUR_RED, "\nUnknown format string '%s'\n", attrib->value.Chars());
				Print(TXTCOLOUR_YELLOW, "Accepted formats are: ");
				LogFormatStrings();
				PrintError("Failed");
				return false;
			}
		}
	}
	else
	if (name == "wave")
	{
		const Sys::C_XmlAttribute* attrib;

		// check all attributes are ok first
		if (VerifyWaveAttributes(attributes) == false)
		{
			attrib = attributes.GetAttributeByName(AttribName(eWaveAttribFilename));
			if (attrib)
				PrintError("Line %d: Parse error in 'wave' block for '%s'\n", attributes.LineNo(), attrib->value.Chars());
			else
				PrintError("Line %d: Parse error in 'wave' block\n", attributes.LineNo());
			return false;
		}

		Wave* pWave = new Wave(this);
		bool bEverythingWasOk = true;
		
		// filename
		attrib = attributes.GetAttributeByName(AttribName(eWaveAttribFilename));
		if (attrib)
		{
			pWave->SetFilename(attrib->value);
		}

		// internal name
		attrib = attributes.GetAttributeByName(AttribName(eWaveAttribInternalName));
		if (attrib)
		{
			pWave->SetInternalName(attrib->value);
		}
		else
		{
			Util::C_String str = Util::ExtractFilename(pWave->Filename());
			pWave->SetInternalName(Util::RemoveFileExtension(str));
		}

		// default level
		attrib = attributes.GetAttributeByName(AttribName(eWaveAttribDefaultLevel));
		if (attrib)
		{
			pWave->SetDefaultLevel((F32)strtod(attrib->value.Chars(), 0));
		}

		// format
		attrib = attributes.GetAttributeByName(AttribName(eCommonAttribWaveFormat));
		if (attrib)
		{
			eFormat eFormat;
			if (GetFormatEnum(Util::C_String(attrib->value), eFormat))
			{
				pWave->SetFormat(eFormat);
			}
			else
			{
				Print(TXTCOLOUR_RED, "\nUnknown format string '%s'\n", attrib->value.Chars());
				Print(TXTCOLOUR_YELLOW, "Accepted formats are: ");
				LogFormatStrings();
				bEverythingWasOk = false;
			}
		}

		// bit depth
		attrib = attributes.GetAttributeByName(AttribName(eCommonAttribBitDepth));
		if (attrib)
		{	
			pWave->SetTargetBitDepth(strtol(attrib->value, 0 , 10));
		}

		// quality
		if (pWave->Format() == E_FORMAT_PCM_WAV_S16_TO_XMA || pWave->Format() == E_FORMAT_PCM_WAV_S16_TO_XMA2)
		{
			attrib = attributes.GetAttributeByName(AttribName(eCommonAttribQuality));
			if (attrib)
			{
				pWave->SetCompressionQuality(strtol(attrib->value, 0 , 10));
			}
		}	

		// pitch shift parameters
		attrib = attributes.GetAttributeByName(AttribName(eCommonAttribPitchShiftUp));
		if (attrib)
		{
			pWave->SetPitchShiftUp(strtol(attrib->value, 0, 10));
		}

		attrib = attributes.GetAttributeByName(AttribName(eCommonAttribPitchShiftDown));
		if (attrib)
		{
			pWave->SetPitchShiftDown(strtol(attrib->value, 0, 10));
		}
		
		//if (pChildTag->GetAttributeValueInt("pitchShiftQuality", pWave->m_iPitchShiftQuality))	
		//{
		//	// TODO:
		//}

		// is it a loop?
		attrib = attributes.GetAttributeByName(AttribName(eWaveAttribLoop));
		if (attrib)
		{
			// loops must be set at the encode stage for adpcm so warn if they are trying to do it via the xml
			if (pWave->Format() == E_FORMAT_ADPCM)
				PrintWarning("\nADPCM loop for '%s' must be set at encode stage. You cannot specify it here. Sorry.\n", pWave->InternalName().Chars());
			
			if (attrib->value == "yes" || attrib->value == "true" || attrib->value ==  "1")
				pWave->SetLoop(E_LOOP_YES);
			else
			if (attrib->value == "no" || attrib->value == "false" || attrib->value ==  "0")
				pWave->SetLoop(E_LOOP_NO);
			else
			{
				Print(TXTCOLOUR_RED, "\nUnknown value '%s' for '%s'. ", attrib->value.Chars(), AttribName(eWaveAttribLoop).Chars());
				Print(TXTCOLOUR_RED, "Expected 'yes', 'true', '1', 'no', 'false', or '0'.\n");
				bEverythingWasOk = false;
			}
		}

		// sample rate
		attrib = attributes.GetAttributeByName(AttribName(eCommonAttribSampleRate));
		if (attrib)
		{
			pWave->SetTargetSampleRate(strtol(attrib->value, 0, 10));
		}

		// quantize method
		attrib = attributes.GetAttributeByName(AttribName(eCommonAttribQuantizeMethod));
		if (attrib)
		{
			if (attrib->value == "insertdelete")
				pWave->SetQuantizeMethod(E_QUANTIZE_INSERT_DELETE);
			else
			if (attrib->value == "resample")
				pWave->SetQuantizeMethod(E_QUANTIZE_RESAMPLE);
			else
			{
				PrintError("Unknown quantize method in 'wavebank' block. Expected 'insertdelete' or 'resample'.\n");
				bEverythingWasOk = false;
			}
		}

		if (pWave->VerifyValues()==false)
			bEverythingWasOk = false;

		// check for duplicate internal names
		if (LookupByName(pWave->InternalName()))
		{
			Print(TXTCOLOUR_RED, "\nDuplicate '%s' '%s'.\n", AttribName(eWaveAttribInternalName).Chars(), pWave->InternalName().Chars());
			bEverythingWasOk = false;
		}

		// add the wave to the list
		if (bEverythingWasOk)
		{
			m_vpWaves.PushBack(pWave);
		}
		else
		{
			if (pWave->Filename() == "")
				PrintError("Line %d: Parse error in 'wave' block\n", attributes.LineNo(), pWave->Filename().Chars());	
			else
				PrintError("Line %d: Parse error in 'wave' block '%s'\n", attributes.LineNo(), pWave->Filename().Chars());	

			delete pWave;
			return false;
		}
	}
	else
	{
		PrintError("Unknown tag '%s'\n", name.Chars());	
	}

	return true;
}

//-----------------------------------------------------------------------------
// Name: WaveBankIniFile::Wave::VerifyValues
// Desc: 
//-----------------------------------------------------------------------------
bool WaveBankIniFile::Wave::VerifyValues()
{
	bool bOk = true;

	if (DefaultLevel() != VALUE_NOT_SPECIFIED)
	{
		if (DefaultLevel() < 0.f || DefaultLevel() > 1.0f)
		{
			Print(TXTCOLOUR_RED, "%s'%s' must be in the range 0.0 to 1.0.\n", bOk ? "\n" : "", AttribName(eWaveAttribDefaultLevel).Chars());
			bOk = false;
		}
	}

	if (Format() == E_FORMAT_NONE)
	{
		Print(TXTCOLOUR_RED, "%s'%s' has not been set\n", bOk ? "\n" : "", AttribName(eCommonAttribWaveFormat).Chars());
		bOk = false;
	}

	if (Format() == E_FORMAT_PCM_WAV_S16_TO_XMA || Format() == E_FORMAT_PCM_WAV_S16_TO_XMA2)
	{
		if (CompressionQuality() != VALUE_NOT_SPECIFIED)
		{
			if (CompressionQuality() < 1 || CompressionQuality() > 100)
			{
				Print(TXTCOLOUR_RED, "%s'%s' must be between 1 (poor) and 100 (best)\n", bOk ? "\n" : "", AttribName(eCommonAttribQuality).Chars());
				bOk = false;
			}
		}
	}

	if (PitchShiftUp() != VALUE_NOT_SPECIFIED)
	{
		if (PitchShiftUp() < 0 || PitchShiftUp() > 2)
		{
			Print(TXTCOLOUR_RED, "%s'%s' must be between 0 (off) and 2 (2 octaves).\n", bOk ? "\n" : "", AttribName(eCommonAttribPitchShiftUp).Chars());
			bOk = false;
		}
	}

	if (PitchShiftDown() != VALUE_NOT_SPECIFIED)
	{
		if (PitchShiftDown() < 0 || PitchShiftDown() > 1)
		{
			Print(TXTCOLOUR_RED, "%s'%s' must be either 0 (off) or 1 (on).\n", bOk ? "\n" : "", AttribName(eCommonAttribPitchShiftDown).Chars());
			bOk = false;
		}
	}

	if (m_strInternalName.Length() > 31)
	{
		Print(TXTCOLOUR_RED, "%s'%s' '%s' too long. Must be less than 32 characters\n", bOk ? "\n" : "", AttribName(eWaveAttribInternalName).Chars(), m_strInternalName.Chars());
		bOk = false;
	}

	if (InternalName() == "")
	{
		Print(TXTCOLOUR_RED, "%s'%s' not specified\n", bOk ? "\n" : "", AttribName(eWaveAttribInternalName).Chars());
		bOk = false;
	}

	if (TargetSampleRate() != VALUE_NOT_SPECIFIED)
	{
		if (TargetSampleRate() < 2000 || TargetSampleRate() > 48000)
		{
			Print(TXTCOLOUR_RED, "%s'%s' has invalid sample rate of %d. Must be between 2000 and 48000\n", bOk ? "\n" : "", AttribName(eCommonAttribWaveFormat).Chars(), TargetSampleRate());
			bOk = false;
		}
	}

	return bOk;
}

//-----------------------------------------------------------------------------
// Name: WaveBankIniFile::EndElement
// Desc: 
//-----------------------------------------------------------------------------
bool WaveBankIniFile::EndElement(const Util::C_String& name)
{
	return true;
}

//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
bool WaveBankIniFile::Parse(const char* pchData, int numBytes)
{
	Sys::E_XmlResult eResult = Sys::XmlParse(*this, pchData, numBytes );

	return (eResult == Sys::E_XmlResult_Success) ? true : false;
}

//-----------------------------------------------------------------------------
// Name: Load
// Desc: open xml file and parse it
//-----------------------------------------------------------------------------
int WaveBankIniFile::Load()
{
	/*LogSubscriber *p1 = new LogSubscriber();
	Debug::GetRootLogger().AddSubscriber( p1 );*/

	Audio::Print(0, "Attempting to load and parse ini file '%s'...", Util::ExtractFilename(m_strFilename).Chars());

	if (!Sys::C_FileSystem::Instance().FileExists(m_strFilename.Chars()))
	{
		PrintError("File '%s' does not exist.", Util::ExtractFilename(m_strFilename).Chars());
		return false;
	}

	Sys::I_File* pFile = Sys::C_FileSystem::Instance().Open(m_strFilename.Chars());

	if (!pFile)
	{
		PrintWarning(Util::C_String(GetWindowsErrorMessage(GetLastError()) + Util::C_String(" [May not be relevant]\n")).Chars());
		PrintError("Could not open '%s'.", Util::ExtractFilename(m_strFilename).Chars());
		return false;
	}

	// read file into buffer
	U32 numBytes = (U32)pFile->Length();
	char *pBuffer = new char[numBytes];
	pFile->Read(pBuffer, numBytes);

	bool result = Parse(pBuffer, numBytes);

	delete [] pBuffer;
	delete pFile;

	if (result==true) 
		Audio::Print(TXTCOLOUR_GREEN, " ok\n");

	return result;
}

//-----------------------------------------------------------------------------
// Name: LookupByName
// Desc: 
//-----------------------------------------------------------------------------
WaveBankIniFile::Wave* WaveBankIniFile::LookupByName(const Util::C_String& strName)
{
	for (int w=0; w<m_vpWaves.Size(); w++)
	{
		if (m_vpWaves[w]->InternalName() == strName)
		{
			return m_vpWaves[w];
		}
	}
	return NULL;
}

//-----------------------------------------------------------------------------
// Name: GetFormatEnum
// Desc: 
//-----------------------------------------------------------------------------
bool WaveBankIniFile::GetFormatEnum(Util::C_String& strFormat, eFormat& format)
{
	Util::C_String striFormat = strFormat.Lower();

	for (int i=0; i<WaveBankIniFile::NUM_FORMATS; i++)
	{
		if (striFormat == GetFormatString(i))
		{
			format = (eFormat)i;
			return true;
		}
	}
	
	return false;
}

} // namespace Audio