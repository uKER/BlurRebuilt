#ifndef __FX_RELOADABLEEFFECT_H__
#define __FX_RELOADABLEEFFECT_H__

namespace FX
{

/// effect with some extra parameters to make it reloadable using the effect manager
class ReloadableEffect : public Effect
{
public:
	ReloadableEffect *m_pNext;
	ReloadableEffect *m_pPrev;
	Util::C_String	m_sFilename;
friend class Manager;
protected:
	int	m_iRefCount;
	Time::C_Time	m_iTime;
};

}

#endif
