#include "stdafx.h"
#include <EditorLib/Xt/XtMergeHelpers.h>

namespace EditorLib
{
namespace XtMergeHelpers
{

template<typename ChangeType, typename IndexType>
bool CanMergeT(Xt::Change* lhs, Xt::Change* rhs)
{
	ChangeType::Iterator itL(&((ChangeType*)lhs)->changes);
	ChangeType::Iterator itR(&((ChangeType*)rhs)->changes);

	while(itL && itR)
	{
		IndexType keyL = itL.Key();
		IndexType keyR = itR.Key();

		if (keyL < keyR)
		{
			return false;
		}
		else if (keyL > keyR)
		{
			return false;
		}
		else
		{
			if (!CanMerge(itL, itR))
				return false;

			++itL;
			++itR;
		}
	}

	return true;
}

bool CanMerge(Xt::Change* cl, Xt::Change* cr)
{
	if (!cl && !cr)
	{
		return true;
	}

	if (!cl)
		return false;

	if (!cr)
		return false;

	if (cl->type != cr->type)
		return false;

	switch(cl->type)
	{
		case Xt::kChangeLeaf:
		{
			break;
		}

		case Xt::kChangeId:
		{
			return ((Xt::ChangeId*)cl)->value == ((Xt::ChangeId*)cr)->value;
		}

		case Xt::kChangeComponent:
		{
			Xt::ChangeComponent* clc = (Xt::ChangeComponent*)cl;
			Xt::ChangeComponent* crc = (Xt::ChangeComponent*)cr;
			if (!CanMerge(clc->data, crc->data) || !CanMerge(clc->id, crc->id) || !CanMerge(clc->type, crc->type))
				return false;

			break;
		}

		case Xt::kChangeBlock:
		{
			if (!CanMergeT<Xt::ChangeBlock, const XtString&>(cl, cr))
				return false;

			break;
		}

		case Xt::kChangeArray:
		{
			if (!CanMergeT<Xt::ChangeArray, XtId>(cl, cr))
				return false;

			Xt::ChangeArray* lhs = (Xt::ChangeArray*)cl;
			Xt::ChangeArray* rhs = (Xt::ChangeArray*)cr;

			if (!CanMerge(lhs->index, rhs->index))
				return false;

			break;
		}

		default:
			return false;
	}

	return true;
}

template<typename ChangeType, typename IndexType>
bool ResolveConflictsT(ChangeType* result, Xt::Change* c)
{
	ChangeType::Iterator it(&((ChangeType*)c)->changes);

	while(it)
	{
		IndexType key = it.Key();
		Xt::Change* resolveResult = 0;
		if (!ResolveConflicts(&resolveResult, it))
			return false;

		if (!XtUt::Insert(&result->changes, key, resolveResult))
			return false;

		++it;
	}

	return true;
}

bool ResolveConflicts(Xt::Change** result, Xt::Change* c)
{
	if (!c)
	{
		*result = 0;
		return true;
	}

	switch(c->type)
	{
		case Xt::kChangeConflict:
		{
			Xt::ChangeConflict* pConflict = (Xt::ChangeConflict*)c;
			return (*result = pConflict->rhs->Clone()) != 0;
		}

		case Xt::kChangeLeaf:
			return (*result = c->Clone()) != 0;

		case Xt::kChangeComponent:
		{
			XtUt::Ptr<Xt::ChangeComponent> component = new Xt::ChangeComponent;

			if (!component || !ResolveConflicts(&component->data, ((Xt::ChangeComponent*)c)->data))
				return false;

			*result = Pop(&component);
		}
		return true;

		case Xt::kChangeBlock:
		{
			XtUt::Ptr<Xt::ChangeBlock> block = new Xt::ChangeBlock;

			if (!block  || !ResolveConflictsT<Xt::ChangeBlock, const XtString&>(block, c))
				return false;

			*result = Pop(&block);
		}
		return true;

		case Xt::kChangeArray:
		{
			XtUt::Ptr<Xt::ChangeArray> array = new Xt::ChangeArray;

			if (!array || !ResolveConflictsT<Xt::ChangeArray, XtId>(array, c))
				return false;

			Xt::ChangeArray* arr = (Xt::ChangeArray*)c;

			if (!ResolveConflicts(&array->index, arr->index))
				return false;

			*result = Pop(&array);
		}
		return true;

		default:
#ifdef BUILD_CONFIG_DEBUG
			assert(!"Unexpected type encountered during conflict resolution!");
#endif
			break;
	}

	return false;
}


}
}
