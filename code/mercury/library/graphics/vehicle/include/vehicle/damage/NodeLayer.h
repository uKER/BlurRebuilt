#ifndef VEHICLEDAMAGE_NODELAYER_H
#define VEHICLEDAMAGE_NODELAYER_H
// Refactored from DamageLayer header.
#include "Vehicle/Damage/Node.h"

namespace Vehicle
{

//! A single layer of nodes.
// NOTE: Please be careful modifying this, you will need to change the SPUDamageJob to match.
class C_NodeLayer
{
public:
	C_NodeLayer()
	{
		m_Width					= 0;
		m_Length				= 0;
	}

	//Replace width and length members with start and end offsets, then work
	//out width/length. This way can have varying size layers

	//! Num zones wide
	int GetWidth() { return m_Width; }

	//! Num zones long
	int GetLength() { return m_Length; }

	//! Num nodes
	int Size() { return 0x40; }

	//! Return node pointer
	C_Node* GetNode(int p_RowIndex, int p_ColumnIndex)
	{
		int index = p_RowIndex * m_Width + p_ColumnIndex;

		BIZ_ASSERT(index >= 0 && index < Size());

		return &m_Nodes[index];
	}
	
public:

	int												m_Width;		//!< Number of zones wide
	int												m_Length;		//!< Number of zones long
	// Flat array for DMA performance.
	C_Node											m_Nodes[0x40];	//!< The nodes that reside within this layer.
};

}

#endif // #ifndef VEHICLEDAMAGE_NODELAYER_H