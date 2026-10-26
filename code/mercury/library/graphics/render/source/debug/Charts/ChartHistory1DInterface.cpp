
#include "stdafx.h"

#include "Render/Debug/Charts/ChartHistory1DInterface.h"

using namespace Render::Chart;

void I_ChartHistory1D::SetHistorySize(U32 nHistoryValues)
{
	m_nHistorySize = nHistoryValues;
	
	for(S32 i = 0; i < m_Series.Size(); ++i)
	{
		C_SeriesHistory* pSeries = (C_SeriesHistory*)m_Series[i];

		pSeries->fValues.Resize(nHistoryValues);

		for(S32 j = 0; j < pSeries->fValues.Size(); ++j)
			pSeries->fValues[j] = m_fMin;

		pSeries->iWrap = 0;
	}
}

U32 I_ChartHistory1D::AddSeries(const Util::C_String& name, const Maths::C_RGBA_U8& colour)
{
	return AddSeries(name, colour, 0);
}

U32 I_ChartHistory1D::AddSeries(const Util::C_String& name, const Maths::C_RGBA_U8& colour, U32 iWrap)
{
	U32 iIndex = m_Series.Size();

	C_SeriesHistory* pSeries = new C_SeriesHistory;

	pSeries->name = name;
	pSeries->colour = colour;
	pSeries->iWrap = iWrap;
	pSeries->fValues.Resize(m_nHistorySize);

	for(S32 i = 0; i < pSeries->fValues.Size(); ++i)
		pSeries->fValues[i] = m_fMin;

	m_Series.PushBack(pSeries);

	return iIndex;
}

void I_ChartHistory1D::RemoveSeries(S32 i)
{
	m_iLastUpdate = 0;
	I_Chart1D::RemoveSeries(i);
}

void I_ChartHistory1D::SetValue(S32 i, F32 fValue)
{
	C_SeriesHistory* pSeries = (C_SeriesHistory*)m_Series[i];
	
	m_iLastUpdate = i;

	pSeries->fValues[pSeries->iWrap] = fValue;
	++pSeries->iWrap;

	if(pSeries->iWrap == m_nHistorySize)
		pSeries->iWrap = 0;
}

void I_ChartHistory1D::Update()
{
	// synchronise will make sure the series always stay in sequence with eachother (eg profiler)
	// sometimes this isn't wanted though (multiple line charts etc)
	if(m_bSynchronise && m_Series.Size() != 0)
	{
		C_SeriesHistory* pLastUpdate = (C_SeriesHistory*)m_Series[m_iLastUpdate];
		U32 iWrapTo = pLastUpdate->iWrap;

		for(S32 i = 0; i < m_Series.Size(); ++i)
		{
			C_SeriesHistory* pSeries = (C_SeriesHistory*)m_Series[i];
				
			if(iWrapTo != pSeries->iWrap)
			{
				pSeries->fValues[pSeries->iWrap] = m_fMin;
				pSeries->iWrap = iWrapTo;
			}
		}
	}
}



