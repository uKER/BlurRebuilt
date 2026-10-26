#pragma once

#include "Lux/ISamplePoints.h"

class JobScheduler;

namespace LUX
{

class SamplePoints
{
public:
	typedef enum Target
	{
        SphericalHarmonics
	};

	// sample point accumulation buffer

						SamplePoints();
	virtual				~SamplePoints() {};
	void				ReadXMLNode(XERCES_CPP_NAMESPACE::DOMNode *pNode);
	void				WriteVector(FILE *f, float *pVec, int c);

	void				AddJobs(JobScheduler *pScheduler);
	BaseObject				*GenerateJob(int iID);
	void				StoreJobResult(BaseObject *pInResult);
	bool				SaveSamplePoints(IIntegrator *pIntegrator);

	bool				SaveSamplePoints(float fBrightness, float fGamma, bool bPrescaleSH, bool bSaveExtraData);
	void				SetGamma(float fGamma);
	void				SetBrightness(float fBrightness);
	void				SetRenderMode(int iMode, bool bBouncePass, Ut::BitArray128 iLightingPassBits);

	Ut::Vector<float3>	m_SamplePositions;
	Ut::Vector<float>	m_SampleResults;
	Target				m_type;
	std::string			m_filename;

	int					m_nScalarsPerSample;
	float				m_fInvGamma;
	float				m_fBrightness;
	int					m_iRenderMode;
	bool				m_bBouncePass;
	Ut::BitArray128		m_iLightingPassBits;

	static const int	m_iBatchSize;
	bool				m_bSaveExtraData;
	bool				m_bBinaryOutput;
};

}
