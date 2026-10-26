#ifndef LUX_SPOT_LIGHT_H
#define LUX_SPOT_LIGHT_H

namespace XERCES_CPP_NAMESPACE
{
	class DOMNode;
}

namespace LUX
{
	class SpotLight : public Light
	{
	public:
		DECLARE_PERSISTENCE(SpotLight);

		void write( ost::Engine &ar );
		void read ( ost::Engine &ar );

		///
		///Default constructor.
		SpotLight();
		///
		///Construct pointlight from Aftk pointlight
		SpotLight(const Aftk::C_SceneSpotLightNode *pAftkLight, Aftk::Matrix44 &mtx);
		///
		///Destructor.
		virtual ~SpotLight();

		///Returns relative number of light paths
		virtual float GetNumLightPaths() const { return 1; }

		///Light sampling functions
		bool		IsDeltaLight() const { return true; }

		Spectrum	Sample(const float3 &vPos, int iSample, float3 *wi, float *pdf, Visibility *visibility) const;
		float		FallOff(const float3 &vDir) const;
		Spectrum	Sample(const float4 &vSample, SD::Ray *pRay, float *pdf) const;

		float		Pdf(const float3 &vPos, const float3 &vwi);
		Spectrum	Power();
		box3f		GetBBox(const LUX::SpatialHierarchy *pScene) const;

		void		SetGIProperties(GlobalIlluminationProperties *pGIProp);

	protected:
		void		Initialize();

		float				m_fCosTotalWidth;
		float				m_fCosFalloffStart;

		Aftk::E_LightDecay	m_iDecayType;
		HMat44				m_worldMatrix;
		HMat44				m_invWorldMatrix;
	};
}

#endif
