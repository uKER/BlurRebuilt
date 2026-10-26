
#ifndef LUX_AREA_LIGHT_H
#define LUX_AREA_LIGHT_H

namespace XERCES_CPP_NAMESPACE
{
	class DOMNode;
}

namespace LUX
{
	class AreaLight : public Light
	{
	public:
		DECLARE_PERSISTENCE(AreaLight);

		void write(ost::Engine &ar);
		void read(ost::Engine &ar);

		class Triangle
		{
		public:
			float3	m_v[3];
			float3	m_vColour;
			float3	m_vCentre;
			float3	m_vNormal;
			float	m_fArea;

					Triangle() {};
					Triangle(const float3 &v1, const float3 &v2, const float3 &v3, const float3 &vColour);
			float	PatchFactor(const float3 &p, const float3 &np) const;
			float	EdgeArea(const float3 &p, const float3 &q, const float3 &n) const;

			friend ost::Engine& operator >> (ost::Engine& ar, Triangle &t)
			{
				ar >> t.m_v[0] >> t.m_v[1] >> t.m_v[2] >> t.m_vColour >> t.m_vCentre >> t.m_vNormal >> t.m_fArea;
				return ar;
			}
			friend ost::Engine& operator << (ost::Engine& ar, Triangle &t)
			{
				ar << t.m_v[0] << t.m_v[1] << t.m_v[2] << t.m_vColour << t.m_vCentre << t.m_vNormal << t.m_fArea;
				return ar;
			}
		};

		class XSample
		{
		public:
			float3	m_vPos;
			int		m_iTriangle;
		};

		///
		///Default constructor.
		AreaLight();
		AreaLight(Aftk::C_SceneAreaLightNode *pLight, Aftk::Matrix44 &mtx);
		///
		///Destructor.
		virtual ~AreaLight();

		///Returns relative number of light paths
		virtual float GetNumLightPaths() const { return 1; }

		///Light sampling functions
		bool IsDeltaLight() const { return false; }

		Spectrum	Sample(const float3 &vPos, const float3 &vNormal, int iSample, float3 *wi, float *pdf, Visibility *visibility) const;
		Spectrum	Sample(const float3 &vPos, int iSample, float3 *wi, float *pdf, Visibility *visibility) const;
		Spectrum	Sample(const float4 &vSample, SD::Ray *pRay, float *pdf) const;

		/// No lambert needs to be applied in the sampler, because it is implicit in the form-factor
		virtual bool GetLambert() const { return false; }

		float		Pdf(const float3 &vPos, const float3 &vwi);
		Spectrum	Power() { return GetEmittance(); }
		box3f		GetBBox(const LUX::SpatialHierarchy *pScene) const;

		void		PreProcess(Sampler *pSampler);

		void		AddTriangle(const float3 &v1, const float3 &v2, const float3 &v3, const float3 &vColour);

	protected:
		void		Initialize();
		Ut::Vector<Triangle>	m_Triangles;
		Ut::Vector<XSample>	m_Samples;
		float		m_fColourScale;
	};
}

#endif
