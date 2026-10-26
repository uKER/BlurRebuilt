#include "stdafx.h"
#include "entityCompressor.h"
#include "netEnums/netEnums.h"
#include "netDebug/netDebug.h"
#include "netState/netState/StateData.h"
#include "netState/netState/StateDataBuffer.h"
#include "netStream/netStream.h"
#include "netStream/netRLEEncoder.h"
#include "netStream/netStreamSizeCalculator.h"

#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", off )
#endif

C_Compression::C_Compression(const int uStateSize)
	: m_uDataFlagSize((int(uStateSize)/8)+(uStateSize%8?1:0))
	, m_uStateSize(uStateSize)
	, m_pszDataFlags(new char[m_uDataFlagSize])
{;}

C_Compression::C_Compression(const C_Compression& xCopy)
	: m_uDataFlagSize(xCopy.m_uDataFlagSize)
	, m_uStateSize(xCopy.m_uStateSize)
	, m_pszDataFlags(new char[m_uDataFlagSize])
{
	memcpy(m_pszDataFlags, xCopy.m_pszDataFlags, m_uDataFlagSize);
}

C_Compression::~C_Compression()
{
	if(m_pszDataFlags)
		delete m_pszDataFlags;

	m_pszDataFlags = NULL;
}

void C_Compression::FlagDifferent(int i)
{ 
	int uByteOffset = i / 8;
	int uBitOffset = i % 8;
	assert((uByteOffset+(uBitOffset?1:0)) <= m_uDataFlagSize);
	m_pszDataFlags[uByteOffset] |= (1 << uBitOffset); 
}

bool C_Compression::IsDifferent(int i) const
{ 
	int uByteOffset = i / 8;
	int uBitOffset = i % 8;
	assert((uByteOffset+(uBitOffset?1:0)) <= m_uDataFlagSize);
	return (m_pszDataFlags[uByteOffset] & (1 << uBitOffset)) ? true : false; 
}

bool C_Compression::serialise(class netStream* stream)
{
	if(netDebug::dbg_enableRLEEncoder)
	{
		BitfieldRLE rle(m_pszDataFlags, m_uDataFlagSize * 8);
		rle.encode();
		rle.serialise(stream);

		if(netDebug::dbg_debugRLEEncoder)
			rle.debugOutput();	

		return stream->ok();
	}
	else
	{
		stream->writeType(m_uDataFlagSize);
		stream->writeBytes(m_pszDataFlags, m_uDataFlagSize);
		return stream->ok();
	}
}

bool C_Compression::deserialise(class netStream* stream)
{
	if(netDebug::dbg_enableRLEEncoder)
	{
		BitfieldRLE rle(m_pszDataFlags, m_uDataFlagSize * 8);
		rle.deserialise(stream);
		rle.decode();
		return stream->ok();
	}
	else
	{
		stream->readType(m_uDataFlagSize);
		stream->readBytes(m_pszDataFlags, m_uDataFlagSize);
		return stream->ok();
	}
}

bool C_Compression::Compress(C_StateDataBuffer& xStateBuffer, int uLastState)
{
	if(!netDebug::dbg_useDeltaCompression)
	{
		FullState();
		return true;
	}

	// find last ack'ed state
	C_StateData* pxLastAckedState = xStateBuffer.FindState(uLastState);
	if(!pxLastAckedState)
	{
		FullState();
		return true;	
	}

	// accumulate differences between range of buffers
	ClearDifference();
	int bStreamDifference = 0;
	for(int i = 0; i < xStateBuffer.NumStates(); ++i)
	{
		C_StateData* xState = xStateBuffer.GetStateData(i);
		C_Compression& xCompressData = xStateBuffer.GetCompression(i);

		if(xState->CurrentState() <= pxLastAckedState->CurrentState())
			continue;

		for(int i = 0; i < m_uDataFlagSize; ++i)
		{
			if(m_pszDataFlags[i] != xCompressData.m_pszDataFlags[i])
			{
				m_pszDataFlags[i] |= xCompressData.m_pszDataFlags[i];
				++bStreamDifference;
			}
		}
	}

	bool bSendState = bStreamDifference > 0;
	bSendState |= xStateBuffer.Latest()->CurrentState() > pxLastAckedState->CurrentState();
	return bSendState;
}

bool C_Compression::Compress(const C_StateData& xDest, const C_StateData& xRef)
{	
	if(!netDebug::dbg_useDeltaCompression)
	{
		FullState();
		return true;
	}

	assert(xDest.Size() == xRef.Size());

	int bStreamDifference = 0;
	ClearDifference();
	
	for(int i = 0; i < m_uStateSize; ++i)
	{
		if(xDest.Data(i) != xRef.Data(i))
		{
			FlagDifferent(i);
			++bStreamDifference;
		}
	}

	#define _DEBUG_COMPRESSION
	#ifdef _DEBUG_COMPRESSION
	for(int i = 0; i < m_uStateSize; ++i)
	{
		bool bFlaggedDifferent = IsDifferent(i);
		bool bDataDifferent = xRef.Data(i) != xDest.Data(i);

		if(bFlaggedDifferent != bDataDifferent)
		{
			// assert(!"compresssion problem! these should be the same");
			continue;
		}
	}
	#endif

	return bStreamDifference > 0;
}

bool C_Compression::UnCompress(C_StateData& xDest, const C_StateData& xRef)		
{
	assert(xDest.Size() == xRef.Size());

	if(xDest.CurrentState() > E_EntityState_Creating)
	{
		if(xRef.CurrentState() <= xDest.CurrentState())
		{
			LOGDEBUG(netDebug::g_Log, "<C_Compression::UnCompress> Skipping entity un-compress: inSQN %d destSQN %d", xRef.CurrentState(), xDest.CurrentState());
			return false;
		}
	}

	for(int i = 0; i < m_uStateSize; ++i)
	{
		#ifndef _DEBUG_FORCE_FULL_STATE
		if(!IsDifferent(i))
			continue;
		#endif

		xDest.Data(i, xRef.Data(i));
	}
	xDest.CurrentState(xRef.CurrentState());

	#ifdef _DEBUG_COMPRESSION
	for(int i = 0; i < m_uStateSize; ++i)
	{
		if(IsDifferent(i) && xDest.Data(i) != xRef.Data(i))
		{
			assert(!"compresssion problem! different data not copied!");
			continue;
		}
	}
	//assert(xRef == xDest);
	#endif

	return true; 
}

// debug/util
int C_Compression::DifferenceSize() const
{
	int uSize = 0;
	for(int i = 0; i < m_uStateSize; ++i)
	{
		if(IsDifferent(i))
			++uSize;
	}
	return uSize;
}

int C_Compression::CompressionHeaderSize() const
{ 
	if(netDebug::dbg_enableRLEEncoder)
	{
		BitfieldRLE rle(m_pszDataFlags, m_uDataFlagSize * 8);
		rle.encode();
		netStreamSizeCalculator_NEW stream;
		rle.serialise(&stream);
		return stream.getWriteByteCount();
	}
	else
	{
		return m_uDataFlagSize;
	}
}

int C_Compression::CompressionHeaderSizeNoRLE() const
{
	return m_uDataFlagSize;
}

#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", on )
#endif

