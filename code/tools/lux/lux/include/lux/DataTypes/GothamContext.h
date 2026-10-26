#ifndef __LUX_DATATYPES_GOTHAMCONTEXT_H__
#define __LUX_DATATYPES_GOTHAMCONTEXT_H__

namespace LUX
{

namespace DataTypes
{

// context class keeps track of multi-layers and the nearest animation actor
class GothamNightContext : public Void
{
public:
	GothamNightContext() :
		m_bDualLayer(false),
		m_fHighestSample(0),
		m_iSubChannel(0)
	{
	}

	int GetSize()
	{
		return 1;
	}

	void GetData(U8 *&p)
	{
		// make a decision which animation this lighting data belongs to
		if (!m_bDualLayer)
		{
			*p = -1;	// no animation
			return;
		}

		*p = (U8)m_iSubChannel;
		p++;
	}

	bool m_bDualLayer;
	int m_iSubChannel;
	float m_fHighestSample;
};

}

}

#endif
