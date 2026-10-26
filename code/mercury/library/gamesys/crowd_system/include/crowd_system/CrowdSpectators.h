#ifndef CROWD_SPECTATORS_H
#define CROWD_SPECTATORS_H

#ifdef _MSC_VER
#pragma once
#endif

#include <crowd_system/CrowdSpectator.h>

#include <btl/containers/bitvector.h>

namespace AnimatedCrowd
{
	class C_CrowdResources;
	class C_CrowdBlock;
	class C_CrowdTextureCache;
};

namespace AnimatedCrowd
{

//! C_CrowdSpectators

class C_CrowdSpectators
{
public:
	C_CrowdSpectators(C_CrowdResources* pResources);
	~C_CrowdSpectators();

	void				Reinitialise();
	
	void				CreateStaticSpectators(C_CrowdBlock& block, const C_CrowdTextureCache* pFlagTextureCache);
	void				DestroyStaticSpectators(C_CrowdBlock& block, const bool resetPaletteMapIndex = false, U8* pPaletteMap = NULL);

	C_CrowdSpectator*	GetSpectators() { return &m_Spectators[0]; }

	bool				Flush();

	const U32			GetNumSpectators() const;
	const U32			GetNumFreeSpectators() const;

private:
	const S32			CreateStaticSpectator(const SpectatorSpawnParams& spawnParams);
	void				DestroyStaticSpectator(const S32 spectatorIndex);

private:
	typedef Btl::BitVectorTemplate<Btl::BitArrayBase<Btl::Vector<U8, Btl::VectorParameters_Array<64> > > > BitArray512;

	C_CrowdResources*	m_pResources;
	
	C_CrowdSpectator	m_Spectators[CROWD_MAX_SPECTATORS];
	BitArray512			m_UsedSpectators;
	
	U32					m_NumUsedSpectators;

	U32					m_FlagWaverBehaviourHash;
};

}; // namespace AnimatedCrowd

#endif // CROWD_SPECTATORS_H
