#include "Common/XtPch.h"
#include <Xt/XtTool.h>

namespace
{

	bool DoPatch(Xt::DomElement* parent, Xt::Indexer* indexer, Xt::Change* change)
	{
		switch(change->type)
		{
			case Xt::kChangeNone:
			return true;

			case Xt::kChangeRemove:
				indexer->Remove(parent);
			return true;

			case Xt::kChangeLeaf:
			{
				Xt::ChangeLeaf* cl = (Xt::ChangeLeaf*)change;

				if (!indexer->Set(parent, &Xt::MetaElement(Clone(cl->element))))
					return false;
			}
			return true;

			case Xt::kChangeBlock:
			{
				Xt::ChangeBlock*	cb		= (Xt::ChangeBlock*)change;
				Xt::DomBlock*		block	= indexer->AcquireBlock(parent);

				if (!block)
					return false;

				for(Xt::ChangeBlock::Iterator itChange(&cb->changes); itChange; ++itChange)
				{
					if (!DoPatch(block, &Xt::BlockIndexer(itChange.Key()), itChange))
						return false;
				}
			}
			return true;

			case Xt::kChangeArray:
			{
				Xt::ChangeArray*	ca		= (Xt::ChangeArray*)change;
				Xt::DomArray*		array	= indexer->AcquireArray(parent);

				if (!array)
					return false;

				for(Xt::ChangeArray::Iterator itChange(&ca->changes); itChange; ++itChange)
				{
					if (!DoPatch(array, &Xt::ArrayIndexer(itChange.Key()), itChange))
						return false;
				}

				if (ca->index && !DoPatch(array, &Xt::ArrayIndexer(Xt::ArrayIndexer::kIndex), ca->index))
					return false;
			}
			return true;

			case Xt::kChangeComponent:
			{
				Xt::ChangeComponent*	cc			= (Xt::ChangeComponent*)change;
				Xt::DomComponent*		component	= indexer->AcquireComponent(parent);

				if (!component)
					return false;

				if (cc->id && !DoPatch(component, &Xt::ComponentIndexer(Xt::ComponentIndexer::kId), cc->id))
					return false;

				if (cc->type && !DoPatch(component, &Xt::ComponentIndexer(Xt::ComponentIndexer::kType), cc->type))
					return false;

				if (cc->data && !DoPatch(component, &Xt::ComponentIndexer(Xt::ComponentIndexer::kData), cc->data))
					return false;
			}
			return true;

			case Xt::kChangeRecord:
			{
				Xt::ChangeRecord*	cr		= (Xt::ChangeRecord*)change;
				Xt::DomRecord*		record	= indexer->AcquireRecord(parent);

				if (!record)
					return false;

				if (cr->name && !DoPatch(record, &Xt::RecordIndexer(Xt::RecordIndexer::kName), cr->name))
					return false;

				if (cr->root && !DoPatch(record, &Xt::RecordIndexer(Xt::RecordIndexer::kRoot), cr->root))
					return false;
			}
			return true;

			case Xt::kChangePack:
			{
				Xt::ChangePack*	cp		= (Xt::ChangePack*)change;
				Xt::DomPack*	pack	= indexer->AcquirePack(parent);

				if (!pack)
					return false;

				if (cp->name && !DoPatch(pack, &Xt::PackIndexer(Xt::PackIndexer::kName), cp->name))
					return false;

				if (cp->properties && !DoPatch(pack, &Xt::PackIndexer(Xt::PackIndexer::kProperties), cp->properties))
					return false;

				for(Xt::ChangePack::Iterator itChange(&cp->changes); itChange; ++itChange)
				{
					if (!DoPatch(pack, &Xt::PackIndexer(itChange.Key()), itChange))
						return false;
				}
			}
			return true;

			case Xt::kChangeId:
			{
				Xt::ChangeId* cs = (Xt::ChangeId*)change;

				if (!indexer->Set(parent, &Xt::MetaId(cs->value)))
					return false;
			}
			return true;

			case Xt::kChangeString:
			{
				Xt::ChangeString* cs = (Xt::ChangeString*)change;

				if (!indexer->Set(parent, &Xt::MetaString(cs->value)))
					return false;
			}
			return true;

			case Xt::kChangeIndex:
			{
				Xt::ChangeIndex* ci = (Xt::ChangeIndex*)change;

				if (!indexer->Set(parent, &Xt::MetaIndex(&ci->order)))
					return false;
			}
			return true;

			case Xt::kChangeConflict:
			{
				Xt::ChangeConflict* cc = (Xt::ChangeConflict*)change;

				// TODO: Just choosing rhs for now

				bool chooseRight = true;

				if (cc->lhs && (cc->lhs->type == Xt::kChangeRemove))
					chooseRight = false;

				if (!DoPatch(parent, indexer, chooseRight ? cc->rhs : cc->lhs))
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
	// Xt::Patch
	//

	bool Patch(DomElement* element, Change* change)
	{
		if (!element || !change)
			return false;

		return DoPatch(element, &Xt::NullIndexer(), change);
	}

}