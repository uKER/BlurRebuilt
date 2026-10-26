#ifndef __MISCSHARED_KDNODE_H__
#define __MISCSHARED_KDNODE_H__

#include <Btl/Containers/Vector.h>
#include <system/Serialise/Btl.h>
#include <Core/Maths.h>
#include <Core/Maths/Storage.h>

namespace Util
{

typedef Btl::Vector<MathsStorage::Box3> Box3Vector;

// maximum recursive depth of the kd-tree tracing code
#define KDNODE_STACK_SIZE 128

// some macros to save us typing so much
#define KDNODE_ISLEAF(n)	(n->iFlagDimAndOffset & (unsigned int)(1<<31))
#define KDNODE_DIMENSION(n) (n->iFlagDimAndOffset & 0x3)
#define KDNODE_OFFSET(n)	(n->iFlagDimAndOffset & (0x7FFFFFFC))
#define KDPTR_OFFSET(X,I)	((KdNode*)( ((char*)(X)) + (I)))

#define MAILBOX_HASH_TABLE_SIZE 128

// mailbox structure
struct KdMailbox
{
	int iPrimitive;		// primitive index
};

template<typename Archive>
void Serialise(Archive &ar, KdMailbox &item)
{
	ar << item.iPrimitive;
}

template<typename Archive>
void Deserialise(Archive &ar, KdMailbox &item)
{
	ar >> item.iPrimitive;
}

typedef Btl::Vector<KdMailbox> MailboxVector;

// the main kd-tree node
struct KdNode
{
	union
	{
		struct	// split node
		{
			unsigned int iFlagDimAndOffset;
			// bits 0..1 : splitting dimension
			// bits 2..30 : offset bits
			// bit 31 (sign) : flag whether node is a leaf
			float fSplitCoordinate;
		};
		struct	// leaf node
		{
			unsigned int iFlagAndNumPrimitives;
			// bits 0..30 : number of primitives attached to this leaf
			// bit 31 (sign) : flag whether node is a leaf
			unsigned int iPrimitive;
			// if only one primitive is attached to this leaf, this is the index of primitive
			// otherwise it's the index into the leaf primitive array
		};
	};
	void	InitLeaf(int *primNums, int np, MailboxVector *pmailboxes);
	void	InitSplit(int iAxis, float f, int iChildren);
};

template<typename Archive>
void Serialise(Archive &ar, KdNode &item)
{
	ar << item.iFlagAndNumPrimitives << item.iPrimitive;
}

template<typename Archive>
void Deserialise(Archive &ar, KdNode &item)
{
	ar >> item.iFlagAndNumPrimitives >> item.iPrimitive;
}

typedef Btl::Vector<KdNode> NodeVector;

// kd stack structure used to recurse through the kd-tree
struct KdStack
{
	KdNode *pNode;
	float	fNear;
	float	fFar;
	int		iPad;	// this makes it easier to retrieve an element by index 
};

struct KdTreeData
{
	NodeVector		m_Nodes;
	MailboxVector	m_Mailboxes;
};

template<typename Archive>
void Serialise(Archive &ar, KdTreeData &item)
{
	ar << item.m_Nodes << item.m_Mailboxes;
}

template<typename Archive>
void Deserialise(Archive &ar, KdTreeData &item)
{
	ar >> item.m_Nodes >> item.m_Mailboxes;
}

}

#endif
