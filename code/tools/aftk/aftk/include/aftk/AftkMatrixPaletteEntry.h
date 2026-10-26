#ifndef AFTK_MATRIX_PALETTE_ENTRY_H
#define AFTK_MATRIX_PALETTE_ENTRY_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace Aftk
{
	struct C_MatrixPaletteEntry
	{
		Matrix44 matrix;
		Matrix44 matrixInverseTranspose;
	};

	typedef Ut::UtVector< Ut::DynamicVector<C_MatrixPaletteEntry> >	MatrixPaletteArray;
	typedef Ut::UtVector< Ut::DynamicVector<C_MatrixPaletteEntry*> >	MatrixPalettePtrArray;
}

#endif
