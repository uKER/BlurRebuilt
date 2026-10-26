#ifndef __EFFECTPERMUTATIONSELECTOR_H__
#define __EFFECTPERMUTATIONSELECTOR_H__

namespace RenderDataConversion
{

class Effect;
class AftkMaterialConverter;
class AftkMeshConverter;
class MaterialManager;

/// convert a polygons of a single Aftk meshnode into to a renderdata primitive
class EffectPermutationSelector
{
friend class Effect;
friend class AnnotationReader;

public:
	/// constructor
	EffectPermutationSelector(MaterialManager *pMgr, const Aftk::C_SceneMeshNode *pMesh, const Aftk::C_MaterialNode *pMtl);
	/// destructor
	~EffectPermutationSelector();

	/// loads an effect and determines which permutation should be used for this particular mesh/material
	Util::C_String GetPermutation(const Util::C_String &sBaseEffect);

protected:
	/// get the aftk material converter
	AftkMaterialConverter *GetMtlConverter();

	/// get the aftk node converter
	AftkMeshConverter *GetMeshConverter();

	/// add a new permutation string, checks for duplicates
	void AddPermutation(const Util::C_String &s);

	///
	Btl::Vector<Util::C_String> m_Permutations;

	MaterialManager *m_pMgr;
	const Aftk::C_SceneMeshNode *m_pMesh;
	const Aftk::C_MaterialNode *m_pMtl;

	AftkMaterialConverter *m_pMtlConverter;
	AftkMeshConverter *m_pMeshConverter;
};

}

#endif
