
#ifndef LUX_HEMISPHERE_LIGHT2_H
#define LUX_HEMISPHERE_LIGHT2_H

namespace XERCES_CPP_NAMESPACE
{
	class DOMNode;
}

namespace LUX
{
	class HemisphereLight2 : public Light
	{
	public:
		DECLARE_PERSISTENCE(HemisphereLight2);

		void write(ost::Engine &ar);
		void read(ost::Engine &ar);

		///
		///Default constructor.
		HemisphereLight2();
		///
		///Destructor.
		virtual ~HemisphereLight2();
		///
		///Construct using an AftkLight - not implemented
//		HemisphereLight(const Aftk::C_SceneHemisphereLightNode *pAftkLight);

		///Light sampling functions
		bool IsDeltaLight() const { return false; }
		bool IsGlobalLight() const { return true; }
		bool UseDiscontinuitySampling() const { return true; }

		void SetXMLParameter(char *pName, char *pValue);

		void SetSpatialHierarchy(const LUX::SpatialHierarchy *pScene, float fMeterSize);

		Spectrum	Sample(const float3 &vPos, const float3 &vNormal, int iSample, float3 *wi, float *pdf, Visibility *visibility) const;
		Spectrum	Sample(const float3 &vPos, int iSample, float3 *wi, float *pdf, Visibility *visibility) const;
		Spectrum	Sample(const float4 &vSample, SD::Ray *pRay, float *pdf) const;
		void		PreProcess(Sampler *pSampler);
		void		SetBounceColour(float3 vColour);

		float		Pdf(const float3 &vPos, const float3 &vwi);
		Spectrum	Power() { return GetEmittance() * 4.f * PI; }
		float3		Le(const float3 &vDir) const;

		void		SetSunDirection(float3 vSunDir);

	protected:
		Aftk::E_LightDecay	m_iDecayType;

		float3				*m_pSamples;
		float3				m_vSunDirection;
		float				m_fSunIntensity;
		float3				m_vBounceColour;
		const SD::SpatialHierarchy *m_pScene;
	};
}

#endif
