#ifndef LOD_INTERFACE_H
#define LOD_INTERFACE_H

namespace Lod
{

class I_LodInterface
{
public:
	I_LodInterface();
	~I_LodInterface();

	virtual void	SetStaticDetail(const F32 cpuDetail, const F32 gpuDetail) {}
	virtual void	SetDynamicDetail(const F32 cpuDetail, const F32 gpuDetail, const bool immediate) {}
};

} // Lod

#endif // LOD_INTERFACE_H
