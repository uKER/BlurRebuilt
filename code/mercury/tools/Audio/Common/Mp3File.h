#ifndef MP3FILE_H
#define MP3FILE_H

namespace Sys
{
	class I_File;
}
namespace Audio
{

struct Mp3Format
{
	Mp3Format() :
		uBitrate(0),
		uSampleRate(0),
		uNumChannels(0),
		bHasCrcCheck(0) {}

	U32 uBitrate;
	U32 uSampleRate;
	U32 uNumChannels;
	bool bHasCrcCheck;
};

class C_Mp3File
{
public:
	C_Mp3File();
	~C_Mp3File();

	bool Open(const char* pchFileName);
	bool Read();
	bool Close();

	const Mp3Format* GetFormat() const;
	const U8* GetData() const;
	U32 DataSize() const;

private:
	bool ReadData();

	bool IsValidHeader(U32& uHeader) const;

	U32 GetFrameSync(U32 u32Hdr) const { return ((u32Hdr>>21) & 0x7ff); }
	U32 GetVersionIndex(U32 u32Hdr) const { return ((u32Hdr>>19) & 0x3); }
	U32 GetLayerIndex(U32 u32Hdr) const { return ((u32Hdr>>17) & 0x3); 	}
	U32 GetBitrateIndex(U32 u32Hdr) const { return ((u32Hdr>>12) & 0xf); }
	U32 GetFrequencyIndex(U32 u32Hdr) const	{ return ((u32Hdr>>10) & 0x3); }
	U32 GetEmphasisIndex(U32 u32Hdr) const { return (u32Hdr & 0x3); }
	U32 GetChannelModeIndex(U32 u32Hdr) { return ((u32Hdr>>6) & 0x3); }
	U32 GetProtectionBit(U32 u32Hdr) { return ((u32Hdr>>16) & 0x1); }

private:
	U8* m_pData;
	U32 m_uDataSize;
	Mp3Format* m_pFormat;
	Sys::I_File* m_pFile;
	U32 m_uDataStart;
};

}	// namespace Audio


#endif	// #ifndef MP3FILE_H
