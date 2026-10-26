#ifndef __CONVERTER_SHARED_CONVERT_EFFECT_H__
#define __CONVERTER_SHARED_CONVERT_EFFECT_H__

namespace RenderDataConversion
{

class Effect;

class EffectManager
{
public:
	/// constructor
	EffectManager(RenderingData::Build::Scene *pSceneExport, const Util::C_String &sFXBPath);
	/// destructor
	~EffectManager();

	/// load effect associated with an Aftk Material
	Effect *GetEffect(const Util::C_String fxName);

private:
	/// load effect
	Effect *CreateEffect(const Util::C_String &sName);

	// map Aftk Material ID to RenderingData Material ID
	Btl::Map<Util::C_String,Effect*> m_Map;

	RenderingData::Build::Scene *m_pSceneExport;
	Util::C_String m_sFXBPath;
};

}

#endif

