#ifndef LUX_DIRECTIONAL_LIGHT_H
#define LUX_DIRECTIONAL_LIGHT_H

namespace XERCES_CPP_NAMESPACE
{
	class DOMNode;
}

namespace LUX
{
	class DirectionalLight : public Light
	{
		DECLARE_PERSISTENCE(DirectionalLight);

		void write(ost::Engine &ar);
		void read(ost::Engine &ar);

	public:
		///
		///Default constructor.
		DirectionalLight();
		///
		///Construct using aftk light node
		DirectionalLight::DirectionalLight(Aftk::C_SceneDirectionalLightNode *pLight, Aftk::Matrix44 &mtx);
		///
		///Destructor.
		virtual ~DirectionalLight();

		void SetDirection(const Normal3& direction);

		const Normal3& GetDirection() const;

		void SetSpatialHierarchy(const LUX::SpatialHierarchy *pScene, float fMeterSize);

		float GetNumLightPaths() const;

		// light sampling functions
		bool IsDeltaLight() const { return true; }
		bool IsGlobalLight() const { return true; }
		void PreProcess(Sampler *pSampler);

		Spectrum	Sample(const float3 &vPos, int iSample, float3 *wi, float *pdf, Visibility *visibility) const;
		Spectrum	Sample(const float4 &vSample, SD::Ray *pRay, float *pdf) const;

		float		Pdf(const float3 &vPos, const float3 &vwi);
		Spectrum	Power();

		virtual void SetXMLParameter(char *pName, char *pValue);

		float3		GetDirection() { return -m_vDirection; }
		bool		GetLambert() const;

	protected:
		void Initialize();

		float3	m_vDirection;
		box3f	m_AreaBox;
		float	m_fDensity;
		float	m_fPenumbra;
		float3	*m_pSamples;
		bool	m_bLambert;
	};
}

#endif

