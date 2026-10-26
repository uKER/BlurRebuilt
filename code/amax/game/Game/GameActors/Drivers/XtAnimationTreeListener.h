#ifndef XTANIMATIONTREELISTENER_H
#define XTANIMATIONTREELISTENER_H

#include <btl/containers/list.h>

struct XtRecord;

namespace AnimationTree
{

class C_XtTree;

/**
Interface for handling animation tree construction and destruction events.
*/
class C_XtAnimTreeListener: public Btl::ListElementDouble<C_XtAnimTreeListener>
{
public:
	C_XtAnimTreeListener();
	virtual ~C_XtAnimTreeListener();

	virtual void OnXtAnimTreeCreated(XtRecord* record, const C_XtTree* tree);
	virtual void OnXtAnimTreeDestroyed(XtRecord* record, const C_XtTree* tree);
	virtual const char* GetXtAnimTreeName() const = 0;
};

} // end AnimationTree

#endif
