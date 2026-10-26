#ifndef LUX_LIGHTING_MODEL_PROPORTIES
#define LUX_LIGHTING_MODEL_PROPORTIES

namespace XERCES_CPP_NAMESPACE
{
	class DOMNode;
}

#define OUTPUT_FORMAT_DONTCARE ((TextureFormat)-1)

#define TIMEOFDAY_ALL		0
#define TIMEOFDAY_SUNNY		1
#define TIMEOFDAY_NIGHT		2
#define TIMEOFDAY_OVERCAST	3

#define OUTPUT_TYPE_RGB					0
#define OUTPUT_TYPE_GOTHAM_NIGHT		1
#define OUTPUT_TYPE_BASIS3V2			2
#define OUTPUT_TYPE_TRANSFER			3
#define OUTPUT_TYPE_DIRECTION			4

// obsolete types
#define OUTPUT_TYPE_SH			101
#define OUTPUT_TYPE_DIR			102
#define OUTPUT_TYPE_BASIS3		103
#define OUTPUT_TYPE_BOUNCE		104
#define OUTPUT_TYPE_FLEXIBLE	105

namespace LUX
{
	class DirectionalLight;
	class Light;
	class RenderChannel;
}
	class CTextureChart;

typedef Ut::Vector<LUX::RenderChannel*> TRenderChannelVector;
typedef Ut::Vector<CTextureChart*> TChartList;

namespace LUX
{
	typedef enum
	{
		INSTANCE_MODE_INSTANTIATE,
		INSTANCE_MODE_UV_OFFSET,
		INSTANCE_MODE_DELETE,
	} InstanceMode;

	class LightingModelProperties
	{
	public:
		
		///
		///Default constructor.
		LightingModelProperties();
		
		///
		///Copy constructor.
		LightingModelProperties(const LightingModelProperties& rhs);
		
		///
		///\briefDestructor.
		virtual ~LightingModelProperties();
		
		///
		///\brief Asignment operator.
		///@param rhs lighting model properties to copy.
		virtual bool						Write(const std::string& fileName) const = 0;

		virtual bool						Read(const std::string& fileName) = 0;

		bool								AddLightsFromScene(Aftk::C_Scene *pScene);

		void								AddAreaLightsFromScene(Aftk::C_Scene *pScene, TChartList *pChartList);

		///
		void								AddLight(Light* pLights);

		///
		void								AddRenderChannel(RenderChannel *pChannel);
		
		///
		void								SetDefaultBakeTarget(LuxModelBakeTargetEnum bakeTarget);

		void								SetDefaultLightMapSet(S32 set);

		void								SetDefaultLightmapSize(S32 size);

		///
		LuxModelBakeTargetEnum				GetDefaultBakeTarget() const;

		int									AddDefaultRenderChannel(const char *szName);
		
		///
		virtual LightingModelProperties*	GetClonePtr() const = 0;
		
		///
		virtual LuxModelTypeEnum			GetType() const = 0;
		
		///
		S32									GetNumLights() const;
		
		///
		const Light*						GetLightPtr(S32 instance) const;
		Light*								GetLightPtr(S32 instance);

		TRenderChannelVector				*GetRenderChannelsPtr();

		TextureFormat						GetOutputFormat() const;

		int									GetOutputType() const;

		int									GetMaxNumBounces() const;

		S32									GetDefaultLightMapSet() const;
		
		S32									GetDefaultLightmapSize() const;

		F32									GetTexelDensity() const;

		F32									GetEpsilon() const;

		F32									GetTexelDensityRatio(int i) const;

		U32									GetNumAntiAliasingTaps() const;

		float								GetBounceScaleFactor() const;

		float								GetAreaLightScaleFactor() const;

		int									GetRenderChannel(const char *pszName);

		bool								GetAntiLightLeaking() const;

		bool								GetSaveExtraSamplePointData() const;

		bool								GetTiled() const;

		bool								GetWhiteAlpha() const;

		int									GetDoubleSidedPolyflag() const { return m_iDoubleSidedPolyflag; }
		int									GetEmitLightPolyflag() const { return m_iEmitLightPolyflag; }
		int									GetDoNotLightPolyflag() const { return m_iDoNotLightPolyflag; }
		int									GetDoNotCastShadowsPolyflag() const { return m_iDoNotCastShadowsPolyflag; }
		int									GetLowResLightmapsPolyflag() const { return m_iLowResLightmapsPolyflag; }
		int									GetLowResLightmapsBits() const { return m_iLowResLightmapsBits; }
		bool								NeedTangentSpace() const;

		float								InputGamma() const { return m_fInputGamma; }
		float								OutputGamma() const { return m_fOutputGamma; }

		void								DeleteAllLights();
		///
		void								Clear();
		void								DeleteExistingDirectionalLights();

	#ifndef DOXYGEN_SHOULD_SKIP_THIS
	protected:
		void Reset();

		virtual void ReadXMLNode(XERCES_CPP_NAMESPACE::DOMNode *pNode);
		bool			CheckLightTimeOfDaySetting(Aftk::C_SceneNode *pNode);

		LuxModelBakeTargetEnum					m_defaultBakeTarget;

		void SetXMLParameter(char *pName, char *pValue);

		Ut::UtVector<Ut::DynamicVector<Light*> >			m_lights;
		TRenderChannelVector								m_Channels;
		int										m_nVertexOutputChannels;

		bool									m_bWhiteAlpha;

		S32										m_defaultLightmapSize;
		F32										m_fTexelDensity;
		F32										m_fLightmapDensity[5];
		U32										m_iAntiAliasing;
		float									m_fBounceScale;
		float									m_fEpsilon;
		float									m_fAreaLightScale;
		TextureFormat							m_iOutputFormat;
		int										m_iTimeOfDay;
		bool									m_bTiled;
		int										m_iMaxBounces;
		bool									m_bAntiLightLeaking;
		bool									m_bSaveExtraSamplePointData;

		// polyflag masks
		int										m_iDoubleSidedPolyflag;
		int										m_iEmitLightPolyflag;
		int										m_iDoNotLightPolyflag;
		int										m_iDoNotCastShadowsPolyflag;
		int										m_iLowResLightmapsPolyflag;
		int										m_iLowResLightmapsBits;

	public:
		float									m_fMaxClusterSize;
		float									m_fMaxClusterArea;
		float									m_fMeterSize;
		float									m_fInputGamma;
		float									m_fOutputGamma;
		float									m_fBrightness;
		bool									m_bPrescaleSH;
		bool									m_bInstancesDefaultToSHLighting;
		bool									m_bAlphaScaling;
		bool									m_bIgnoreDirectionalLightsFromMaya;
		int										m_nProgressiveBouncePasses;
		bool									m_bColouredBounceLighting;
		int										m_iOutputType;
		InstanceMode							m_iInstanceMode;


	#endif
};

}

#endif
