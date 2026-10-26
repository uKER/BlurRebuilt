#pragma once

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

typedef Ut::UtVector< Ut::DynamicVector<SAttribute> > TAttributeVector;
typedef Ut::UtVector< Ut::DynamicVector<U32> > TMatIDList;
class CTextureChart;
typedef Ut::Vector<CTextureChart*> TChartList;

class CAttributeTable
{
public:
		CAttributeTable() : m_piConversionTable(0), m_nTextures(0), m_nMaterials(0) {};
		~CAttributeTable() {};

	const Aftk::C_MaterialResource	**m_piConversionTable;	// convert from material + chart no -> attribute no.
	TAttributeVector		m_AttributeList;		// conert from attribute no -> material + chart no.
	SAttribute				m_Dummy;

	int						m_nTextures;
	int						m_nMaterials;

	void					Initialize(int nMat, int nAtlas);
	const Aftk::C_MaterialResource	*GetAttribute(int iMaterial, int iAtlas, bool *pbNew);
	void					SetAttribute(int iMaterial, int iAtlas, const Aftk::C_MaterialResource *pRef);
	int						GetNumAttributes();
	void					Purge();
};
