
#ifndef LIGHTING_MODEL
#define LIGHTING_MODEL

namespace Progress
{
class I_ProgressBar;
};

namespace LUX
{
	class Lux;

	// parent lighting model class that is inherited by all lighting model classes
	class LightingModel
	{
	public:
		LightingModel();
		LightingModel(const LightingModel& rhs);
		virtual ~LightingModel();
		
		virtual bool Light(	
			LUX::SpatialHierarchy* pHierachy, 
			const LightingModelProperties* pProperties,
			Lux* pLux,
			Aftk::C_Scene* pAftkScene,
			Progress::I_ProgressBar *pProgress) = 0;

		virtual LuxModelTypeEnum GetType() const = 0;

		const Lux *GetLux() const { return m_pLux; }

	#ifndef DOXYGEN_SHOULD_SKIP_THIS
	protected:
		void Clear();
		void Reset();

		const LUX::SpatialHierarchy*						m_pHierarchy;
		Aftk::C_Scene*										m_pAftkScene;
		const LightingModelProperties*					m_pProperties;
		Lux*											m_pLux;

		std::string										m_lightMapFilePrefix;
	#endif
	};
}

#endif
