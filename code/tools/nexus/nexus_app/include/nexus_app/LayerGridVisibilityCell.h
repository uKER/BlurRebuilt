#ifndef LAYER_GRID_VISIBILITY_CELL_H
#define LAYER_GRID_VISIBILITY_CELL_H

class LayerGridVisibilityCell : public CExtGridCellColor
{
public:
	DECLARE_DYNCREATE(LayerGridVisibilityCell);
	IMPLEMENT_ExtGridCell_Clone(LayerGridVisibilityCell, CExtGridCellColor);
	CDemoColorPickerCell(
		CExtGridDataProvider * pDP = NULL
		);
	virtual void OnButtonPressed(
		CExtGridWnd & wndGrid,
		INT nButtonType,
		const RECT & rcCellExtra,
		const RECT & rcCell,
		LONG nVisibleColNo,
		LONG nVisibleRowNo,
		LONG nColNo,
		LONG nRowNo,
		INT nColType,
		INT nRowType
		);
	virtual void OnSetCheck(
		bool bCheck,
		CExtGridWnd & wndGrid,
		const RECT & rcCellExtra,
		const RECT & rcCell
		);
};

#endif
