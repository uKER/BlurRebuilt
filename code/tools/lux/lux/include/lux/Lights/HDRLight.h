
#ifndef LUX_HDR_LIGHT_H
#define LUX_HDR_LIGHT_H

namespace XERCES_CPP_NAMESPACE
{
	class DOMNode;
}

class HDRImage;

namespace LUX
{
	struct HDRSample
	{
		float3	vDir;
		float3	vCol;
		float	pdf;
	};

	class HDRLight : public Light
	{
	public:
		DECLARE_PERSISTENCE(HDRLight);

		void write(ost::Engine &ar);
		void read(ost::Engine &ar);

		///
		///Default constructor.
		HDRLight();
		///
		///Destructor.
		virtual ~HDRLight();
		///
		///Construct using an AftkLight - not implemented
//		HDRLight(const AftkSceneHDRLightNode *pAftkLight);

		///Light sampling functions
		bool IsDeltaLight() const { return false; }
		bool IsGlobalLight() const { return true; }
		bool UseDiscontinuitySampling() const { return false; }

		Spectrum	Sample(const float3 &vPos, const float3 &vNormal, int iSample, float3 *wi, float *pdf, Visibility *visibility) const;
		Spectrum	Sample(const float3 &vPos, int iSample, float3 *wi, float *pdf, Visibility *visibility) const;
		Spectrum	Sample(const float4 &vSample, SD::Ray *pRay, float *pdf) const;
		void		PreProcess(Sampler *pSampler);

		float		Pdf(const float3 &vPos, const float3 &vwi);
		Spectrum	Power() { return GetEmittance() * 4.f * PI; }

		void		SetGIProperties(GlobalIlluminationProperties *pGI);

	protected:
		virtual void SetXMLParameter(char *pName, char *pValue);
		void		 ReadImage(char *pszFilename);

		Aftk::E_LightDecay	m_iDecayType;

		HDRImage			*m_pHDRMap;

		HDRSample			*m_pSamples;
		float				m_fYaw;
		float				m_fInputGamma;
	};

	// Utility Classes and Functions
	struct Distribution1D
	{
		// Distribution1D Methods
		Distribution1D(float *f, int n);
		~Distribution1D();
		float Sample(float u, float *pdf);

		// Distribution1D Data
		float *func, *cdf;
		float funcInt, invFuncInt, invCount;
		int count;
	};

}

#endif
