#ifndef SCENE_GRAPH_VISIBLEDETECT_H
#define SCENE_GRAPH_VISIBLEDETECT_H

namespace SceneGraph
{

enum E_VisDetectFlag
{
	// bitfield
	E_VisDetectFlag_Visible				= 0xffff,// these flags are set by the scenegraph when a modelinstance is visible in a particular view
	E_VisDetectFlag_AddedToNewList		= 1<<30, // this flag is used by the scenegraph to avoid duplicates in the 'newly visible' list
	E_VisDetectFlag_AddedToVisibleList	= 1<<29, // this flag is used to avoid duplicates in the 'current visible' list
	E_VisDetectFlag_DefaultVisible		= 1<<28  // this flag is always set when an object is visible
};

// probe structure that can detect the visibilty of an object using the scenegraph
class C_VisibleDetect
{
protected:
	C_VisibleDetect() : flag(0) {}

public:
	// returns true if the object is currently visible
	U32 IsVisible() const { return (flag & (E_VisDetectFlag_Visible | E_VisDetectFlag_DefaultVisible)); }
	U32 IsVisible(U32 view) const { return flag & view; }
	U32 GetVisibleFlags() const { return flag & E_VisDetectFlag_Visible; }

	U32 flag;		// E_VisDetectFlag
};

}

#endif // SCENE_GRAPH_VISIBLEDETECT_H
