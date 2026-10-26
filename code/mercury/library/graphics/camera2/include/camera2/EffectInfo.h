
#ifndef CAMERA2_EFFECTINFO_H
#define CAMERA2_EFFECTINFO_H

namespace Cam2
{

class C_EffectInfo
{
public:
	C_EffectInfo();
	
	void SetInfo(F32 nearStart, F32 nearEnd, F32 farStart, F32 farEnd, F32 farScale);
	F32 NearStart() const { return m_NearStart; }
	F32 NearEnd() const { return m_NearEnd; }
	F32 FarStart() const { return m_FarStart; }
	F32 FarEnd() const { return m_FarEnd; }
	F32 FarScale() const { return m_FarScale; }
	
private:
	F32 m_NearStart, m_NearEnd;
	F32 m_FarStart, m_FarEnd, m_FarScale;
};

} // End of namespace

#endif
