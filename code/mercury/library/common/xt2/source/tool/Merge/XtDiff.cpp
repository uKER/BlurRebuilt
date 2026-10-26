#include "Common/XtPch.h"
#include <Xt/XtTool.h>

/*

	STRICT BEHAVIOUR -

		Leafs verify their operations
		Dom nodes can become conflicted or obstructed
		Possible to do inverse patches
		Possible to save dom with conflicts and obstructions to be resolved later

	LOOSE BEHAVIOUR -

		Leafs commit their operations even if the data they're replacing doesn't match
		Dom nodes always in valid state
		Inverse patches can only be created via diffing two dom trees
		All conflicts must be resolved in the change set


	** PATCHING ARRAYS **

		Array index conflict behaviour under STRICT PATCH -

			Array would be unindexable, [index/whole node] marked as conflicted


		Array index conflict behaviour under LOOSE PATCH -

			Array index attempts a best case self healing, making sure all array nodes
			have a presence in the index.

			(1) Array elements are added and removed, new elements are put on the end in a random order

			(2) New index is either missing some entries, or has extra.
					- If has extra, simply remove the extras
					- If missing, ...

						Old:	ABcD
						New:	BADE
						Merged:	BAcDE

						Some kind of LCS problem -- However, LCS with all unique values


	** MERGING ARRAYS **

		Determine if index changes overlap -

			Identical	- Choose one
			Overlap		- Create a ChangeConflict
			Seperate	- Some how automatically merge the changes (at worse case can create a ChangeConflict)

		So:

			Partial Index -> Easy to determine overlap, just check each section for overlap, or touching

			Full Index -> No real way to determine overlap, unless I maintain additional data indicating changed indices

				HOWEVER - Additions and removals cannot conflict here, they would conflict in the element array.
				SO - To merge this all I need to do is come up with a reasonable approximation of the order of the stuff in the element array
				FINALLY - The best approach I can think of is to store a single run for the index, starting at the first change, and ending at the last

	Base: ABCD
	Lhs: ABCDE
	Rhs: BADF

	All		: ABCDEF
	--------:--------
	Common	: AB D
	Lhs		:   C E
	Rhs		:      F

	At this point all I know is that C, E and F have been added, without the common ancestor I cannot know that C was actually deleted in the RHS.

	I can possibly resolve this by including information for each index entry about whether it is added, removed, or common

	Base: A B C D
	Lhs: A B C D +E
	Rhs: ~B ~A -C D +F

	Common: A B D
	Lhs: C +E
	Rhs: -C +F

	Merged: ABD (C -C) +E +F -> ABDEF (without order)

		<ABD><E><F> -> Order of last two is unknown without user help
		ABD @E @F -> @ are conflicted entries, auto merging just needs to guess where the entries go, doesn't have to get it perfect, just mark those
						entries as being conflicted

	ABCDE
	BADF

	struct IndexInfo
	{
		enum { none, moved, added, removed, conflicted } action;
		XtId id;
	};

	(1) Attempt auto merge.
	(2) If any array entries are marked as conflicted then create a ChangeConflict and finish.
	(3) If auto merge was successful, then just set that as the resulting merge. (ChangeIndex?)


*/

void LcsTest()
{
}

int FindRhsIndex(Xt::DomArray* rhs, XtId id)
{
	// TODO: Use a hash map instead, or something... just a straight array sorted? no need for complex stuff

	for(Xt::DomArray::IndexIterator iRhs(rhs); iRhs; ++iRhs)
		if (iRhs.Key() == id)
			return iRhs.Index();

	return -1;
}

namespace
{

	//
	// DiffTrail
	//

	struct DiffIndexer;

	struct DiffTrail
	{
		DiffIndexer* parent;

	public:
		DiffTrail(DiffIndexer* parent_) :
			parent(parent_)
		{
		}

		virtual ~DiffTrail()
		{
		}

	public:
		virtual bool Create() = 0;
	};

	template<typename ChangeType>
	struct DiffTrailT : public DiffTrail
	{
		ChangeType* change;

	public:
		DiffTrailT(DiffIndexer* parent_) :
			DiffTrail(parent_),
			change(0)
		{
		}

	public:
		virtual bool Create()
		{
			ChangeType* ch = new ChangeType;

			if (!parent->Set(ch))
				return false;

			change = ch;

			return true;
		}
	};

	template<> bool DiffTrailT<Xt::Change>::Create()
	{
		return true;
	}

	//
	// DiffIndexer
	//

	struct DiffIndexer
	{
	public:
		virtual ~DiffIndexer()
		{
		}

	public:
		virtual bool Set(Xt::Change* change) = 0;
	};

	template<typename ChangeType, typename IndexType>
	struct DiffIndexerT : public DiffIndexer
	{
		DiffTrailT<ChangeType>*	parent;
		IndexType				index;

	public:
		DiffIndexerT(DiffTrailT<ChangeType>* parent_) :
			parent(parent_),
			index()
		{
		}

		DiffIndexerT(DiffTrailT<ChangeType>* parent_, IndexType index_) :
			parent(parent_),
			index(index_)
		{
		}

	public:
		virtual bool Set(Xt::Change* change)
		{
			if (!parent->change && !parent->Create())
				return false;

			return DoSet(change);
		}

		bool DoSet(Xt::Change* change);
	};

	class ArrayIndex { };

	class RecordName { };
	class RecordRoot { };

	class ComponentId { };
	class ComponentType { };
	class ComponentData { };

	class PackName { };
	class PackProperties { };

	class Root { };

	// Block

	template<>
	bool DiffIndexerT<Xt::ChangeBlock, const XtString&>::DoSet(Xt::Change *change)
	{
		return XtUt::Insert(&parent->change->changes, index, change);
	}

	// Array

	template<>
	bool DiffIndexerT<Xt::ChangeArray, XtId>::DoSet(Xt::Change *change)
	{
		return XtUt::Insert(&parent->change->changes, index, change);
	}

	template<>
	bool DiffIndexerT<Xt::ChangeArray, ArrayIndex>::DoSet(Xt::Change *change)
	{
		if (parent->change->index)
		{
			delete change;
			return 0;
		}

		return (parent->change->index = change) != 0;
	}

	// Component

	template<>
	bool DiffIndexerT<Xt::ChangeComponent, ComponentId>::DoSet(Xt::Change *change)
	{
		if (parent->change->id)
		{
			delete change;
			return 0;
		}

		return (parent->change->id = change) != 0;
	}

	template<>
	bool DiffIndexerT<Xt::ChangeComponent, ComponentType>::DoSet(Xt::Change *change)
	{
		if (parent->change->type)
		{
			delete change;
			return 0;
		}

		return (parent->change->type = change) != 0;
	}

	template<>
	bool DiffIndexerT<Xt::ChangeComponent, ComponentData>::DoSet(Xt::Change *change)
	{
		if (parent->change->data)
		{
			delete change;
			return 0;
		}

		return (parent->change->data = change) != 0;
	}

	// Record

	template<>
	bool DiffIndexerT<Xt::ChangeRecord, RecordName>::DoSet(Xt::Change *change)
	{
		if (parent->change->name)
		{
			delete change;
			return 0;
		}

		return (parent->change->name = change) != 0;
	}

	template<>
	bool DiffIndexerT<Xt::ChangeRecord, RecordRoot>::DoSet(Xt::Change *change)
	{
		if (parent->change->root)
		{
			delete change;
			return 0;
		}

		return (parent->change->root = change) != 0;
	}

	// Pack

	template<>
	bool DiffIndexerT<Xt::ChangePack, PackName>::DoSet(Xt::Change *change)
	{
		if (parent->change->name)
		{
			delete change;
			return 0;
		}

		return (parent->change->name = change) != 0;
	}

	template<>
	bool DiffIndexerT<Xt::ChangePack, PackProperties>::DoSet(Xt::Change *change)
	{
		if (parent->change->properties)
		{
			delete change;
			return 0;
		}

		return (parent->change->properties = change) != 0;
	}

	template<>
	bool DiffIndexerT<Xt::ChangePack, XtId>::DoSet(Xt::Change *change)
	{
		return XtUt::Insert(&parent->change->changes, index, change);
	}

	// Root

	template<>
	bool DiffIndexerT<Xt::Change, Root>::DoSet(Xt::Change *change)
	{
		if (parent->change)
		{
			delete change;
			return 0;
		}

		return (parent->change = change) != 0;
	}

	//
	// DoDiffArrayIndex
	//

	bool TestArrayEquality(XtUt::Array<XtId>* lhs, XtUt::Array<XtId>* rhs)
	{
		if (lhs->count != rhs->count)
			return false;

		for(int i = 0; i < (int)lhs->count; i++)
		{
			if ((*lhs)[i] != (*rhs)[i])
				return false;
		}

		return true;
	}

	bool DoDiffArrayIndex(Xt::DomArray* lhs, Xt::DomArray* rhs, DiffTrailT<Xt::ChangeArray>* trail)
	{
		if (TestArrayEquality(&lhs->index, &rhs->index))
			return true;

		return DiffIndexerT<Xt::ChangeArray, ArrayIndex>(trail).Set(Xt::NewChangeIndex(&rhs->index));
	}

	//
	// DoDiff
	//

	bool DoDiff(Xt::DomElement* el, Xt::DomElement* er, DiffIndexer* indexer);

	template<typename DomType, typename Iterator, typename ChangeType, typename IndexType>
	bool DoDiffCompareT(Xt::DomElement* el, Xt::DomElement* er, DiffTrailT<ChangeType>* trail)
	{
		Iterator itL((DomType*)el);
		Iterator itR((DomType*)er);

		while(itL && itR)
		{
			IndexType keyL = itL.Key();
			IndexType keyR = itR.Key();

			if (keyL < keyR)
			{
				if (!DoDiff(itL, 0, &DiffIndexerT<ChangeType, IndexType>(trail, keyL)))
					return false;

				++itL;
			}
			else if (keyL > keyR)
			{
				if (!DoDiff(0, itR, &DiffIndexerT<ChangeType, IndexType>(trail, keyR)))
					return false;

				++itR;
			}
			else
			{
				if (!DoDiff(itL, itR, &DiffIndexerT<ChangeType, IndexType>(trail, keyR)))
					return false;

				++itL;
				++itR;
			}
		}

		for(; itL; ++itL)
		{
			if (!DoDiff(itL, 0, &DiffIndexerT<ChangeType, IndexType>(trail, itL.Key())))
				return false;
		}

		for(; itR; ++itR)
		{
			if (!DoDiff(0, itR, &DiffIndexerT<ChangeType, IndexType>(trail, itR.Key())))
				return false;
		}

		return true;
	}

	Xt::Change* NewChangeLeafDeep(Xt::DomElement* element)
	{
		if (!element)
			return 0;

		switch(element->kind)
		{
			case Xt::kDomBlock:
			{
				XtUt::Ptr<Xt::ChangeBlock> change = new Xt::ChangeBlock;

				if (!change)
					return 0;

				for(Xt::DomBlock::Iterator it((Xt::DomBlock*)element); it; ++it)
				{
					if (!XtUt::Insert(&change->changes, it.Key(), NewChangeLeafDeep(it)))
						return 0;
				}

				return Pop(&change);
			}
			break;

			case Xt::kDomArray:
			{
				XtUt::Ptr<Xt::ChangeArray> change = new Xt::ChangeArray;

				if (!change)
					return 0;

				if ((change->index = Xt::NewChangeIndex(&((Xt::DomArray*)element)->index)) == 0)
					return 0;

				for(Xt::DomArray::ElementIterator it((Xt::DomArray*)element); it; ++it)
				{
					if (!XtUt::Insert(&change->changes, it.Key(), NewChangeLeafDeep(it)))
						return 0;
				}

				return Pop(&change);
			}
			break;

			case Xt::kDomComponent:
			{
				Xt::DomComponent*				com		= (Xt::DomComponent*)element;
				XtUt::Ptr<Xt::ChangeComponent>	change	= new Xt::ChangeComponent;

				if (!change)
					return 0;

				if ((change->id = Xt::NewChangeId(com->id)) == 0)
					return 0;

				if ((change->type = Xt::NewChangeString(com->type)) == 0)
					return 0;

				if ((change->data = NewChangeLeafDeep(&com->data)) == 0)
					return 0;

				return Pop(&change);
			}
			break;
		}

		return NewChangeLeaf(element);
	}

	bool DoDiff(Xt::DomElement* el, Xt::DomElement* er, DiffIndexer* indexer)
	{
		if (el == er) // Early out !el && !er
			return true;

		if (!er)
			return indexer->Set(new Xt::ChangeRemove);

		if (!el || (el->kind != er->kind))
			return indexer->Set(NewChangeLeafDeep(er));

		switch(el->kind)
		{
			case Xt::kDomField:
			{
				if (((Xt::DomField*)el)->value != ((Xt::DomField*)er)->value)
					return indexer->Set(NewChangeLeaf(er));
			}
			return true;

			case Xt::kDomRef:
			{
				if (((Xt::DomRef*)el)->value != ((Xt::DomRef*)er)->value)
					return indexer->Set(NewChangeLeaf(er));
			}
			return true;

			case Xt::kDomBlock:
			{
				DiffTrailT<Xt::ChangeBlock> trail(indexer);

				if (!DoDiffCompareT<Xt::DomBlock, Xt::DomBlock::Iterator, Xt::ChangeBlock, const XtString&>(el, er, &trail))
					return false;
			}
			return true;

			case Xt::kDomArray:
			{
				DiffTrailT<Xt::ChangeArray> trail(indexer);

				if (!DoDiffCompareT<Xt::DomArray, Xt::DomArray::ElementIterator, Xt::ChangeArray, XtId>(el, er, &trail))
					return false;

				if (!DoDiffArrayIndex((Xt::DomArray*)el, (Xt::DomArray*)er, &trail))
					return false;
			}
			return true;

			case Xt::kDomComponent:
			{
				Xt::DomComponent* lhs = (Xt::DomComponent*)el;
				Xt::DomComponent* rhs = (Xt::DomComponent*)er;

				DiffTrailT<Xt::ChangeComponent> trail(indexer);

				if (lhs->id != rhs->id)
				{
					if (!DiffIndexerT<Xt::ChangeComponent, ComponentId>(&trail).Set(Xt::NewChangeId(rhs->id)))
						return false;
				}

				if (lhs->type != rhs->type)
				{
					if (!DiffIndexerT<Xt::ChangeComponent, ComponentType>(&trail).Set(Xt::NewChangeString(rhs->type)))
						return false;
				}

				if (!DoDiff(&lhs->data, &rhs->data, &DiffIndexerT<Xt::ChangeComponent, ComponentData>(&trail)))
					return false;
			}
			return true;

			case Xt::kDomRecord:
			{
				Xt::DomRecord* lhs = (Xt::DomRecord*)el;
				Xt::DomRecord* rhs = (Xt::DomRecord*)er;

				DiffTrailT<Xt::ChangeRecord> trail(indexer);

				if (lhs->name != rhs->name)
				{
					if (!DiffIndexerT<Xt::ChangeRecord, RecordName>(&trail).Set(Xt::NewChangeString(rhs->name)))
						return false;
				}

				if (!DoDiff(&lhs->root, &rhs->root, &DiffIndexerT<Xt::ChangeRecord, RecordRoot>(&trail)))
					return false;
			}
			return true;

			case Xt::kDomPack:
			{
				Xt::DomPack* lhs = (Xt::DomPack*)el;
				Xt::DomPack* rhs = (Xt::DomPack*)er;

				DiffTrailT<Xt::ChangePack> trail(indexer);

				if (lhs->name != rhs->name)
				{
					if (!DiffIndexerT<Xt::ChangePack, PackName>(&trail).Set(Xt::NewChangeString(rhs->name)))
						return false;
				}

				if (!DoDiff(&lhs->properties, &rhs->properties, &DiffIndexerT<Xt::ChangePack, PackProperties>(&trail)))
					return false;

				if (!DoDiffCompareT<Xt::DomPack, Xt::DomPack::Iterator, Xt::ChangePack, XtId>(lhs, rhs, &trail))
					return false;
			}
			return true;

			default:
				assert(0);
			return false;
		}
	}

}

namespace Xt
{

	//
	// Xt::Diff
	//

	Change* Diff(DomElement* lhs, DomElement* rhs, bool* result)
	{
		DiffTrailT<Xt::Change> trail(0);

		if (result)
			*result = false;

		if (!DoDiff(lhs, rhs, &DiffIndexerT<Xt::Change, Root>(&trail)))
		{
			delete trail.change;

			return 0;
		}

		if (result)
			*result = true;

		return trail.change;
	}

}