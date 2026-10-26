#pragma once

#define PVS_TYPE_POSITION 1
#define PVS_TYPE_DIRECTION 2

struct PvsPoint
{
	int iType;
	float3 vPos;
	float3 vDir;

	friend ost::Engine &operator <<( ost::Engine &ar, const PvsPoint &t )
	{
		ar << t.iType;
		ar << t.vPos << t.vDir;
		return ar;
	}
	friend ost::Engine &operator >>( ost::Engine &ar, PvsPoint &t )
	{
		ar >> t.iType;
		ar >> t.vPos >> t.vDir;
		return ar;
	}
};

class SampleArea : public BaseObject
{
public:
	SampleArea() {};

	int				GetNumSamples()			{ return m_nSamples; }
	void			SetNumSamples(int n )	{ m_nSamples = n; }
	virtual bool	GetSample(int iSample, float3 &vPos, float3 &vDir) = 0;

	virtual void write( ost::Engine &ar ) { return BaseObject::write(ar); }
	virtual void read ( ost::Engine &ar ) { return BaseObject::read(ar);  }

	int		m_nSamples;
};

class SampleList : public SampleArea
{
public:
	DECLARE_PERSISTENCE(SampleList);

	bool			GetSample(int iSample, float3 &vPos, float3 &vDir);

	virtual void write( ost::Engine &ar ) { ar << m_vPoints; return SampleArea::write(ar); }
	virtual void read ( ost::Engine &ar )
	{
		ar >> m_vPoints;
		m_nSamples = m_vPoints.Size() * 5;
		return SampleArea::read(ar);
	}

	Ut::Vector<PvsPoint>	m_vPoints;
};
