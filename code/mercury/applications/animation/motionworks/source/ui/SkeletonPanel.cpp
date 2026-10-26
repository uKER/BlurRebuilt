#include "stdafx.h"
#include "SkeletonPanel.h"
#include "Frame.h"
#include <animation/skeleton/Skeleton.h>
#include "../PreviewState.h"
#include "GlobalSignals.h"
#include <wx/wupdlock.h>
#include <btl/Containers/vector.h>

IMPLEMENT_CLASS(C_SkeletonPanel, wxPanel)

enum E_ID
{
	E_ID_TreeCtrl	= wxID_HIGHEST + 200,
	E_ID_ExpandAllChildren,
	E_ID_CollapseAllChildren,
	E_ID_SkeletonChooserButton
};

BEGIN_EVENT_TABLE(C_SkeletonPanel, wxPanel)
	EVT_TREE_ITEM_MENU(E_ID_TreeCtrl,C_SkeletonPanel::OnItemMenu)
	EVT_TREE_SEL_CHANGED(E_ID_TreeCtrl, C_SkeletonPanel::OnItemSelected)
	EVT_MENU(E_ID_ExpandAllChildren, C_SkeletonPanel::OnExpandAllChildren)
	EVT_MENU(E_ID_CollapseAllChildren, C_SkeletonPanel::OnCollapseAllChildren)
END_EVENT_TABLE()

C_SkeletonPanel::C_SkeletonPanel(
	wxWindow* parent,
	wxWindowID winId,
	const wxPoint& pos,
	const wxSize& size
):
	wxPanel( parent, winId, pos, size ),
	m_skeleton( NULL )
{
	wxBoxSizer* topSizer = new wxBoxSizer( wxVERTICAL );
	
	// Skeleton name.
	m_skeletonNameStaticText = new wxStaticText(
		this, wxID_ANY, wxT( "" ), wxDefaultPosition, wxDefaultSize,
		wxST_NO_AUTORESIZE | wxST_DOTS_MIDDLE | wxBORDER_THEME );
	
	topSizer->Add( m_skeletonNameStaticText, wxSizerFlags( 0 ).Border( wxALL, 2 ).Expand() );

	// Filter.
	wxBoxSizer* filterSizer = new wxBoxSizer( wxHORIZONTAL );
	
	// Filter all by name or filter hierarchy.
	// Hierarchy includes all children of joints that pass the filter, even though they might not pass itself. probably just make this default?
	
	filterSizer->Add( new wxStaticText( this, wxID_ANY, _T("Filter:") ), wxSizerFlags().Right().Border(wxALL,2) );
	m_filterTextCtrl = new wxTextCtrl( this, wxID_ANY, _T(""), wxDefaultPosition, wxDefaultSize );
	filterSizer->Add( m_filterTextCtrl, wxSizerFlags(1).Expand().Border(wxALL,2) );
	Connect( m_filterTextCtrl->GetId(), ::wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler(C_SkeletonPanel::OnFilterChanged) );
	
	topSizer->Add(filterSizer, wxSizerFlags( 0 ).Border(wxALL,4).Expand());

	// Tree view.
	m_treeCtrl = new wxTreeCtrl( this, E_ID_TreeCtrl, wxDefaultPosition, wxSize(400, 600), wxTR_HAS_BUTTONS );
	topSizer->Add( m_treeCtrl, wxSizerFlags( 1 ).Border( wxALL, 4 ).Expand() );

	SetSizer( topSizer );
	topSizer->SetSizeHints( this );
	
	m_skeleton = NULL;
	
	C_GlobalSignals::Instance().jointSelected.connect(this, &C_SkeletonPanel::OnJointSelected);
	C_GlobalSignals::Instance().skeletonLoaded.connect(this, &C_SkeletonPanel::OnSkeletonLoaded);
	m_ignoreJointSelectedSignal = false;
}

void C_SkeletonPanel::OnJointSelected(S32 jointIndex)
{
	if (m_ignoreJointSelectedSignal) return;

	// Find matching item.
	wxTreeItemId result = FindItemWithJointIndex(m_treeCtrl->GetRootItem(), jointIndex);
	
	if (result.IsOk())
	{
		m_treeCtrl->SelectItem(result);
		m_treeCtrl->ScrollTo(result);
		m_treeCtrl->EnsureVisible(result);
	}
	else
	{
		m_treeCtrl->Unselect();
	}
}

void C_SkeletonPanel::OnSkeletonLoaded(const Animation::C_Skeleton* skeleton, Animation::C_SkeletonInstance* skeletonInstance, Animation::C_Pose* pose, const char* name)
{
	m_skeleton = skeleton;
	BuildTreeFromSkeleton( *m_treeCtrl, m_skeleton, m_filterTextCtrl->GetValue().c_str() );
	
	if ( skeleton )
	{
		m_treeCtrl->Enable( true );

		m_skeletonNameStaticText->SetLabel( wxString(name) + wxString::Format( " (%d joints)", skeleton->GetNumJoints() ) );
		m_skeletonNameStaticText->Enable( true );
	}
	else
	{
		m_treeCtrl->Enable( false );
		m_skeletonNameStaticText->SetLabel( wxT( "" ) );
		m_skeletonNameStaticText->Enable( false );
	}
	
	Layout();

}

wxTreeItemId C_SkeletonPanel::FindItemWithJointIndex(const wxTreeItemId& item, S32 jointIndex) const
{
	const C_JointTreeItemData* data = static_cast<const C_JointTreeItemData*>(m_treeCtrl->GetItemData(item));
	if (data != NULL)
	{
		if (data->m_jointIndex == jointIndex) return item;
	}
	
	wxTreeItemIdValue cookie;
	wxTreeItemId child = m_treeCtrl->GetFirstChild(item, cookie);
	
	while (child.IsOk())
	{
		wxTreeItemId result = FindItemWithJointIndex(child, jointIndex);
		if (result.IsOk())
		{
			return result;
		}
		child = m_treeCtrl->GetNextChild(item, cookie);
	}
	
	return wxTreeItemId();
}

void C_SkeletonPanel::BuildTreeFromSkeleton(
	wxTreeCtrl& treeCtrl,
	const Animation::C_Skeleton* skeleton,
	const Btl::String& filter
) const
{
	wxWindowUpdateLocker updateLocker( &treeCtrl );
	
	treeCtrl.DeleteAllItems();

	if ( !skeleton )
	{
		return;
	}
	
	const U32 numJoints = skeleton->GetNumJoints();
	Btl::BitVector includeFlags;
	includeFlags.Resize( numJoints );
	includeFlags.ClearBits();
	
	Btl::BitVector filterFlags = includeFlags;
	
	
	S32 firstVisible = -1;
	
	for ( U32 i = 0; i < numJoints; ++i )
	{
		Btl::String jointName( skeleton->GetJointName( i ).GetString() );
		
		if ( includeFlags.GetBitValue(i) )
		{
			// Already included.
			continue;
		}
		
		if ( filter.Length() == 0 || (jointName.ISearch( filter ) >= 0) )
		{
			if ( firstVisible < 0 )
			{
				firstVisible = i;
			}
			filterFlags.SetBit( i );
			
			S32 jointIndex = i;
			while ( jointIndex >= 0 )
			{
				includeFlags.SetBit( jointIndex );
				jointIndex = skeleton->GetJointParentIndex( jointIndex );
			}
			
			for ( U32 j = i + 1; j < numJoints; ++j )
			{
				S32 parentIndex = skeleton->GetJointParentIndex( j );
				
				if ( parentIndex >= 0 )
				{
					if ( filterFlags.GetBitValue( parentIndex ) )
					{
						filterFlags.SetBit( j );
						includeFlags.SetBit( j );
					}
				}
			}
		}
	}
		
	Btl::Vector< wxTreeItemId > jointTreeItemIds;
	jointTreeItemIds.Resize( skeleton->GetNumJoints() + 1 );
	
	jointTreeItemIds[ 0 ] = treeCtrl.AddRoot( "< Root >", -1, -1, new C_JointTreeItemData( -1 ) );

	Btl::String dynamicStr = "dynamic";
	Btl::String nullStr = "_null";
	
	wxColour kRed(220, 0, 0);
	wxColour kGreen(0, 0, 190);
	
	for ( U32 i = 0, count = skeleton->GetNumJoints(); i < count; ++i )
	{
		if ( includeFlags.GetBitValue(i) )
		{
			Btl::String jointName( skeleton->GetJointName( i ).GetString() );
			C_JointTreeItemData* data = new C_JointTreeItemData( i );
			const wxTreeItemId& parentId = jointTreeItemIds[ skeleton->GetJointParentIndex( i ) + 1 ];
			assert(parentId.IsOk());
			jointTreeItemIds[ i + 1 ] = treeCtrl.AppendItem( parentId, jointName.Chars(), -1, -1, data );
			
			
			if (jointName.ISearch(dynamicStr, 0) >= 0)
			{
				// Make dynamic joints appear in red.
				treeCtrl.SetItemTextColour(jointTreeItemIds[i+1], kRed);
			}
			else if (jointName.ISearch(nullStr, 0) >= 0)
			{
				// Null joints appear green.
				treeCtrl.SetItemTextColour(jointTreeItemIds[i+1], kGreen);
			}
		}
	}
	
	gClipPreviewState->filteredJoints = filterFlags;
	
	treeCtrl.ExpandAll();
	wxTreeItemId firstVisibleItemId = jointTreeItemIds[ firstVisible + 1 ];
	treeCtrl.SelectItem( firstVisibleItemId );
	treeCtrl.EnsureVisible( firstVisibleItemId );
}

void C_SkeletonPanel::OnItemMenu( wxTreeEvent& e )
{
	if ( !m_skeleton || !e.GetItem() )
	{
		return;
	}

	const C_JointTreeItemData* data = static_cast<const C_JointTreeItemData*>( m_treeCtrl->GetItemData( e.GetItem() ) );
	
	wxString menuTitle;
	
	if ( data->m_jointIndex >= 0 )
	{
		menuTitle = wxString::Format( "%s [%d]", m_skeleton->GetJointName( data->m_jointIndex ).GetString(), data->m_jointIndex );
	}
	else
	{
		menuTitle = "< Root >";
	}
	wxMenu menu( menuTitle );

	menu.Append( E_ID_ExpandAllChildren, wxT( "Expand All Children" ) );
	menu.Append( E_ID_CollapseAllChildren, wxT( "Collapse All Children" ) );

	m_focusItem = e.GetItem();

	//menu.AppendSeparator();
	
	// TODO: set compression settings?
	// TODO: show curves? or just have a curve window that shows them?

	PopupMenu( &menu );
}

void C_SkeletonPanel::OnItemSelected( wxTreeEvent& e )
{
	S32 jointIndex = -1;
	if ( m_skeleton && e.GetItem().IsOk() )
	{
		const C_JointTreeItemData* data = static_cast<const C_JointTreeItemData*>( m_treeCtrl->GetItemData( e.GetItem() ) );
		jointIndex = data->m_jointIndex;
	}
	
	m_ignoreJointSelectedSignal = true;
	C_GlobalSignals::Instance().jointSelected(jointIndex);
	m_ignoreJointSelectedSignal = false;
}

void C_SkeletonPanel::OnFilterChanged( wxCommandEvent& e )
{
	BuildTreeFromSkeleton( *m_treeCtrl, m_skeleton, e.GetString().c_str() );
}

void C_SkeletonPanel::OnExpandAllChildren( wxCommandEvent& e )
{
	if ( m_focusItem )
	{
		m_treeCtrl->ExpandAllChildren( m_focusItem );
	}
}

void C_SkeletonPanel::OnCollapseAllChildren( wxCommandEvent& e )
{
	if ( m_focusItem )
	{
		m_treeCtrl->CollapseAllChildren( m_focusItem );
	}
}




