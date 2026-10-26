#ifndef LUX_BOUNCEGATHER_LIGHT2_H
#define LUX_BOUNCEGATHER_LIGHT2_H

namespace XERCES_CPP_NAMESPACE
{
	class DOMNode;
}

#define PROGBOUNCEPASS_SEED 0
#define PROGBOUNCEPASS_BOUNCE 1
#define PROGBOUNCEPASS_FINALGATHER 2

namespace LUX
{
	class BounceTexData;

	class BounceGatherLight : public Light
	{
	public:
		DECLARE_PERSISTENCE(BounceGatherLight);

		void write(ost::Engine &ar);
		void read(ost::Engine &ar);

		///
		///Default constructor.
		BounceGatherLight();
		///
		///Destructor.
		virtual ~BounceGatherLight();

		///Light sampling functions
		bool IsDeltaLight() const { return false; }
		bool IsGlobalLight() const { return true; }
		bool UseDiscontinuitySampling() const { return true; }

		void SetXMLParameter(char *pName, char *pValue);

		void SetSpatialHierarchy(const LUX::SpatialHierarchy *pScene, float fMeterSize);
		void SetBounceTextureData(BounceTexData *pBounceTextureData);
		void SetBouncePass(int iBouncePass);

		// dummy sample functions - these are just here so we can inherit from Light
		Spectrum	Sample(const float3 &vPos, const float3 &vNormal, int iSample, float3 *wi, float *pdf, Visibility *visibility) const;
		Spectrum	Sample(const float3 &vPos, int iSample, float3 *wi, float *pdf, Visibility *visibility) const;

		// actual sample functions - these return samples on multiple render channels simultaneously
		void		Sample(const float3 &vPos, const float3 &vNormal, float *pdf, int iSample, Ut::Vector<LightSample> &samples) const;
		void		Sample(const float3 &vPos, float *pdf, int iSample, Ut::Vector<LightSample> &samples) const;

		// 
		void		DoSample(const float3 &vPos, const float3 &vDir, Ut::Vector<LightSample> &samples) const;

		Spectrum	Sample(const float4 &vSample, SD::Ray *pRay, float *pdf) const;
		void		PreProcess(Sampler *pSampler);

		float		Pdf(const float3 &vPos, const float3 &vwi);
		Spectrum	Power() { return GetEmittance() * 4.f * PI; }
		float3		Le(const float3 &vDir) const;

	protected:
		Aftk::E_LightDecay	m_iDecayType;

		float3				*m_pSamples;
		float3				*m_pSamplesPoint;
		BounceTexData		*m_pBounceTexData;
		float				m_fBounceScale;
		int					m_iBouncePass;
		bool				m_bCalcLightTransfer;
		bool				m_bHemisphere;
		const LUX::SpatialHierarchy *m_pScene;
	public:
		int					m_iBounceChannel;
	};
}

#endif
