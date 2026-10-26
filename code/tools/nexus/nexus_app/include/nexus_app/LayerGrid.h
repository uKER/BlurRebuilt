#ifndef NEXUS_LAYER_GRID_H
#define NEXUS_LAYER_GRID_H

namespace Nexus
{
	class LayerGrid : public CExtGridWnd
	{
	public:
		bool Create(CWnd* pParentWnd);

		void Destroy();

		void Update();

		void Clear();

	private:
		void InitTable(
			LPCTSTR* arrHdrNames,
			INT* arrHdrExtents,
			INT* arrHdrIconIndexes, 
			int nColCount);

		void OnLButtonDown(UINT nFlags, CPoint point);

		DECLARE_MESSAGE_MAP();
	};
}

#endif
