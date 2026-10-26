#ifndef __LUX_DATATYPES_THECLUBCONTEXT_H__
#define __LUX_DATATYPES_THECLUBCONTEXT_H__

namespace LUX
{

namespace DataTypes
{

// context class keeps track of the amount of sun, sky and directional light that hit this chart
class TheClubContextData : public Void
{
public:
	TheClubContextData() :
	  m_fGamma(1.f),
	  m_fSun(0.f),
	  m_fSky(0.f),
	  m_fDirectional(0.f)
	{
	}

	int GetSize()
	{
		return 1;
	}

	void GetData(U8 *&p)
	{
		*p = (U8)((m_fSun?1:0) + (m_fSky?2:0) + (m_fDirectional?4:0));
		p++;
	}

	float m_fGamma, m_fSun, m_fSky, m_fDirectional;
};

}

}

#endif
