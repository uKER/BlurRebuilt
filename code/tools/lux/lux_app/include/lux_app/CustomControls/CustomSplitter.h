#pragma once

#define LAYOUT_MODE_SINGLE	1
#define LAYOUT_MODE_QUAD	0

class CCustomSplitterWnd : public CSplitterWnd
{
public:
    DECLARE_DYNAMIC(CCustomSplitterWnd)

	CCustomSplitterWnd() 
	{
		m_cxSplitterGap = m_cxOldSplitterGap = 3;//m_cxSplitterGap;
		m_cySplitterGap = m_cyOldSplitterGap = 3;//m_cySplitterGap;
	}

	int m_cxOldSplitterGap, m_cyOldSplitterGap;

	void SetNoSplitterGap(bool bTrue)
	{
		if (bTrue)
		{
			m_cxSplitterGap = 1;
			m_cySplitterGap = 1;
		}
		else
		{
			m_cxSplitterGap = m_cxOldSplitterGap;
			m_cySplitterGap = m_cyOldSplitterGap;
		}
	}

	virtual void OnDrawSplitter(CDC* pDC, ESplitType nType, const CRect& rect)
	{
	};

	virtual void RecalcLayout( );

	int m_iLayoutMode;

    DECLARE_MESSAGE_MAP()
    afx_msg UINT    OnNcHitTest(CPoint point);
};
