#pragma once

class LMPolyAttributes : public Aftk::C_AttachData
{
public:
	LMPolyAttributes() : Aftk::C_AttachData()
	{
		bFaceCharted	= false;
		iChartLink		= -1;
		iChart			= -1;
		iAtlas			= -1;
		vNormal[0] = vNormal[1] = vNormal[2] = 0;
	}

	virtual ~LMPolyAttributes()
	{
	}

	virtual U32 GetType() const
	{
		return 1;
	}

	//member vars
	bool			bFaceCharted;
	int				iChartLink;
	int				iChart;
	int				iAtlas;
	LUX::Normal3	vNormal;
	short m_x, m_y, m_cx, m_cy;		// dimensions of the subchart

protected:
	void Copy(const LMPolyAttributes& rhs)
	{
		bFaceCharted		= rhs.bFaceCharted;
		iChartLink			= rhs.iChartLink;
		iChart				= rhs.iChart;
		iAtlas				= rhs.iAtlas;
		vNormal				= rhs.vNormal;
		m_x					= rhs.m_x;
		m_y					= rhs.m_y;
		m_cx				= rhs.m_cx;
		m_cy				= rhs.m_cy;
	}

	virtual Aftk::C_AttachData* CreateClone() const
	{
		LMPolyAttributes* pClone = new LMPolyAttributes();
		pClone->Copy(*this);
		return (Aftk::C_AttachData*)pClone;
	}
};

void AttachLMapData(Aftk::C_SceneMeshNode *m_pMesh);



