#ifndef LOGICALFS_PAKFS_PAK_COMPRESS_RAW_H
#define LOGICALFS_PAKFS_PAK_COMPRESS_RAW_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Pak
{

class C_PakCompressRaw : public I_PakCompress
{
public:
	C_PakCompressRaw(U32 flags);
	virtual ~C_PakCompressRaw();
	virtual bool Open(
		C_PakIoMediator* m_pPakRWOptimiser,
		Sys::C_StreamPos physicalStartPos, 
		const U32 fileAccessAttributes = Sys::E_FileAccess_Read,
		Sha1::C_HashContext* pHashContext = NULL);

	virtual bool Close();

#ifdef CFG_CONVERTER
	virtual bool Write(U8* buffer, U32 nrBytes);
#endif
	virtual bool Read(U8* buffer, U32 nrBytes);

	virtual bool Restart(U32 streamToLogicalPos);

	virtual U32 GetPhysicalLength() const;

private:
	U32 GetLogicalPos(Sys::C_StreamPos physicalPos) const;
	C_PakIoMediator*	m_pMediator;
	Sys::C_StreamPos	m_physicalStartPos;
	U32					m_fileAccessAttributes;
	//Physical size of compressed data.
	U32					m_dataPhysicalLength;
	Sys::C_StreamPos	m_physicalReadPos;
};

}

#endif
