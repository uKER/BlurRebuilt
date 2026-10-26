#ifndef UI_UITABLE_H
#define UI_UITABLE_H

#include "ui\Game\Gadgets\Buttons.h"

namespace UI
{

class UINode;

/*---------------------------------------------------------------------------------------
	Class : UITable

	Desc : 
---------------------------------------------------------------------------------------*/
class UITable : public C_GadgetBase
{
public:
				DECLARE_GADGET( E_UIButtonType_Table, "TABLE" );

	void 			Init();
	void 			PostInit();
	void 			Setup( lua_State* pState );
	void			Render();
	void			FrameUpdate();
	bool			MessageHandler( const Messaging::C_MessageBase *pMessage );

	void			Select( const bool bSelect, const bool bSendTimelineMsg = true );

	void			AddRow( UINode* pNode, const F64 sValue );
	void			AddColumn();
	const UINode*	GetNodeByValue( const F64 fValue );

	void			ActivateElement( const bool bActivate );

	void			SetSelection( const F64 fValue );
	F64				GetSelection() const;

	bool			ElementActivated() const 
					{
						return m_ElementActive;
					}


private:
	void			MoveDown();
	void			MoveUp();
	void			MoveLeft();
	void			MoveRight();

	void			PositionElements();

					/// Hightlights the current table element
	void			HighLightElement( const S32 sCol, const S32 sRow, const bool bHighlight );

					/// Selects the data in the current table element
	void			SelectElement( const bool bSelect );

					/// Returns true if the table has at least 1 column with 1 row
	bool			TableValid() const
					{
						return ( m_ActiveNodes.Size() && m_ActiveNodes[m_Column].Size() );
					}
public:
	S32				m_Pad;
	F32				m_RowPadding;
	F32				m_ColumnPadding;

private:
	typedef Btl::Vector<UINode*>	TRow;
	typedef Btl::Vector<TRow>		TColumn;

	TColumn			m_Nodes;		// Contains the original nodes that was added
	TColumn			m_ActiveNodes;	// Contains a filtered version of active nodes only
	S32				m_Row;
	S32				m_Column;
	bool			m_ElementActive;
	bool			m_FirstFrame;
};


}; // namespace UI


#endif // UI_NODELIST_H

