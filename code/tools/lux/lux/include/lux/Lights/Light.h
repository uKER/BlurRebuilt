#ifndef LUX_LIGHT_H
#define LUX_LIGHT_H

#include "Lux\Common.h"
#include "Lux\LightSample.h"

namespace XERCES_CPP_NAMESPACE
{
	class DOMNode;
}

class Sampler;

namespace LUX
{
	class GlobalIlluminationProperties;

	class Light : public BaseObject
	{
		DECLARE_PERSISTENCE(Light);

		void write( ost::Engine &ar );
		void read ( ost::Engine &ar );

	public:
		///
		///Default constructor.
		Light();
		///Construct a light from an Aftk scene light node
		Light(const Aftk::C_SceneNode *pAftkLight, Aftk::Matrix44 &mtx);

		///
		///Destructor.
		virtual ~Light();

		void SetName(const std::string& name);

		void SetPosition(const float3& position);

		const std::string& GetName() const;

		LuxLightTypeEnum GetType() const;

		// perform any preprocessing (like generating sample points)
		virtual void PreProcess(Sampler *pSampler) {};

		// -- functions for quering the light for rendering purposes
		// returns whether a light is described by delta distribution (e.g. pointlights, directional as oppose to area/hemisphere)
		virtual bool IsDeltaLight() const { assert(0); return true; };
		// query whether this light is local (i.e. spot) or global (i.e. ambient, directional)
		virtual bool IsGlobalLight() const { return false; }

		///Sample incoming radiance to a point p. iSample indicates the sample number
		///returns: wi is the incoming light direction, pdf is the probability function, visibility ray is in 'visibility'
		virtual Spectrum Sample(const float3 &vPos, const float3 &vNormal, int iSample, float3 *wi, float *pdf, Visibility *visibility) const;

		// as above, but without supplying a surface normal
		virtual Spectrum Sample(const float3 &vPos, int iSample, float3 *wi, float *pdf, Visibility *visibility) const;

		///Returns the relative probability density (PDF) of light hitting a particular point, from a particular direction
		virtual float Pdf(const float3 &vPos, const float3 &vwi) { assert(0); return 0; }

		///Sample a random ray -leaving- the light source from a 4d sampling position u1,u2,u3,u4.
		///returns: ray and probability in pdf
		virtual Spectrum Sample(const float4 &vSample, SD::Ray *pRay, float *pdf) const { assert(0); return float3(0,0,0); };

		///Returns total emissive power of the light
		virtual Spectrum Power() { assert(0); return 0; }

		///Returns bounding box of area that this light affects
		virtual box3f GetBBox(const LUX::SpatialHierarchy *pScene) const;

		///Returns relative number of light paths
		virtual float GetNumLightPaths() const { return 0; }

		///Returns true if lambert shading needs to be applied in the sampler
		virtual bool GetLambert() const { return true; }

		// Set the spatial hierarchy
		virtual void SetSpatialHierarchy(const LUX::SpatialHierarchy *pScene, float fMeterSize) {};

		// Set the light cutoff point - any light below this intensity can be discarded
		virtual void SetCutoffThreshold(float f) { m_fCutoffThreshold = f; }

		// Set the gamma value for the scene. lights are inversely transformed by the gamma to counteract the gamma transform of the output
		virtual void SetGIProperties(GlobalIlluminationProperties *pGIProp) {};

		//
		virtual bool UseDiscontinuitySampling() const { return false; }

		virtual void SetXMLParameter(char *pName, char *pValue);

		void		ApplyGammaSettingsToIntensity(GlobalIlluminationProperties *pGIProp);

		// updating intensity / colour
		void				SetIntensity(float f);
		void				SetColour(float3 v);
		inline const float3		&GetColour(void) const { return m_vColour; }
		inline const float3		&GetEmittance() const { return m_vEmittance; }

		void LoadAftkLightSettings(const Aftk::C_SceneNode *pAftkLight);

		void Initialize();

	public:
		std::string			m_name;

		LuxLightTypeEnum	m_type;

		float3				m_vPos;
		int					m_iChannel;

		int					m_nSamples;
		float				m_fCutoffThreshold;
		std::string			m_channel;

	private:

		float3				m_vEmittance;	// m_vColour and m_fIntensity are multiplied together and stored here
		float3				m_vColour;
		float				m_fIntensity;

		GlobalIlluminationProperties *m_pGIProp;

		void				RecalculateEmittance();

	friend class LightingModelProperties;
	};
}

#define MAYA_LIGHT_GLOBAL_SCALE 0.1

#endif

