#include "Common/XtPch.h"
#include <Xt/XtTool.h>

namespace
{

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

	template<typename ChangeType, typename IndexType>
	bool DoMergeCompareT(ChangeType* result, Xt::Change* lhs, Xt::Change* rhs)
	{
		ChangeType::Iterator itL(&((ChangeType*)lhs)->changes);
		ChangeType::Iterator itR(&((ChangeType*)rhs)->changes);

		while(itL && itR)
		{
			IndexType keyL = itL.Key();
			IndexType keyR = itR.Key();

			if (keyL < keyR)
			{
				if (!XtUt::Insert(&result->changes, keyL, itL->Clone()))
					return false;

				++itL;
			}
			else if (keyL > keyR)
			{
				if (!XtUt::Insert(&result->changes, keyR, itR->Clone()))
					return false;

				++itR;
			}
			else
			{
				Xt::Change* mergeResult = 0;

				if (!DoMerge(&mergeResult, itL, itR))
					return false;

				if (!XtUt::Insert(&result->changes, keyR, mergeResult))
					return false;

				++itL;
				++itR;
			}
		}

		for(; itL; ++itL)
		{
			if (!XtUt::Insert(&result->changes, itL.Key(), itL->Clone()))
				return false;
		}

		for(; itR; ++itR)
		{
			if (!XtUt::Insert(&result->changes, itR.Key(), itR->Clone()))
				return false;
		}

		return true;
	}

	bool DoMerge(Xt::Change** result, Xt::Change* cl, Xt::Change* cr)
	{
		if (!cl && !cr)
		{
			*result = 0;
			return true;
		}

		if (!cl)
			return (*result = cr->Clone()) != 0;

		if (!cr)
			return (*result = cl->Clone()) != 0;

		if (cl->type != cr->type)
			return (*result = NewChangeConflict(cl, cr)) != 0;

		switch(cl->type)
		{
			case Xt::kChangeRemove:
			return (*result = cr->Clone()) != 0;

			case Xt::kChangeLeaf:
			{
				Xt::ChangeLeaf* lhs = (Xt::ChangeLeaf*)cl;
				Xt::ChangeLeaf* rhs = (Xt::ChangeLeaf*)cr;

				// TODO:	Merge experience would be improved if this created additional heirarchy and only conflicted on individual
				//			mismatched changes, rather than conflicting the entire tree.

				if (Compare(lhs->element, rhs->element))
					return (*result = NewChangeConflict(cl, cr)) != 0;
			}
			return (*result = cr->Clone()) != 0;

			case Xt::kChangeBlock:
			{
				XtUt::Ptr<Xt::ChangeBlock> block = new Xt::ChangeBlock;

				if (!block || !DoMergeCompareT<Xt::ChangeBlock, const XtString&>(block, cl, cr))
					return false;

				*result = Pop(&block);
			}
			return true;

			case Xt::kChangeArray:
			{
				XtUt::Ptr<Xt::ChangeArray> array = new Xt::ChangeArray;

				if (!array || !DoMergeCompareT<Xt::ChangeArray, XtId>(array, cl, cr))
					return false;

				Xt::ChangeArray* lhs = (Xt::ChangeArray*)cl;
				Xt::ChangeArray* rhs = (Xt::ChangeArray*)cr;

				if (!DoMerge(&array->index, lhs->index, rhs->index))
					return false;

				*result = Pop(&array);
			}
			return true;

			case Xt::kChangeComponent:
			{
				XtUt::Ptr<Xt::ChangeComponent> component = new Xt::ChangeComponent;

				Xt::ChangeComponent* lhs = (Xt::ChangeComponent*)cl;
				Xt::ChangeComponent* rhs = (Xt::ChangeComponent*)cr;

				if (!component || !DoMerge(&component->id, lhs->id, rhs->id))
					return false;

				if (!DoMerge(&component->type, lhs->type, rhs->type))
					return false;

				if (!DoMerge(&component->data, lhs->data, rhs->data))
					return false;

				*result = Pop(&component);
			}
			return true;

			case Xt::kChangeRecord:
			{
				XtUt::Ptr<Xt::ChangeRecord> record = new Xt::ChangeRecord;

				Xt::ChangeRecord* lhs = (Xt::ChangeRecord*)cl;
				Xt::ChangeRecord* rhs = (Xt::ChangeRecord*)cr;

				if (!record || !DoMerge(&record->name, lhs->name, rhs->name))
					return false;

				if (!DoMerge(&record->root, lhs->root, rhs->root))
					return false;

				*result = Pop(&record);
			}
			return true;

			case Xt::kChangePack:
			{
				XtUt::Ptr<Xt::ChangePack> pack = new Xt::ChangePack;

				Xt::ChangePack* lhs = (Xt::ChangePack*)cl;
				Xt::ChangePack* rhs = (Xt::ChangePack*)cr;

				if (!pack || !DoMerge(&pack->name, lhs->name, rhs->name))
					return false;

				if (!DoMerge(&pack->properties, lhs->properties, rhs->properties))
					return false;

				if (!DoMergeCompareT<Xt::ChangePack, XtId>(pack, cl, cr))
					return false;

				*result = Pop(&pack);
			}
			return true;

			case Xt::kChangeId:
			{
				Xt::ChangeId* lhs = (Xt::ChangeId*)cl;
				Xt::ChangeId* rhs = (Xt::ChangeId*)cr;

				if (lhs->value != rhs->value)
					return (*result = NewChangeConflict(cl, cr)) != 0;
			}
			return (*result = cr->Clone()) != 0;

			case Xt::kChangeString:
			{
				Xt::ChangeString* lhs = (Xt::ChangeString*)cl;
				Xt::ChangeString* rhs = (Xt::ChangeString*)cr;

				if (lhs->value != rhs->value)
					return (*result = NewChangeConflict(cl, cr)) != 0;
			}
			return (*result = cr->Clone()) != 0;

			case Xt::kChangeIndex:
			{
				Xt::ChangeIndex* lhs = (Xt::ChangeIndex*)cl;
				Xt::ChangeIndex* rhs = (Xt::ChangeIndex*)cr;

				if (!TestArrayEquality(&lhs->order, &rhs->order))
					return (*result = NewChangeConflict(cl, cr)) != 0;
			}
			return (*result = cr->Clone()) != 0;

			default:
				assert(0);
			return false;
		}
	}

}

namespace Xt
{

	//
	// Xt::Merge
	//

	Change* Merge(Change* lhs, Change* rhs)
	{
		Change* result = 0;

		if (!DoMerge(&result, lhs, rhs))
			return 0;

		return result;
	}

}