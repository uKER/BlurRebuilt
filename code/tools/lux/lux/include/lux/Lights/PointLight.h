
#ifndef LUX_POINT_LIGHT_H
#define LUX_POINT_LIGHT_H

namespace XERCES_CPP_NAMESPACE
{
	class DOMNode;
}

namespace LUX
{
	class PointLight : public Light
	{
	public:
		DECLARE_PERSISTENCE(PointLight);

		void write(ost::Engine &ar);
		void read(ost::Engine &ar);

		///
		///Default constructor.
		PointLight();
		///
		///Destructor.
		virtual ~PointLight();
		///
		///Construct using an AftkLight
		PointLight(const Aftk::C_ScenePointLightNode *pAftkLight, Aftk::Matrix44 &mtx);

		///Returns relative number of light paths
		virtual float GetNumLightPaths() const { return 1; }

		///Light sampling functions
		bool IsDeltaLight() const { return true; }

		void		SetGIProperties(GlobalIlluminationProperties *pGIProp);

		Spectrum	Sample(const float3 &vPos, int iSample, float3 *wi, float *pdf, Visibility *visibility) const;
		Spectrum	Sample(const float4 &vSample, SD::Ray *pRay, float *pdf) const;

		float		Pdf(const float3 &vPos, const float3 &vwi);
		Spectrum	Power() { return GetEmittance() * 4.f * PI; }
		box3f		GetBBox(const LUX::SpatialHierarchy *pScene) const;

		void		PreProcess(Sampler *pSampler);

	protected:
		void		Initialize();
		Aftk::E_LightDecay	m_iDecayType;
		float				m_fMaxRadiusSq;
		float				m_fCutoffRadius;
	};
}

#endif
