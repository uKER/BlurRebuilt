// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#ifndef BD_GRID_H
#define BD_GRID_H

#include <bdGraphics/bdMesh.h>

/// Grid class.
/// Class to represent a graphical grid object. Inherits from mesh and adds its own init method to allow a grid to be
/// created. The number of columns and rows is not the number of lines for each, it is the number of squares for each.
class bdGrid : public bdMesh
{
public:

	BD_DECLARE_NEW_AND_DELETE_OPERATORS

	/// Create the grid while constructing the object.
	/// \param numColumns[in]		The number of columns required in the grid.
	/// \param numRows[in]			The number of rows required in the grid.
	/// \param columnSpacing[in]	The width of each column in the grid.
	/// \param rowSpacing[in]		The height of each row in the grid.
	bdGrid(	bdUInt numColumns = 10,
			bdUInt numRows = 10,
			bdFloat32 columnSpacing = 1.0f,
			bdFloat32 rowSpacing = 1.0f);

	/// Grid creation method.
	/// Makes this object a grid of the specified number of rows and columns with the given amount of spacing between
	/// the rows and columns. The grid is created on the XY-plane.
	/// \param numColumns[in]		The number of columns required in the grid.
	/// \param numRows[in]			The number of rows required in the grid.
	/// \param columnSpacing[in]	The width of each column in the grid.
	/// \param rowSpacing[in]		The height of each row in the grid.
	/// \return
	/// - True, if the grid was successfully created
	/// - False, if the grid was not created
	bdBool init(bdUInt numColumns = 10,
				bdUInt numRows = 10,
				bdFloat32 columnSpacing = 1.0f,
				bdFloat32 rowSpacing = 1.0f);
};

#endif // BD_GRID_H
