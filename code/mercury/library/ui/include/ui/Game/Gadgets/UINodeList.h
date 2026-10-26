
#ifndef UI_NODELIST_H
#define UI_NODELIST_H

#include "ui\Game\Gadgets\BaseUIVariableSelect.h"

namespace UI
{

/*---------------------------------------------------------------------------------------
	Class : UINode

	Desc : 
---------------------------------------------------------------------------------------*/
class UINode : public UIRenderBranch
{
public:
	DECLARE_GADGET( E_UIButtonType_Node, "NODE" );

	void			Init();

	void			Select( const bool bSelect, const bool bSendTimelineMsg = true );

	void			SendPrivateTimelineMessaage( const char* msg, const bool active, const F32 time = -1, const bool reset = false );

#ifndef CFG_GOLD
	void			Render();
#endif // CFG_GOLD

	//void			SendPrivateTimelineMessaage( const char* msg, const bool active, const F32 time = -1, const bool reset = false );

public:
	bool			m_Locked;
	F64				m_Value;
	S32				m_SortKey;
};


/*---------------------------------------------------------------------------------------
	Class : UINodeList

	Desc : 
---------------------------------------------------------------------------------------*/
class UINodeList : public I_SelectableGadgetBase
{
public:
	enum E_ListStyle
	{
		E_ListStyle_Horizontal = 0,
		E_ListStyle_Vertical,
		E_ListStyle_Table,
		E_ListStyle_Radial,
		E_ListStyle_TableFixed,

		E_ListStyle_Count,
	};

	enum E_RadialRotation
	{
		E_RadialRotation_X = 0,
		E_RadialRotation_Y,
		E_RadialRotation_Z,
	};

public:
							DECLARE_GADGET( E_UIButtonType_NodeList, "NODE_LIST" );

	void 					Init();
	void 					PostInit();
	void 					Setup( lua_State* pState );
	void 					FrameUpdate();
	bool					MessageHandler( const Messaging::C_MessageBase *pMessage );

	void					SetPad( const S32 pad )		{ m_Pad = pad; }

	void					AddNode( UINode* pNode );
	void					AddNode( UINode* pNode, const F64 uValue, const bool bLocked );
	void					AddNode( UINode* pNode, const bool bLocked );

	void					ScanForNodes( const bool bSnap = false );

	void					Select( const bool bSelect, const bool bSendTimelineMsg = true );
	void					Activate( const bool bActivate, const bool bSendTimelineMsg = true );

	const bool				SetCurrentItemByIndex( const S32 index );

	const bool				SetCurrentItemByValue( const F64 value );

	const S32				GetCurrentItemIndex() const
							{
								return (S32)m_CurrentItem;
							}

	const F64				GetCurrentItemValue( const U32 arg = 0 ) const
							{
								if ( m_CurrentItem < m_Nodes.Size() )
									return (F64)((UINode*)m_Nodes[m_CurrentItem])->m_Value;
								else
									return -1.0f;
							}

	void					SetListStyle( const E_ListStyle eStyle );

	void					SetItemLocked( const U32 uIndex, const bool bLocked );
	void					SetItemLockedByValue( const F64 value, const bool bLocked );

	void					OpenCurrentItem();
	void					CloseCurrentItem();

	UINode*					GetNodeByIndex( const U32 uIndex );

	void 					Render();

	void					SetItemSortKey( const U32 uIndex, const S32 sSortKey );
	void					SetItemSortKeyByValue( const F64 value, const S32 sSortKey );

	void					Sort();			// by the Node's m_SortKey - ascending

private:
	void					MoveDown();

	void					MoveUp();

	void					MoveLeft();

	void					MoveRight();

							/// Will check that the current item isnt locked and set current to the next free.
	void					SelectFirstUnlocked();

	bool					CheckCurrentItem( bool bBackwardsFirst = false );
	bool					CheckCurrentItemForwards();
	bool					CheckCurrentItemBackwards();

	void					CalculateDirectionalPositions();
	void					CalculateTablePositions();
	void					CalculateRadialPositions();
	void					DepthSort();

	void					ActivateCurrentItem( const U32 uPrevious, const U32 uCurrent );

	U32						NumActiveNodes();
	void					ScaleTableItems();

public:
	S32						m_Pad;

	E_ListStyle				m_ListStyle;
	C_Vector3Direction		m_Direction;
	F32						m_MoveSpeed;
	Padding					m_Padding;
	U32						m_ItemsVisible;
	F32						m_EndAlpha;

	// Update settings.
	bool					m_LoopList;
	bool					m_FlipList;
	bool					m_HighlightLocked;
	bool					m_HideLocked;
	bool					m_FlipControls;
	bool					m_KeepSelected;
	bool					m_PlayToggleSound;

	// Radial settings.
	F32						m_Radius;
	E_RadialRotation		m_RadialRotation;

	// Table settings.
	U32						m_Columns;
	F32						m_RowSpacing;
	F32						m_ColumnSpacing;

	// table fixed settings
	U32						m_uNumActiveNodes;
	U32						m_uIdealRows;
	U32						m_uIdealCols;

	F32						m_RadialAlphaFadeInnerAngle;
	F32						m_RadialAlphaFadeOuterAngle;

private:
	S32						m_CurrentItem;
	Maths::HVec3			m_CurrentPos;
	Maths::HVec3			m_CurrentDest;

	Btl::Vector<S32>		m_SortedRef;
	ButtonArray				m_Nodes;
	bool					m_FirstTimeEnable;

	float					m_ScrollOffset;
	float					m_ScrollOffsetVel;
	float					m_TargetScrollOffset;
};


}; // namespace UI


#endif // UI_NODELIST_H

