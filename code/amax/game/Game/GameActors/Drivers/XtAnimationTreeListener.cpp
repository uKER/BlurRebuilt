#include "stdafx.h"
#include <Game/GameActors/Drivers/XtAnimationTreeListener.h>
#include <Game/GameActors/Drivers/XtAnimationTree.h>

namespace AnimationTree
{

//
// C_XtAnimTreeListener
//

C_XtAnimTreeListener::C_XtAnimTreeListener()
{
	C_XtTree::AddListener(this);
}

C_XtAnimTreeListener::~C_XtAnimTreeListener()
{
	C_XtTree::RemoveListener(this);
}

void C_XtAnimTreeListener::OnXtAnimTreeCreated(XtRecord* record, const C_XtTree* tree)
{
}

void C_XtAnimTreeListener::OnXtAnimTreeDestroyed(XtRecord* record, const C_XtTree* tree)
{
}

} // end AnimationTree