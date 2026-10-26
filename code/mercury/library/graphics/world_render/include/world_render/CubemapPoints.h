#ifndef __CUBEMAPPOINTS_H__
#define __CUBEMAPPOINTS_H__

#include <btl/Containers/vector.h>
#include <misc_shared/KdTree/KdTree.h>

namespace RenderingData
{
namespace Target
{
struct SharedTextureData;
}
}

namespace WorldRender
{

struct CubemapZone
{
	Maths::HMat43 mtx;
	int iLocation;

	bool Intersects(const Maths::HVec3 PCREF vPos) const
	{
		Maths::HVec3 v = vPos * mtx;
		return (v.X() > -1.f && v.X() < 1.f &&
				v.Y() > -1.f && v.Y() < 1.f &&
				v.Z() > -1.f && v.Z() < 1.f);			
	}

	operator const Maths::C_AxisBox () const
	{
		Maths::C_AxisBox box;
		Maths::HMat43 _mtx = mtx.InverseRotTrans();
		for (int i=0; i<8; i++)
			box.UpdateExtents(Maths::HVec3((i&1)?-1.f:1.f,(i&2)?-1.f:1.f,(i&4)?-1.f:1.f) * _mtx);
		return box;
	}

	template<typename T>
	void Deserialise(T& ar)
	{
		ar >> mtx >> iLocation;
	}

};

struct CubemapLocation
{
	Maths::HVec3 vPos;
	Util::C_String sName;
	RenderingData::Target::SharedTextureData *pTexture;

	template<typename T>
	void Deserialise(T& ar)
	{
		ar >> vPos >> sName;
	}
};

// C_LocalisedCubemapData
class C_LocalisedCubemapData
{
public:
	/// constructor
	C_LocalisedCubemapData();

	/// destructor
	~C_LocalisedCubemapData();

	/// returns a platform texture pointer to the cubemap texture for a given world position in ppOutput
	/// returns true if a localised cubemap was found, or false if the default was given
	/// if the user intends to use the texture they need to increase it's refcount
	bool GetCubemap(const Maths::HVec3 PCREF v, RenderingData::Target::SharedTextureData **ppTex) const;

	/// show all localised cubemap zones 
	void DebugRender( DbUtil::I_DebugRender* pDebugRender, const Maths::HVec3 PCREF vCamera );

	/// Get GPU texture pointer from Shared Texture, and increase the refcount
	static const void *GetGPUTexture(RenderingData::Target::SharedTextureData *pTexture);

	/// release GPU texture
	static void ReleaseGPUTexture(RenderingData::Target::SharedTextureData *pTexture);

	S32 GetLocationCount() const;

	const CubemapLocation* GetLocation(S32 index) const;

	template<typename T>
	void Deserialise(T& ar)
	{
		ar >> m_Locations >> m_Zones >> m_sLocalisedCubemapPath >> m_sDefaultCubemapFilename;
		ResolveTextureReferences();
	}

private:
	/// return a shared texture pointer from the shared texture manager, given a filename
	RenderingData::Target::SharedTextureData *GetSharedTexture(Util::C_String sFilename);

	/// find all cubemap textures in the shared texture manager
	void ResolveTextureReferences();

	Btl::Vector<CubemapLocation> m_Locations;
	Util::KdTree<CubemapZone> m_Zones;
	RenderingData::Target::SharedTextureData *m_pDefaultTexture;
	Util::C_String m_sLocalisedCubemapPath, m_sDefaultCubemapFilename;
	bool m_bFoundTextures;
};

} // namespace WorldRender

#endif // __CUBEMAPPOINTS_H__

