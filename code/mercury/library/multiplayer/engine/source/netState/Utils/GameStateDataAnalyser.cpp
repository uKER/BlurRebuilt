#include "stdafx.h"
#include "gamestateDataAnalyser.h"
#include "netState/netEntity/entityHeader.h"
#include "netState/netEntity/entityState.h"
#include "netEnums/netEnums.h"
#include "netstate/factory/ifactory.h"
#include <math.h>

#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", off ) 
#endif

bool C_DataAnalyser::IncomingState(C_EntityState* pxEntityState, const char* strTypeName)
{
#define _DATA_ANALYSER
#ifdef _DATA_ANALYSER
	if(!pxEntityState)
		return false;
	
	const C_Header& xHeader = pxEntityState->Header();
	if(xHeader.m_uEntityId < 0 || xHeader.m_uEntityId >= m_uMaxEntities)
		return false;
	
	const C_Compression& xCompression	= pxEntityState->Compression();
	C_EntityData& xEntityData			= m_pxEntityData[xHeader.m_uEntityId];	
	C_TypeData&	xTypeData				= GetCurrentTypeData(xHeader.m_uTypeId);
	bool bFullState						= xCompression.IsFullState();
	
	// log entity data
	xEntityData.m_bActive						= true;
	xEntityData.m_uTypeId						= xHeader.m_uTypeId;
	xEntityData.m_xIncoming.m_uEntHead			+= sizeof(C_Header);
	xEntityData.m_xIncoming.m_uCompHead			+= xCompression.CompressionHeaderSize();
	xEntityData.m_xIncoming.m_uCompHeadNoRLE	+= xCompression.CompressionHeaderSizeNoRLE();
	xEntityData.m_xIncoming.m_uData				+= xCompression.DifferenceSize();
	xEntityData.m_xIncoming.m_uFullCount		+= bFullState ? 1:0;
	xEntityData.m_xIncoming.m_uCompCount		+= bFullState ? 0:1;

	// log type data
	xTypeData.m_xIncoming.m_uEntHead			+= sizeof(C_Header);
	xTypeData.m_xIncoming.m_uCompHead			+= xCompression.CompressionHeaderSize();
	xTypeData.m_xIncoming.m_uCompHeadNoRLE		+= xCompression.CompressionHeaderSizeNoRLE();
	xTypeData.m_xIncoming.m_uData				+= xCompression.DifferenceSize();
	xTypeData.m_xIncoming.m_uFullCount			+= bFullState ? 1:0;
	xTypeData.m_xIncoming.m_uCompCount			+= bFullState ? 0:1;


#endif

	return true;
}

bool C_DataAnalyser::OutgoingState(C_EntityState* pxEntityState, const char* strTypeName)
{
#ifdef _DATA_ANALYSER
	if(!pxEntityState)
		return false;

	const C_Header& xHeader				= pxEntityState->Header();
	const C_Compression& xCompression	= pxEntityState->Compression();
	C_EntityData& xEntityData			= m_pxEntityData[xHeader.m_uEntityId];	
	C_TypeData&	xTypeData				= GetCurrentTypeData(xHeader.m_uTypeId);
	bool bFullState						= xCompression.IsFullState();
	
	// log entity data
	xEntityData.m_bActive						= true;
	xEntityData.m_uTypeId						= xHeader.m_uTypeId;
	xEntityData.m_xOutgoing.m_uEntHead			+= sizeof(C_Header);
	xEntityData.m_xOutgoing.m_uCompHead			+= xCompression.CompressionHeaderSize();
	xEntityData.m_xOutgoing.m_uCompHeadNoRLE	+= xCompression.CompressionHeaderSizeNoRLE();
	xEntityData.m_xOutgoing.m_uData				+= xCompression.DifferenceSize();
	xEntityData.m_xOutgoing.m_uFullCount		+= bFullState ? 1:0;
	xEntityData.m_xOutgoing.m_uCompCount		+= bFullState ? 0:1;

	// log type data
	xTypeData.m_xOutgoing.m_uEntHead			+= sizeof(C_Header);
	xTypeData.m_xOutgoing.m_uCompHead			+= xCompression.CompressionHeaderSize();
	xTypeData.m_xOutgoing.m_uCompHeadNoRLE		+= xCompression.CompressionHeaderSizeNoRLE();
	xTypeData.m_xOutgoing.m_uData				+= xCompression.DifferenceSize();
	xTypeData.m_xOutgoing.m_uFullCount			+= bFullState ? 1:0;
	xTypeData.m_xOutgoing.m_uCompCount			+= bFullState ? 0:1;


#endif

	return true;
}

int C_DataAnalyser::DebugOutputEntity(int i, char *str, int uBufferSize)
{
	int numlines = 0;
	
	if(i < 0 || i >= m_uMaxEntities) 
		return numlines;
	I_Factory* pxFactory = I_Factory::GetInstance();
	if(!pxFactory)
		return numlines;
		
	C_EntityData& xEntityData	= m_pxEntityData[i];

	unsigned int uTotalBytes = xEntityData.m_xOutgoing.m_uEntHead + xEntityData.m_xOutgoing.m_uCompHead + xEntityData.m_xOutgoing.m_uData;
	float fHeader = xEntityData.m_xOutgoing.m_uEntHead / (float)uTotalBytes;
	float fCompression = xEntityData.m_xOutgoing.m_uCompHead / (float)uTotalBytes;
	float fCompressionNoRLE = xEntityData.m_xOutgoing.m_uCompHeadNoRLE / (float)uTotalBytes;
	float fData = xEntityData.m_xOutgoing.m_uData / (float)uTotalBytes;

	char strExtra[512];
	sprintf_s(	strExtra,
				sizeof(strExtra), 
				"\tEnt[%s]\tH/C/D[%d/%d(%d)/%d]\tFull/Comp[%d/%d]\n",
				pxFactory->NetEntityString(xEntityData.m_uTypeId),
				uTotalBytes ? (int)(fHeader*100) : 0,
				uTotalBytes ? (int)(fCompression*100) : 0,
				uTotalBytes ? (int)(fCompressionNoRLE*100) : 0,
				uTotalBytes ? (int)(fData*100) : 0,
				xEntityData.m_xOutgoing.m_uFullCount,
				xEntityData.m_xOutgoing.m_uCompCount);

	strcat_s(str, uBufferSize, strExtra);
	numlines++;

	return numlines;
}
 
int C_DataAnalyser::DebugOutputSummary(char *str, int uBufferSize)
{
	int numlines = 0;
	
	m_uTotalBytesSent = 0;
	unsigned int uTotalHeader(0);
	unsigned int uTotalCompression(0);
	unsigned int uTotalCompressionNoRLE(0);
	unsigned int uTotalData(0);

	for(int i = 0; i < m_uMaxEntities; ++i)
	{
		C_EntityData& xEntityData	= m_pxEntityData[i];

		m_uTotalBytesSent		+= xEntityData.m_xOutgoing.m_uEntHead + xEntityData.m_xOutgoing.m_uCompHead + xEntityData.m_xOutgoing.m_uData;
		uTotalHeader			+= xEntityData.m_xOutgoing.m_uEntHead;
		uTotalCompression		+= xEntityData.m_xOutgoing.m_uCompHead;
		uTotalCompressionNoRLE	+= xEntityData.m_xOutgoing.m_uCompHeadNoRLE;
		uTotalData				+= xEntityData.m_xOutgoing.m_uData;
	}

	float fHeader			= (m_uTotalBytesSent>0) ? (uTotalHeader/(float)m_uTotalBytesSent) : 0;
	float fCompression		= (m_uTotalBytesSent>0) ? (uTotalCompression/(float)m_uTotalBytesSent) : 0;
	float fCompressionNoRLE	= (m_uTotalBytesSent>0) ? (uTotalCompressionNoRLE/(float)m_uTotalBytesSent) : 0;
	float fData				= (m_uTotalBytesSent>0) ? (uTotalData/(float)m_uTotalBytesSent) : 0;

	sprintf_s(str, uBufferSize, "Total[%dkB] : Header[%d]\tCompression[%d(%d)]\tData[%d]", 
		(int)(m_uTotalBytesSent/1000.0f), (int)(fHeader*100.0f), (int)(fCompression*100.0f), (int)(fCompressionNoRLE*100.0f), (int)(fData*100.0f));

	numlines++;
	return numlines;
}

int C_DataAnalyser::DebugOutputTypeSummary(char *str, int uBufferSize)
{
	int numlines = 0;
	I_Factory* pxFactory = I_Factory::GetInstance();
	if(!pxFactory)
		return numlines;

	sprintf_s(str, uBufferSize, "\0");

	for(C_TypeList::Iterator xTypeDataIt = m_xGlobalTypeData.Begin(); xTypeDataIt != m_xGlobalTypeData.End(); ++xTypeDataIt)
	{
		const C_TypeData& xTypeData	= xTypeDataIt->GetValue();
		const int uType				= xTypeDataIt->GetKey();

		unsigned int uTotalHeader			= xTypeData.m_xOutgoing.m_uEntHead;
		unsigned int uTotalCompression		= xTypeData.m_xOutgoing.m_uCompHead;
		unsigned int uTotalCompressionNoRLE	= xTypeData.m_xOutgoing.m_uCompHeadNoRLE;
		unsigned int uTotalData				= xTypeData.m_xOutgoing.m_uData;
		unsigned int uTotalTypeBytes		= uTotalHeader + uTotalCompression + uTotalData;

		char strExtra[512];
		sprintf_s(
		strExtra, sizeof(strExtra), 
		"%s[%d]\n", 
		pxFactory->NetEntityString(uType), (int)((uTotalTypeBytes / (float)m_uTotalBytesSent)*100) );
		numlines++;

		strcat_s(str, uBufferSize, strExtra);
	}
	return numlines;
}

void C_DataAnalyser::DebugGui(DbUtil::I_DebugRenderPtr debugRenderPtr)
{
	const float pi = 3.14159265;
	const float rad = 50;
	const float step = 0.2;

	// calculate totals
	C_Stats xDataTotalOut;
	C_Stats xDataTotalIn;
	for(int i = 0; i < m_uMaxEntities; ++i)
	{
		C_EntityData& xEntityData = m_pxEntityData[i];

		xDataTotalOut = xDataTotalOut + xEntityData.m_xOutgoing;
		xDataTotalIn = xDataTotalIn + xEntityData.m_xIncoming;
	}
	unsigned int uTotalOutData = xDataTotalOut.m_uEntHead + xDataTotalOut.m_uCompHead + xDataTotalOut.m_uData;
	unsigned int uTotalInData = xDataTotalIn.m_uEntHead + xDataTotalIn.m_uCompHead + xDataTotalIn.m_uData;

	// draw data summary (header, compression, data)
	{
		// TODO
	}

	// draw type summary
	float fCurrentDataPoint = 0;
	static Maths::HVec2 xChartCenter(75,250);
	if(uTotalOutData)
	{
		// draw chart
		for(C_TypeList::Iterator xTypeDataIt = m_xGlobalTypeData.Begin(); xTypeDataIt != m_xGlobalTypeData.End(); ++xTypeDataIt)
		{
			const C_TypeData& xTypeData	= xTypeDataIt->GetValue();
			const int uType				= xTypeDataIt->GetKey();

			unsigned int uTotalHeader			= xTypeData.m_xOutgoing.m_uEntHead;
			unsigned int uTotalCompression		= xTypeData.m_xOutgoing.m_uCompHead;
			unsigned int uTotalData				= xTypeData.m_xOutgoing.m_uData;
			unsigned int uTotalTypeBytes		= uTotalHeader + uTotalCompression + uTotalData;

			const float fAmount = uTotalTypeBytes / (float)uTotalOutData;
			const float fRes = 0.0027;
			const int uSteps = (int)(fAmount / fRes);

			Maths::RGBA xColour = StringToColour(xTypeData.m_strTypeName);

			for(float i = 0; i < uSteps; ++i)
			{
				float fStepInc = (fAmount / (float)uSteps);
				float start = fCurrentDataPoint;
				float end = fCurrentDataPoint + fStepInc;

				start *= 2;
				end *= 2;

				Maths::HVec2 xChartPointA(cos(start * pi) * rad, sin(start * pi) * rad);
				Maths::HVec2 xChartPointB(cos(end * pi) * rad, sin(end * pi) * rad);

				debugRenderPtr->Draw2dTriangle(xChartCenter, xChartCenter + xChartPointA, xChartCenter + xChartPointB, xColour);
				fCurrentDataPoint += fStepInc;
			}
		}

		// draw key
		static int uHeight = 20;
		Maths::HVec2 xTextOffset(-rad, rad);
		Maths::RGBA xColour = Maths::RGBA(255,255,255);
		debugRenderPtr->Draw2dText(xChartCenter + xTextOffset, "Entity Type", xColour, uHeight);
		for(C_TypeList::Iterator xTypeDataIt = m_xGlobalTypeData.Begin(); xTypeDataIt != m_xGlobalTypeData.End(); ++xTypeDataIt)
		{
			const C_TypeData& xTypeData	= xTypeDataIt->GetValue();
			const int uType				= xTypeDataIt->GetKey();

			Maths::RGBA xColour = StringToColour(xTypeData.m_strTypeName);

			xTextOffset = xTextOffset + Maths::HVec2(0, uHeight);			
			debugRenderPtr->Draw2dText(xChartCenter + xTextOffset, xTypeData.m_strTypeName, xColour, uHeight);
		}

		// calculate histogram totals
		C_Stats xHistDataTotalOut;
		C_Stats xHistDataTotalIn;
		for(int i = 0; i < E_LOG_SIZE; ++i)
		{
			C_TypeList& xTypeList = m_xTypeDataLog[i];
			for(C_TypeList::Iterator xTypeDataIt = xTypeList.Begin(); xTypeDataIt != xTypeList.End(); ++xTypeDataIt)
			{
				const C_TypeData& xTypeData	= xTypeDataIt->GetValue();

				xHistDataTotalOut = xHistDataTotalOut + xTypeData.m_xOutgoing;
				xHistDataTotalIn = xHistDataTotalIn + xTypeData.m_xIncoming;
			}
		}
		unsigned int uHistTotalOutData = xHistDataTotalOut.m_uEntHead + xHistDataTotalOut.m_uCompHead + xHistDataTotalOut.m_uData;
		unsigned int uHistTotalInData = xHistDataTotalIn.m_uEntHead + xHistDataTotalIn.m_uCompHead + xHistDataTotalIn.m_uData;

		// TODO: draw chart of recent history

		// Draw histogram
		int i = (m_uLogIndex==0) ? (E_LOG_SIZE-1) : m_uLogIndex-1;
		static Maths::HVec2 xHistOffset = xChartCenter + Maths::HVec2(rad, 305);
		float fHeight = rad * 2.0f;
		for(int uCount = 0; uCount < E_LOG_SIZE; ++uCount)
		{
			C_TypeList& xTypeList = m_xTypeDataLog[i];

			float fCurrentHeight = 0;
			for(C_TypeList::Iterator xTypeDataIt = xTypeList.Begin(); xTypeDataIt != xTypeList.End(); ++xTypeDataIt)
			{
				const C_TypeData& xTypeData	= xTypeDataIt->GetValue();

				unsigned int uTotalHeader			= xTypeData.m_xOutgoing.m_uEntHead;
				unsigned int uTotalCompression		= xTypeData.m_xOutgoing.m_uCompHead;
				unsigned int uTotalData				= xTypeData.m_xOutgoing.m_uData;
				unsigned int uTotalTypeBytes		= uTotalHeader + uTotalCompression + uTotalData;

				const float fMaxPacketSize = 1024.0f;
				const float fScale = 1024.0f;
				const float fAmount = uTotalTypeBytes / fScale;
				const float fBarLevel = fMaxPacketSize / fScale;

				Maths::RGBA xColour = StringToColour(xTypeData.m_strTypeName);
				debugRenderPtr->Draw2dLine(xHistOffset + Maths::HVec2(uCount, fCurrentHeight), xHistOffset + Maths::HVec2(uCount, fCurrentHeight+(fAmount*-fHeight)), xColour);
				debugRenderPtr->Draw2dLine(xHistOffset + Maths::HVec2(0, fBarLevel*-fHeight), xHistOffset + Maths::HVec2(E_LOG_SIZE, fBarLevel*-fHeight), Maths::RGBA(255,255,255,255));

				fCurrentHeight += fAmount*-fHeight;
			}

			if(--i < 0)
				i = E_LOG_SIZE-1;
		}
	}
}

bool C_DataAnalyser::IsActive(int i)
{ 
	if(i >= 0 && i < m_uMaxEntities) 
		return m_pxEntityData[i].m_bActive; 

	return false; 
}

C_DataAnalyser::C_TypeData& C_DataAnalyser::GetGlobalTypeData(const int uTypeId)
{
	return m_xGlobalTypeData[uTypeId];
}

C_DataAnalyser::C_TypeData& C_DataAnalyser::GetCurrentTypeData(const int uTypeId)
{
	return m_xCurrentTypeData[uTypeId];
}

Maths::RGBA	C_DataAnalyser::StringToColour(const char* str)
{
    U64 hash = 5381;
    int c;
    while (c = *str++)
        hash = ((hash << 5) + hash) + c;

	Maths::RGBA rgba(hash);
	return Maths::RGBA(rgba.R(), rgba.G(), rgba.B());
}

void C_DataAnalyser::Update()
{
#ifdef _DATA_ANALYSER
	// increment global data
	for(C_TypeList::Iterator xTypeDataIt = m_xCurrentTypeData.Begin(); xTypeDataIt != m_xCurrentTypeData.End(); ++xTypeDataIt)
	{
		C_TypeData& xCurrentTypeData	= xTypeDataIt->GetValue();
		const int uType					= xTypeDataIt->GetKey();

		C_TypeData&	xTypeData = GetGlobalTypeData(uType);

		xTypeData = xTypeData + xCurrentTypeData;
	}

	// store data in log
	m_xTypeDataLog[m_uLogIndex++] = m_xCurrentTypeData;
	if(m_uLogIndex >= E_LOG_SIZE)
		m_uLogIndex = 0;

	// reset current data
	for(C_TypeList::Iterator xTypeDataIt = m_xCurrentTypeData.Begin(); xTypeDataIt != m_xCurrentTypeData.End(); ++xTypeDataIt)
	{
		C_TypeData& xCurrentTypeData	= xTypeDataIt->GetValue();
		xCurrentTypeData.Reset();
	}
#endif
}

#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", on) 
#endif