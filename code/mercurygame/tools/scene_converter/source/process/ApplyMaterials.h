#ifndef __APPLYMATERIALS_H__
#define __APPLYMATERIALS_H__

#include <btl/Containers/Vector.h>
#include <btl/Containers/Map.h>

/*
 * attribute table
 *
 * the attribute table is for mapping a material + atlas to a new material list
 */
typedef struct
{
	short		iMaterial;
	short		iAtlas;
} SAttribute;

class C_ApplyMaterials
{
public:
		/// constructor
		C_ApplyMaterials();

		/// destructor
		~C_ApplyMaterials();

	void					Initialize(Aftk::C_Scene *pScene, int nTex);

	void					SetMaterialToPolygon(Aftk::C_Polygon *pPoly, int iAtlas);

private:
	void					GetMaterialsFromAftkManager();
	const Aftk::C_MaterialResource	*GetAttribute(int iMaterial, int iAtlas, bool *pbNew);
	void					SetAttribute(int iMaterial, int iAtlas, const Aftk::C_MaterialResource *pRef);
	int						GetNumAttributes();
	void					Purge();
	const Aftk::C_MaterialResource *GetMaterialCombo(int iMat, int iAtlas);
	virtual const Aftk::C_MaterialResource *GenerateNewMaterial(Aftk::C_MaterialNode &newMaterial, int iAtlas ) = 0;
	void SetMaterialResourcePtr(Aftk::C_Polygon* pPoly, const Aftk::C_MaterialResource* pRef);

	const Aftk::C_MaterialResource	**m_piConversionTable;	// convert from material + chart no -> attribute no.
	Btl::Vector<SAttribute>	m_AttributeList;		// conert from attribute no -> material + chart no.
	SAttribute				m_Dummy;

	int						m_nTextures;
	int						m_nMaterials;
	Btl::Vector<Aftk::C_MaterialNode> m_MaterialList;
	Btl::Map<const Aftk::C_MaterialNode *, int> m_MtlMap;

	Aftk::C_Scene			*m_pAftkScene;
	int						m_nNewMaterials;
};

#endif
