
#ifndef LUX_HEMISPHERE_LIGHT_H
#define LUX_HEMISPHERE_LIGHT_H

namespace XERCES_CPP_NAMESPACE
{
	class DOMNode;
}

class HDRLoaderResult;

namespace LUX
{
	class HemisphereLight : public Light
	{
	public:
		DECLARE_PERSISTENCE(HemisphereLight);

		void write(ost::Engine &ar);
		void read(ost::Engine &ar);

		///
		///Default constructor.
		HemisphereLight();
		HemisphereLight(const Aftk::C_SceneHemisphereLightNode *pAftkLight, Aftk::Matrix44 &mtx);
		///
		///Destructor.
		virtual ~HemisphereLight();
		///
		///Construct using an AftkLight - not implemented
//		HemisphereLight(const Aftk::C_SceneHemisphereLightNode *pAftkLight);

		///Light sampling functions
		bool IsDeltaLight() const { return false; }
		bool IsGlobalLight() const { return true; }
		bool UseDiscontinuitySampling() const { return true; }
		void Initialize();

		Spectrum	Sample(const float3 &vPos, const float3 &vNormal, int iSample, float3 *wi, float *pdf, Visibility *visibility) const;
		Spectrum	Sample(const float3 &vPos, int iSample, float3 *wi, float *pdf, Visibility *visibility) const;
		Spectrum	Sample(const float4 &vSample, SD::Ray *pRay, float *pdf) const;
		void		PreProcess(Sampler *pSampler);
		void		SetSpatialHierarchy(const LUX::SpatialHierarchy *pScene, float fMeterSize);

		float		Pdf(const float3 &vPos, const float3 &vwi);
		Spectrum	Power();
		float		GetNumLightPaths() const;
		float3		Le(const float3 &vDir) const;

		// true = hemisphere, false = full 360 degree sphere
		void		SetHemisphere(bool bOn);

		void		SetXMLParameter(char *pName, char *pValue);

	protected:
		Aftk::E_LightDecay	m_iDecayType;

		float3				*m_pSamples;
		float3				m_vWorldCentre;
		float				m_fWorldRadius;
		float				m_fDensity;
		bool				m_bHemisphere; // true = hemisphere, false = full 360 degree sphere
	};
}

#endif
