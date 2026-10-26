#include "Common/XtPch.h"
#include "Xt/XtTool.h"
#include "XtFlaskHelpers.h"
#include "Xt/Impl/Grimoire.h"

namespace
{
	void OutputMsg(XtPackBuildParams* params, XtSeverity severity, const char* text, va_list vl)
	{
		if (!params)
			return;

		char buffer[512];

		_vsnprintf_s(buffer, sizeof(buffer), _TRUNCATE, text, vl);

		params->Output(severity, buffer);
	}

	void MsgInfo(XtPackBuildParams* params, const char* text, ...)
	{
		va_list vl;

		va_start(vl, text);
		OutputMsg(params, kXtSevInfo, text, vl);
		va_end(vl);
	}

	void MsgWarning(XtPackBuildParams* params, const char* text, ...)
	{
		va_list vl;

		va_start(vl, text);
		OutputMsg(params, kXtSevWarning, text, vl);
		va_end(vl);
	}

	void MsgError(XtPackBuildParams* params, const char* text, ...)
	{
		va_list vl;

		va_start(vl, text);
		OutputMsg(params, kXtSevError, text, vl);
		va_end(vl);
	}

	void MsgTrace(XtPackBuildParams* params, const char* text, ...)
	{
		va_list vl;

		va_start(vl, text);
		OutputMsg(params, kXtSevTrace, text, vl);
		va_end(vl);
	}
}

bool NewId(CompileContext* ctx, XtId id, CompileElement* element)
{
	if (!id)
		return false;

	CompileId* cid = new CompileId;

	if (!XtUt::Insert(&ctx->ids, id, cid))
		return false;

	cid->id			= id;
	cid->element	= element;

	return true;
}

CompileId* FindId(CompileContext* ctx, XtId id)
{
	return id ? XtUt::At(&ctx->ids, id) : 0;
}

bool Initialise(CompileContext* ctx, ExportSchema* schema)
{
	ctx->schema = schema;

	for(int i = 0; i < kXtComponentBehaviourCount; i++)
	{
		if (!Allocate(&ctx->maps[i], Count(&schema->types), Count(&schema->types)))
			return false;
	}

	return true;
}

CompileComponent* AcquireComponent(CompileContext* ctx, ExportType* type, XtComponentBehaviour behaviour)
{
	CompileComponent* component = ctx->maps[behaviour][type->index];

	if ((component == 0) && ((component = AppendEx(&ctx->components, new CompileComponent)) != 0))
	{
		component->index		= ctx->components.count - 1;
		component->type			= type;
		component->behaviour	= behaviour;

		ctx->maps[behaviour][type->index] = component;
	}

	return component;
}

CompileElement* NewElement(CompileContext* ctx, ExportType* type, XtComponentBehaviour componentBehaviour, CompileElementBehaviour elementBehaviour, Xt::DomComponent* dom, CompileField* owner)
{
	CompileComponent* component = AcquireComponent(ctx, type, componentBehaviour);

	if (!component)
		return 0;

	CompileElement* element = AppendEx(&component->elements, new CompileElement);

	if (!element)
		return 0;

	element->index		= component->elements.count - 1;
	element->component	= component;
	element->dom		= dom;
	element->behaviour	= elementBehaviour;

	int totalFieldCount = (componentBehaviour == kXtCbObject) ? type->totalFieldCount : 1;

	if (!Allocate(&element->fields, totalFieldCount, totalFieldCount))
		return 0;

	if (owner)
		owner->element = element;

	return element;
}

int NewElements(CompileContext* ctx, ExportType* type, CompileComponent* component, XtComponentBehaviour componentBehaviour, CompileElementBehaviour elementBehaviour, Xt::DomArray* dom, CompileField* owner)
{
	if (!component)
		return -1;

	int baseIndex = component->elements.count;

	int					startIndex	= Count(&component->elements);
	CompileElement**	elements	= AppendExN(&component->elements, Count(dom));

	if (!elements)
		return -1;

	int totalFieldCount = (componentBehaviour == kXtCbObject) ? type->totalFieldCount : 1;

	for(size_t i = 0; i < Count(dom); i++)
	{
		CompileElement* element = (elements[i] = new CompileElement);

		if (!element)
			return -1;

		element->index		= baseIndex + i;
		element->component	= component;
		element->dom		= At(dom, i);
		element->behaviour	= elementBehaviour;

		if (!Allocate(&element->fields, totalFieldCount, totalFieldCount))
			return -1;
	}

	if (owner)
		owner->element = elements[0];

	return startIndex;
}

//
// Compile
//

bool CompileStruct(CompileContext* ctx, Xt::DomComponent* dom, CompileField* owner, ExportType* type = 0);
bool CompileStructTop(CompileContext* ctx, ExportType* type, CompileElement* element);

bool CompileStructArray(CompileContext* ctx, ExportField* field, Xt::DomArray* dom, CompileField* cf)
{
	if (Count(dom) <= 0)
		return true;

	XtComponentBehaviour	cb;
	CompileElementBehaviour	eb;

	switch(field->behaviour)
	{
		case kXtFbAtom:		cb = kXtCbObject;	eb = kCeNone;	break;
		case kXtFbEnum:		cb = kXtCbObject;	eb = kCeNone;	break;
		case kXtFbFlags:	cb = kXtCbObject;	eb = kCeNone;	break;
		case kXtFbStruct:	cb = kXtCbObject;	eb = kCeNone;	break;
		case kXtFbPtr:		cb = kXtCbPtr;		eb = kCeNone;	break;
		case kXtFbHandle:	cb = kXtCbHandle;	eb = kCeNone;	break;
		case kXtFbImplWeak:	cb = kXtCbPtr;		eb = kCeWeak;	break;

		default: assert(0); return false;
	}

	CompileComponent*	component		= AcquireComponent(ctx, field->type, cb);
	int					arrElStartIndex	= NewElements(ctx, field->type, component, cb, eb, dom, cf);

	if (arrElStartIndex < 0)
		return false;

	switch(field->behaviour)
	{
		case kXtFbStruct:
			for(size_t j = 0; j < Count(dom); j++)
			{
				CompileElement* arrElement = component->elements[arrElStartIndex + j];

				if (!CompileStructTop(ctx, field->type, arrElement))
					return false;
			}
		break;

		case kXtFbPtr:
			for(size_t j = 0; j < Count(dom); j++)
			{
				CompileElement* arrElement = component->elements[arrElStartIndex + j];

				if (!CompileStruct(ctx, AsComponent(arrElement->dom), &arrElement->fields[0]))
					return false;
			}
		break;
	}

	return true;
}

bool CompileStructBase(CompileContext* ctx, ExportType* type, Xt::DomBlock* dom, CompileElement* element, CompileField* fields)
{
	if (!dom)
		return true;

	if (type->bases.count > 0)
	{
		for(size_t i = 0; i < type->bases.count; i++)
		{
			ExportBase* base = type->bases[i];

			if (!CompileStructBase(ctx, base->type, dom, element, fields + base->fieldOffset))
				return false;
		}
	}

	for(size_t i = 0; i < type->fields.count; i++)
	{
		ExportField*	field	= type->fields[i];
		CompileField*	cf		= fields + field->fieldIndex;

		if (field->array)
		{
			if (!CompileStructArray(ctx, field, AsArray(At(dom, field->name)), cf))
				return false;
		}
		else
		{
			switch(field->behaviour)
			{
				case kXtFbAtom:
				case kXtFbEnum:
				case kXtFbFlags:
				case kXtFbImplWeak:
				case kXtFbHandle:
				break;

				case kXtFbStruct:
					if (!CompileStructBase(ctx, field->type, AsBlock(At(dom, field->name)), element, cf))
						return false;
				break;

				case kXtFbPtr:
					if (!CompileStruct(ctx, AsComponent(At(dom, field->name)), cf))
						return false;
				break;

				default:
					assert(0);
				return false;
			}
		}
	}

	return true;
}

bool CompileStructTop(CompileContext* ctx, ExportType* type, CompileElement* element)
{
	Xt::DomComponent* domComponent = AsComponent(element->dom);

	if (!domComponent)
		return true;

	if (domComponent->id && !NewId(ctx, domComponent->id, element))
		return false;

	return CompileStructBase(ctx, type, &domComponent->data, element, element->fields.elements);
}

bool CompileStruct(CompileContext* ctx, Xt::DomComponent* dom, CompileField* owner, ExportType* type)
{
	if (!dom)
		return true;

	if (!type && ((type = FindTypeByName(ctx->schema, dom->type)) == 0))
		return true;

	if (type->behaviour != kXtFbStruct)
		return false;

	CompileElement* element = NewElement(ctx, type, kXtCbObject, kCeNone, dom, owner);

	if (!element)
		return false;

	if (!CompileStructTop(ctx, type, element))
		return false;

	return true;
}

//
// Allocate
//

bool AllocateData(CompileContext* ctx)
{
	size_t totalBytes	= 0;
	size_t totalHandles	= 0;

	for(size_t i = 0; i < ctx->components.count; i++)
	{
		CompileComponent* component = ctx->components[i];

		totalBytes		+= XtUt::AlignUpper(SizeOf(component) * component->elements.count, kXtFlaskAlign);
		totalHandles	+= HandleCountOf(component) * component->elements.count;
	}

	if ((totalBytes > 0) && !Allocate(&ctx->record->data, totalBytes))
		return false;

	if (!Allocate(&ctx->record->handleFixups, 0, totalHandles))
		return false;

	Clear(&ctx->record->data);

	XtU8*	data	= ctx->record->data.start;
	size_t	offset	= 0;

	for(size_t i = 0; i < ctx->components.count; i++)
	{
		CompileComponent* component = ctx->components[i];

		component->offset = offset;

		offset += XtUt::AlignUpper(SizeOf(component) * component->elements.count, kXtFlaskAlign);
	}

	return true;
}

//
// Export
//

bool EmitRecord(CompileContext* ctx);
bool EmitStrings(ExportStringTable* strings, XtUt::Block* data, XtPackBuildContext* pb);
bool EmitComponents(CompileContext* ctx);

bool CompileRecord(ExportSchema* schema, Xt::DomRecord* dom, ExportRecord* record, XtPackBuildParams* params, XtPackBuildContext* pb)
{
	if (!schema || !dom || !record)
		return false;

	CompileContext ctx;

	ctx.record = record;

	if ((record->id = dom->id) == 0)
	{
		MsgError(params, "%s(%08X): Record has id of zero", (const char*)dom->name, dom->id);
		return false;
	}

	if (!Initialise(&ctx, schema))
	{
		MsgError(params, "%s(%08X): CompileRecord->Initialise failed", (const char*)dom->name, dom->id);
		return false;
	}

	ExportType* type = FindTypeByName(schema, dom->root.type);

	if (!type)
	{
		MsgError(params, "%s(%08X): Record of unknown type '%s'", (const char*)dom->name, dom->id, (const char*)dom->root.type);
		return false;
	}

	if (!CompileStruct(&ctx, &dom->root, 0, type))
	{
		MsgError(params, "%s(%08X): CompileRecord->CompileStruct failed", (const char*)dom->name, dom->id);
		return false;
	}

	if (!AllocateData(&ctx))
	{
		MsgError(params, "%s(%08X): CompileRecord->AllocateData failed", (const char*)dom->name, dom->id);
		return false;
	}

	if (!EmitRecord(&ctx))
	{
		MsgError(params, "%s(%08X): CompileRecord->EmitRecord failed", (const char*)dom->name, dom->id);
		return false;
	}

	if (!EmitStrings(&ctx.strings, &record->strings, pb))
	{
		MsgError(params, "%s(%08X): CompileRecord->EmitStrings failed", (const char*)dom->name, dom->id);
		return false;
	}

	if (!EmitComponents(&ctx))
	{
		MsgError(params, "%s(%08X): CompileRecord->EmitComponents failed", (const char*)dom->name, dom->id);
		return false;
	}

	return true;
}

bool CompileTocRefSortFunc(XtId lhs, XtId rhs)
{
	return lhs < rhs;
}

bool CompileRefList(FlaskExportContext* fec, XtFeRefList* rl)
{
	size_t refCount = fec->records.count;

	for(size_t i = 0; i < fec->records.count; i++)
		refCount += fec->records[i]->handleFixups.count;

	XtUt::Array<XtId> sortedIds;

	if (refCount > 0)
	{
		if (!Allocate(&sortedIds, refCount, refCount))
			return false;

		for(size_t i = 0, n = 0; i < fec->records.count; i++)
		{
			ExportRecord* record = fec->records[i];

			sortedIds[n++] = record->id;

			for(size_t j = 0; j < record->handleFixups.count; j++)
				sortedIds[n++] = *(XtId*)(record->data.start + record->handleFixups[j].offset);
		}

		XtUt::HeapSort(&sortedIds, CompileTocRefSortFunc);

		const XtId*	src = sortedIds.elements;
		const XtId*	end = sortedIds.elements + sortedIds.count;
		XtId*		dst = sortedIds.elements;

		*dst++ = *src++;

		for(; src < end; src++)
		{
			if (*src != *(dst - 1))
				*dst++ = *src;
		}

		sortedIds.count = dst - sortedIds.elements;
	}

	if (!Allocate(&rl->refs, 0, sortedIds.count))
		return false;

	for(size_t i = 0; i < sortedIds.count; i++)
	{
		ExportType* type = fec->pb->callback(fec->pb, sortedIds[i]);

		if (!type)
			continue;

		XtFeRecordRef* ref = AppendN(&rl->refs, 1);

		ref->index	= rl->refs.count - 1;
		ref->id		= sortedIds[i];
		ref->type	= type;
	}

	return true;
}

XtFeRecordRef* FindRef(XtFeRefList* rl, XtId id)
{
	size_t			i	= 0;
	size_t			j	= rl->refs.count;
	XtFeRecordRef*	arr	= rl->refs.elements;

	while(i < j)
	{
		size_t	midIdx = (i + j) / 2;
		XtId	midVal = arr[midIdx].id;

		if (id < midVal)
			j = midIdx;
		else if (id > midVal)
			i = midIdx + 1;
		else
			return &arr[midIdx];
	}

	return 0;
}

bool FixupRefs(FlaskExportContext* fec, XtFeRefList* rl)
{
	for(size_t i = 0; i < fec->records.count; i++)
	{
		ExportRecord* record = fec->records[i];

		for(size_t j = 0; j < record->handleFixups.count; j++)
		{
			ExportHandleFixup*	fixup	= &record->handleFixups[j];
			XtId*				id		= (XtId*)(record->data.start + fixup->offset);
			XtFeRecordRef*		ref		= FindRef(rl, *id);

			*id = ref ? ref->index : (XtId)-1;
		}
	}

	return true;
}

void SwizzleRecords(FlaskExportContext* fec)
{
	for(size_t i = 0; i < fec->records.count; i++)
	{
		ExportRecord* record = fec->records[i];

		SwizzleRecord(record);
	}
}

int RegisterDependency(XtFeTypeList* td, ExportType* type, bool stub)
{
	if (XtFeTypeRef* ref = td->map[type->index])
	{
		bool oldStub = ref->stub;

		ref->stub = ref->stub && stub;

		return (ref->stub != oldStub) ? 1 : 0;
	}

	XtFeTypeRef* ref = AppendN(&td->types, 1);

	ref->index	= td->types.count - 1;	
	ref->type	= type;
	ref->stub	= stub;

	td->map[type->index] = ref;

	return 1;
}

bool CompileTypes(FlaskExportContext* fec, XtFeRefList* rl, XtFeTypeList* att, ExportSchema* schema, CompileFlaskContext* flask, XtPackBuildParams* params)
{
	if (!Allocate(&att->types, 0, schema->types.count))
		return false;

	if (!Allocate(&att->map, schema->types.count, schema->types.count))
		return false;

	for(size_t i = 0; i < rl->refs.count; i++)
		RegisterDependency(att, rl->refs[i].type, true);

	for(size_t i = 0; i < fec->records.count; i++)
	{
		ExportRecord* record = fec->records[i];

		for(size_t j = 0; j < record->components.count; j++)
		{
			ExportComponent* component = &record->components[j];

			RegisterDependency(att, schema->types[component->type->index], (component->behaviour != kXtCbObject));
		}
	}

	for(;;)
	{
		int changes = 0;

		for(size_t i = 0; i < att->types.count; i++)
		{
			XtFeTypeRef* at = &att->types[i];

			for(size_t j = 0; j < at->type->bases.count; j++)
			{
				ExportBase* base = at->type->bases[j];

				changes += RegisterDependency(att, base->type, at->stub);
			}

			if (!at->stub)
			{
				for(size_t j = 0; j < at->type->fields.count; j++)
				{
					ExportField* field = at->type->fields[j];

					switch(field->behaviour)
					{
						case kXtFbAtom:
						case kXtFbEnum:
						case kXtFbFlags:
						case kXtFbStruct:
							changes += RegisterDependency(att, field->type, false);
						break;

						case kXtFbPtr:
						case kXtFbHandle:
						case kXtFbImplWeak:
							changes += RegisterDependency(att, field->type, true);
						break;

						case kXtFbLabel:
						break;

						default:
							assert(0);
						return false;
					}
				}
			}
		}

		if (changes <= 0)
			break;
	}

	size_t baseCount	= 0;
	size_t fieldCount	= 0;

	for(size_t i = 0; i < att->types.count; i++)
	{
		XtFeTypeRef* at = &att->types[i];

		if (!at->stub)
		{
			baseCount	+= at->type->bases.count;
			fieldCount	+= at->type->fields.count;
		}
	}

	if (!Allocate(&flask->types, 0, att->types.count))
		return false;

	if (!Allocate(&flask->bases, 0, baseCount))
		return false;

	if (!Allocate(&flask->fields, 0, fieldCount))
		return false;

	ExportStringTable strings;

	XtEndian targetEndian = fec->pb->targetProfile->endian;

	for(size_t i = 0; i < att->types.count; i++)
	{
		XtFeTypeRef*	at		= &att->types[i];
		ExportType*		type	= at->type;	
		FlaskType*		ft		= AppendN(&flask->types, 1);

		const ExportString* str = Add(&strings, type->name, kEseAnsi);

		if (!str)
			return false;

		ft->name		= EndianSwizzle<XtU16>(str->offset, targetEndian);
		ft->baseCount	= EndianSwizzle<XtU16>(at->stub ? 0 : type->bases.count, targetEndian);
		ft->fieldCount	= EndianSwizzle<XtU16>(at->stub ? 0 : type->fields.count, targetEndian);
		ft->behaviour	= EndianSwizzle<XtU16>(type->behaviour, targetEndian);
		ft->atom		= EndianSwizzle<XtU16>(type->atom, targetEndian);
		ft->size		= EndianSwizzle<XtU16>(type->size, targetEndian);

		if (!at->stub)
		{
			for(size_t j = 0; j < type->bases.count; j++)
			{
				ExportBase*	base	= type->bases[j];
				FlaskBase*	fb		= AppendN(&flask->bases, 1);

				fb->type	= EndianSwizzle<XtU16>(att->map[base->type->index]->index, targetEndian);
				fb->offset	= EndianSwizzle<XtU16>(base->offset, targetEndian);
			}

			for(size_t j = 0; j < type->fields.count; j++)
			{
				ExportField*	field	= type->fields[j];
				FlaskField*		ff		= AppendN(&flask->fields, 1);

				const ExportString* str = Add(&strings, field->name, kEseAnsi);

				if (!str)
					return false;

				ff->name		= EndianSwizzle<XtU16>(str->offset, targetEndian);
				ff->type		= EndianSwizzle<XtU16>((field->behaviour != kXtFbLabel) ?  att->map[field->type->index]->index : -1, targetEndian);
				ff->offset		= EndianSwizzle<XtU16>(field->offset, targetEndian);
				ff->behaviour	= EndianSwizzle<XtU8>((field->behaviour == kXtFbImplWeak) ? kXtFbPtr : field->behaviour, targetEndian);
				ff->array		= EndianSwizzle<XtU8>(field->array, targetEndian);
			}
		}
	}

	return EmitStrings(&strings, &flask->strings, fec->pb);
}

bool CompileTocSortFunc(ExportRecord* lhs, ExportRecord* rhs)
{
	return lhs->id <= rhs->id;
}

bool CompileToc(FlaskExportContext* fec, XtFeRefList* rl, XtFeTypeList* att, CompileFlaskContext* flask)
{
	XtUt::HeapSort(&fec->records, CompileTocSortFunc);

	size_t componentCount = 0;

	for(size_t i = 0; i < fec->records.count; i++)
		componentCount += fec->records[i]->components.count;

	if (!Allocate(&flask->refs, 0, rl->refs.count))
		return false;

	if (!Allocate(&flask->records, 0, fec->records.count))
		return false;

	if (!Allocate(&flask->components, 0, componentCount))
		return false;

	XtEndian targetEndian = fec->pb->targetProfile->endian;

	size_t recordIndex = 0;

	for(size_t i = 0; i < rl->refs.count; i++)
	{
		XtFeRecordRef*	rr	= &rl->refs[i];
		FlaskRef*		ref	= AppendN(&flask->refs, 1);;

		ref->id		= EndianSwizzle<XtId>(rr->id, targetEndian);
		ref->type	= EndianSwizzle<XtU16>(att->map[rr->type->index]->index, targetEndian);

		if ((recordIndex < fec->records.count) && (fec->records[recordIndex]->id == rr->id))
		{
			ExportRecord* record = fec->records[recordIndex];

			if ((record->components.count <= 0) || (record->components[0].type != rr->type))
				return false;

			ref->record	= EndianSwizzle<XtU16>(recordIndex++, targetEndian);
		}
		else
			ref->record = EndianSwizzle<XtU16>(-1, targetEndian);
	}

	if (recordIndex != fec->records.count)
		return false;

	for(size_t i = 0; i < fec->records.count; i++)
	{
		ExportRecord*	record	= fec->records[i];
		FlaskRecord*	fr		= AppendN(&flask->records, 1);
		XtFeRecordRef*	ref		= FindRef(rl, record->id);

		if (!ref)
			return false;

		fr->ref				= EndianSwizzle<XtU32>(ref->index, targetEndian);
		fr->componentCount	= EndianSwizzle<XtU32>(record->components.count, targetEndian);
		fr->dataBytes		= EndianSwizzle<XtU32>(Capacity(&record->data), targetEndian);
		fr->stringBytes		= EndianSwizzle<XtU32>(Capacity(&record->strings), targetEndian);

		for(size_t j = 0; j < record->components.count; j++)
		{
			ExportComponent*	component	= &record->components[j];
			FlaskComponent*		fc			= AppendN(&flask->components, 1);

			fc->type		= EndianSwizzle<XtU16>(att->map[component->type->index]->index, targetEndian);
			fc->count		= EndianSwizzle<XtU16>(component->count, targetEndian);
			fc->behaviour	= EndianSwizzle<XtU16>(component->behaviour, targetEndian);
		}
	}

	return true;
}

template<typename Type>
size_t ComputeBlobBytes(XtUt::Array<Type>* arr)
{
	return XtUt::AlignUpper(arr->count * sizeof(Type), kXtFlaskAlign);
}

template<typename Type>
void WriteBlob(XtUt::Block* mb, size_t offset, XtUt::Array<Type>* arr)
{
	Write(mb, offset, arr->elements, arr->count * sizeof(Type));
}

bool CompileFlask(FlaskExportContext* fec, CompileFlaskContext* flask, XtUt::Block* data)
{
	size_t headerBytes		= sizeof(FlaskHeader);
	size_t typeBytes		= ComputeBlobBytes(&flask->types);
	size_t baseBytes		= ComputeBlobBytes(&flask->bases);
	size_t fieldBytes		= ComputeBlobBytes(&flask->fields);
	size_t stringBytes		= Capacity(&flask->strings);
	size_t refBytes			= ComputeBlobBytes(&flask->refs);
	size_t recordBytes		= ComputeBlobBytes(&flask->records);
	size_t componentBytes	= ComputeBlobBytes(&flask->components);
	size_t dataBytes		= 0;

	for(size_t i = 0; i < fec->records.count; i++)
	{
		ExportRecord* record = fec->records[i];

		dataBytes += Capacity(&record->data);
		dataBytes += Capacity(&record->strings);
	}

	size_t totalBytes = headerBytes + typeBytes + baseBytes + fieldBytes + stringBytes + refBytes + recordBytes + componentBytes + dataBytes;

	if (!Allocate(data, totalBytes))
		return false;

	Clear(data);

	size_t headerOffset		= 0;
	size_t typeOffset		= headerOffset + headerBytes;
	size_t baseOffset		= typeOffset + typeBytes;
	size_t fieldOffset		= baseOffset + baseBytes;
	size_t stringOffset		= fieldOffset + fieldBytes;
	size_t refOffset		= stringOffset + stringBytes;
	size_t recordOffset		= refOffset + refBytes;
	size_t componentOffset	= recordOffset + recordBytes;
	size_t dataOffset		= componentOffset + componentBytes;

	XtEndian targetEndian = fec->pb->targetProfile->endian;

	FlaskHeader header;

	header.tag					= EndianSwizzle(kXtFlaskTag, targetEndian);
	header.version				= EndianSwizzle(2, targetEndian);

	header.types.offset			= EndianSwizzle(typeOffset, targetEndian);
	header.types.count			= EndianSwizzle(flask->types.count, targetEndian);

	header.bases.offset			= EndianSwizzle(baseOffset, targetEndian);
	header.bases.count			= EndianSwizzle(flask->bases.count, targetEndian);

	header.fields.offset		= EndianSwizzle(fieldOffset, targetEndian);
	header.fields.count			= EndianSwizzle(flask->fields.count, targetEndian);

	header.strings.offset		= EndianSwizzle(stringOffset, targetEndian);
	header.strings.count		= EndianSwizzle(stringBytes, targetEndian);

	header.refs.offset			= EndianSwizzle(refOffset, targetEndian);
	header.refs.count			= EndianSwizzle(flask->refs.count, targetEndian);

	header.records.offset		= EndianSwizzle(recordOffset, targetEndian);
	header.records.count		= EndianSwizzle(flask->records.count, targetEndian);

	header.components.offset	= EndianSwizzle(componentOffset, targetEndian);
	header.components.count		= EndianSwizzle(flask->components.count, targetEndian);

	header.data.offset			= EndianSwizzle(dataOffset, targetEndian);
	header.data.count			= EndianSwizzle(dataBytes, targetEndian);

	Write(data, headerOffset, &header, headerBytes);

	WriteBlob(data, typeOffset,		&flask->types);
	WriteBlob(data, baseOffset,		&flask->bases);
	WriteBlob(data, fieldOffset,	&flask->fields);

	Write(data, stringOffset, flask->strings.start, stringBytes);

	WriteBlob(data, refOffset,			&flask->refs);
	WriteBlob(data, recordOffset,		&flask->records);
	WriteBlob(data, componentOffset,	&flask->components);

	for(size_t i = 0; i < fec->records.count; i++)
	{
		ExportRecord* record = fec->records[i];

		Write(data, dataOffset, record->data.start, Capacity(&record->data));

		dataOffset += Capacity(&record->data);

		Write(data, dataOffset, record->strings.start, Capacity(&record->strings));

		dataOffset += Capacity(&record->strings);
	}

	return true;
}

bool XtPackBuildEx(ExportSchema* schema, XtPackBuildContext* pb, XtUt::Block* data, XtPackBuildParams* params)
{
	FlaskExportContext fec;

	fec.schema	= schema;
	fec.pb		= pb;

	if (!Allocate(&fec.records, 0, pb->records.count))
	{
		MsgError(params, "Out of memory");
		return false;
	}

	for(size_t i = 0; i < pb->records.count; i++)
	{
		Xt::DomRecord* dom = pb->records[i];

		if (!dom)
			continue;

		if (!FindTypeByName(schema, dom->root.type))
		{
			MsgWarning(params, "Skipping record of unknown type");
			MsgTrace(params, "Record: %s", dom->name.Text());
			MsgTrace(params, "Type: %s", dom->root.type.Text());
			continue;
		}

		ExportRecord* record = Append(&fec.records, new ExportRecord);

		if (!record)
		{
			MsgError(params, "Out of memory");
			return false;
		}

		if (!CompileRecord(schema, pb->records[i], record, params, pb))
			return false;
	}

	CompileFlaskContext flask;
	XtFeRefList rl;
	XtFeTypeList att;

	if (!CompileRefList(&fec, &rl))
	{
		MsgError(params, "CompileRefList failed");
		return false;
	}

	if (!FixupRefs(&fec, &rl))
	{
		MsgError(params, "FixupRefs failed");
		return false;
	}

	if (XtGetEndian() != pb->targetProfile->endian)
		SwizzleRecords(&fec);

	if (!CompileTypes(&fec, &rl, &att, schema, &flask, params))
	{
		MsgError(params, "CompileTypes failed");
		return false;
	}

	if (!CompileToc(&fec, &rl, &att, &flask))
	{
		MsgError(params, "CompileToc failed");
		return false;
	}

	if (!CompileFlask(&fec, &flask, data))
	{
		MsgError(params, "CompileFlask failed");
		return false;
	}

	return true;
}

struct LvPackBuildPrepCheck : LvComponents
{
	bool hasExports;

public:
	LvPackBuildPrepCheck() :
		hasExports(false)
	{
	}

public:
	virtual bool Visit(LtStruct* type, Xt::DomBlock* dom)
	{
		if (type->hasExport)
		{
			hasExports = true;
			return false;
		}

		return true;
	}
};

struct LvPackBuildPrep : LvComponents
{
	Xt::DomDb* db;

public:
	LvPackBuildPrep(Xt::DomDb* db_) :
		db(db_)
	{
	}

public:
	bool DoExport(LtStruct* type, Xt::DomBlock* dom)
	{
		if (type->base)
		{
			if (!DoExport(type->base->type, dom))
				return false;
		}

		for(XtUt::List<LarvaField>::Iterator it(&type->fields); it; ++it)
		{
			LarvaAttr* attr = it->attrs.FindAttr("Export");

			if (!attr)
				continue;

			XtString method = attr->GetString("method");

			if (method == "shaderConst")
			{
				if (!XtCustomExport_ShaderConst(&XtCustomExportContext(db, type, dom, it, attr)))
					return false;
			}
		}

		return true;
	}

public:
	virtual bool Visit(LtStruct* type, Xt::DomBlock* dom)
	{
		if (type->hasExport)
			return type->hasExport ? DoExport(type, dom) : true;

		return true;
	}
};

bool XtPackBuildPrep(Xt::DomDb* db, XtPackBuildContext* pb, XtPackBuildParams* params)
{
	for(size_t i = 0; i < pb->records.count; i++)
	{
		Xt::DomRecord* record = pb->records[i];

		if (LtStruct* type = AsStruct(TypeOf(db->schema, record)))
		{
			LvPackBuildPrepCheck check;

			VisitAllComponents(db->schema, type, &record->root, &check);

			if (check.hasExports)
			{
				if ((record = XtUt::AppendEx(&pb->tmpRecords, (Xt::DomRecord*)Clone(record))) == 0)
				{
					MsgError(params, "XtPackBuildPrep failed cloning record: %s", record->name.Text());
					return false;
				}

				pb->records[i] = record;

				if (!VisitAllComponents(db->schema, type, &record->root, &LvPackBuildPrep(db)))
				{
					MsgError(params, "XtPackBuildPrep failed for record: %s", record->name.Text());
					return false;
				}
			}
		}
	}

	return true;
}

XtPackBuildParams::XtPackBuildParams()
{
}

XtPackBuildParams::~XtPackBuildParams()
{
}

void XtPackBuildParams::Output(XtSeverity severity, const char* text)
{
	switch(severity)
	{
		case kXtSevInfo:	DebugLn("XtPackBuild: [INF] %s", text); break;
		case kXtSevWarning:	DebugLn("XtPackBuild: [WRN] %s", text); break;
		case kXtSevError:	DebugLn("XtPackBuild: [ERR] %s", text); break;
		case kXtSevTrace:	DebugLn("XtPackBuild:       %s", text); break;
	}
}

ExportType* XtPackRecordTypeCallback(XtPackBuildContext* pb, XtId id)
{
	Xt::DomDb*		db		= (Xt::DomDb*)pb->context;
	Xt::DomRecord*	record	= FindRecordById(db, id);

	if (!record)
		return 0;

	return FindTypeByName(db->exportSchema, record->root.type);
}

bool XtPackBuild(XtTargetProfile* targetProfile, Xt::DomDb* db, Xt::DomRecord** records, size_t count, XtUt::Block* data, XtPackBuildParams* params)
{
	assert(db != 0);
	assert(records != 0);
	assert(data != 0);

	XtPackBuildContext pb;

	pb.targetProfile	= targetProfile;
	pb.callback			= XtPackRecordTypeCallback;
	pb.context			= db;

	if (!Allocate(&pb.records, 0, count))
		return false;

	for(size_t i = 0; i < count; i++)
		Append(&pb.records, records[i]);

	if (!XtPackBuildPrep(db, &pb, params))
		return false;

	return XtPackBuildEx(db->exportSchema, &pb, data, params);
}

bool XtPackBuild(XtTargetProfile* targetProfile, Xt::DomDb* db, Xt::DomPack* pack, XtUt::Block* data, XtPackBuildParams* params)
{
	assert(db != 0);
	assert(pack != 0);
	assert(data != 0);

	XtPackBuildContext pb;

	pb.targetProfile	= targetProfile;
	pb.callback			= XtPackRecordTypeCallback;
	pb.context			= db;

	int count = 0;

	for(Xt::DomPack::Iterator itRecord(pack); itRecord; ++itRecord)
	{
		if (itRecord->root.type == "XtLocTable")
			continue;

		count++;
	}

	if (!Allocate(&pb.records, 0, count))
		return false;

	for(Xt::DomPack::Iterator itRecord(pack); itRecord; ++itRecord)
	{
		if (itRecord->root.type == "XtLocTable")
			continue;

		Append(&pb.records, (Xt::DomRecord*)itRecord);
	}

	if (!XtPackBuildPrep(db, &pb, params))
		return false;

	return XtPackBuildEx(db->exportSchema, &pb, data, params);
}