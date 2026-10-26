#include "stdafx.h"
#include "Mp3File.h"
#include <system/serialise/I_File.h>

// based on docs here
// http://www.mpgedit.org/mpgedit/mpeg_format/MP3Format.html

namespace Audio
{

//-----------------------------------------------------------------------------
// Name: C_Mp3file()
//-----------------------------------------------------------------------------
C_Mp3File::C_Mp3File() : m_pData(0),
				m_uDataSize(0),
				m_uDataStart(0),
				m_pFormat(0),
				m_pFile(0)
{
}

//-----------------------------------------------------------------------------
// Name: ~C_Mp3file()
//-----------------------------------------------------------------------------
C_Mp3File::~C_Mp3File()
{
	SAFE_DELETE_ARY(m_pData);
	SAFE_DELETE_PTR(m_pFormat);
	SAFE_DELETE_PTR(m_pFile);
}

//-----------------------------------------------------------------------------
// Name: Open()
// Desc: Opens the file.
//-----------------------------------------------------------------------------
bool C_Mp3File::Open(const char* pchFileName)
{
	m_pFile = Sys::C_FileSystem::Instance().Open(pchFileName);

	if (!m_pFile)
		return false;

	//printf("opened %s\n", pchFileName);

	return true;
}

//-----------------------------------------------------------------------------
// Name: ReadFormat()
// Desc: Searches the file for the first header and reads it.
// TODO: read byte by byte instead of reading the whole file
//-----------------------------------------------------------------------------
bool C_Mp3File::Read()
{
	if (!m_pFile)
		return false;

	if (m_pFormat)
		return false;

	if (m_pFile->Length() < 8)
		return false;

	ReadData();

	U8* pCur = m_pData;
	U32 u32Hdr = 0;

	// search for the first 32 bit header.
	// can be byte aligned.
	do
    {
		u32Hdr = *((U32*)pCur);

		EndianSwitchDWords((DWORD*)&u32Hdr, 1);

		pCur++;
    }
	while(!IsValidHeader(u32Hdr) && (pCur != (m_pData + m_pFile->Length())));

	if (pCur == m_pData + m_pFile->Length())
		return false;

	/*if ((GetFrameSync(u32Hdr) & 0x7ff) == 0x7ff)
	{
		printf("%d (0x%x) 0x%x framesync %x, version %x, layer %x, bitrate %x, freq %x, emphasis %x, channels %x\n", 
			(U32)pos, (U32)pos,
			u32Hdr,
			(GetFrameSync(u32Hdr) & 0x7ff),
			(GetVersionIndex(u32Hdr) & 0x3),
			(GetLayerIndex(u32Hdr) & 0x3),
			(GetBitrateIndex(u32Hdr) & 0xf),
			(GetFrequencyIndex(u32Hdr) & 0x3),
			(GetEmphasisIndex(u32Hdr) & 0x3),
			(GetChannelModeIndex(u32Hdr) & 0x3));
	}*/
 
	m_pFormat = new Mp3Format;
	if (!m_pFormat)
		return false;

	// 00 - Stereo
	// 01 - Joint stereo (Stereo)
	// 10 - Dual channel (2 mono channels)
	// 11 - Single channel (Mono)

	if(GetChannelModeIndex(u32Hdr) == 0x3) // single channel
		m_pFormat->uNumChannels = 1;
	else
		m_pFormat->uNumChannels = 2;
	
	// TODO: get bitrate and check if VBR (need to read more of the file to see if VBR).

    U32 table[4][3] = 
	{    
		{32000, 16000,  8000}, // MPEG 2.5
		{    0,     0,     0}, // reserved
		{22050, 24000, 16000}, // MPEG 2
		{44100, 48000, 32000}  // MPEG 1
    };

    m_pFormat->uSampleRate = table[GetVersionIndex(u32Hdr)] [GetFrequencyIndex(u32Hdr)];

	// i am presuming the mp3 decoder needs to consume the header and all data after it.
	// so we say the mp3 data starts with this first header.
	// note: there can be a 16 bit CRC value after the header and before the data.
	// to see if this 16 bit value exists then call GetProtectionBit()
	
	m_uDataStart = 0;

	/*printf("     = datastart %d, %d channels, %d samplerate.\n", m_uDataStart, m_pFormat->uNumChannels, m_pFormat->uSampleRate);

	printf("     = 0x%xframesync %x, version %x, layer %x, bitrate %x, freq %x, emphasis %x, channels %x\n", 
		u32Hdr,
		GetFrameSync(u32Hdr) & 0x7ff,
		GetVersionIndex(u32Hdr) & 0x3,
		GetLayerIndex(u32Hdr) & 0x3,
		GetBitrateIndex(u32Hdr) & 0xf,
		GetFrequencyIndex(u32Hdr) & 0x3,
		GetEmphasisIndex(u32Hdr) & 0x3,
		GetChannelModeIndex(u32Hdr) & 0x3);*/

	return true;
}

//-----------------------------------------------------------------------------
// Name: ReadData()
// Desc: Reads the data into a buffer.
//		 ReadFormat must be called first.
//		 Currently reads the whole file.
//-----------------------------------------------------------------------------
bool C_Mp3File::ReadData()
{
	if (!m_pFile || m_pData)
		return false;

	m_pFile->SetPosition(0);

	if (m_pFile->GetPosition() != 0)
		return false;

	m_uDataSize = static_cast<U32>(m_pFile->Length());

	m_pData = new U8[m_uDataSize];

	if (!m_pData)
		return false;

	//printf("     = datalength %d\n", m_uDataSize);

	return m_pFile->Read(m_pData, m_uDataSize);
}

//-----------------------------------------------------------------------------
// Name: Close()
// Desc: Closes the file. Does not free the data.
//-----------------------------------------------------------------------------
bool C_Mp3File::Close()
{
	SAFE_DELETE_PTR(m_pFile);
	return true;
}

//-----------------------------------------------------------------------------
// Name: GetFormat()
// Desc: Opens the file.
//-----------------------------------------------------------------------------
const Mp3Format* C_Mp3File::GetFormat() const
{
	return m_pFormat;
}

//-----------------------------------------------------------------------------
// Name: GetData()
// Desc: Opens the file.
//-----------------------------------------------------------------------------
const U8* C_Mp3File::GetData() const
{
	return m_pData;
}

U32 C_Mp3File::DataSize() const
{
	return m_uDataSize;
}


bool C_Mp3File::IsValidHeader(U32& u32Hdr) const
{
	//U8* pByte = (U8*)&u32Hdr;
	//if ((pByte[0] == 0xFF) && ((pByte[1] & 0xE0) == 0xE0) && ((pByte[2] & 0xF0) != 0xF0) ) // first 11 bits should be 1
	//	return true;

	// if all 11 frame sync bits are set
	// if version bits are not set to 0x01 - ie reserved
	// if layer bits are not set to 0x00 - ie reserved
	// if bitrate index is not set to
    return (((GetFrameSync(u32Hdr) & 0x7ff) == 0x7ff) &&
            ((GetVersionIndex(u32Hdr) & 0x3) != 0x01) &&
            ((GetLayerIndex(u32Hdr) & 0x3) != 0x0) && 
            ((GetBitrateIndex(u32Hdr) & 0xf) != 0x0) &&
            ((GetBitrateIndex(u32Hdr) & 0xf) != 0xf) &&
            ((GetFrequencyIndex(u32Hdr) & 0x3) != 0x3) &&
            ((GetEmphasisIndex(u32Hdr) & 0x3) != 0x2));
}

};	// namespace Audio
