#ifndef __SYSTEM_LODMANAGER_H__
#define __SYSTEM_LODMANAGER_H__

#include <core/utility/singleton.h>
#include <btl/containers/set.h>
#include <Lod/LodInterface.h>

namespace Lod
{

typedef Btl::Set<I_LodInterface*>	LodInterfaces;

class C_LodManager : public Ut::Singleton<C_LodManager>
{
public:
	C_LodManager();
	~C_LodManager();

	void	Initialise(const F32 frameRate);
	void	SetAdapationSpeed(const F32 adaptSpeed) { m_AdaptSpeed = adaptSpeed; }

	void	Update(const F32 frameTime, const bool updateCpu = true, const bool updateGpu = true);
	void	Render() const;
	
	const F32	GetCpuDetail() const { return m_CpuDetail; }
	const F32	GetGpuDetail() const { return m_GpuDetail; }

	friend class I_LodInterface;

private:
	void	SetStaticDetail(const F32 cpuDetail, const F32 gpuDetail);
	void	SetDynamicDetail(const F32 cpuDetail, const F32 gpuDetail, const bool immediate);
	void	AddLodInterface(I_LodInterface* pLodInterface);
	void	RemoveLodInterface(I_LodInterface* pLodInterface);

	F32					m_TargetFrameTime;
	F32					m_AdaptSpeed;
	F32					m_CpuDetail;
	F32					m_GpuDetail;
	LodInterfaces		m_LodInterfaces;
};

}

#endif
