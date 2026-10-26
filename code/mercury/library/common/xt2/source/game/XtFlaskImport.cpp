#include "Common/XtPch.h"
#include "XtGameImpl.h"
#include <Xt/Impl/XtUt.h>
#include "Common/XtFlask.h"

#if defined(PLATFORM_PC)
#define vsnprintf(buf, sizeofBuf, text, vl) _vsnprintf_s(buf, sizeofBuf, _TRUNCATE, text, vl);
#endif

namespace
{
	template<typename Type>
	Type* ReadBlob(XtUt::Block* flask, size_t offset, size_t count)
	{
		if ((offset & (kXtFlaskAlign - 1)) != 0)
			return 0;

		if (count > (0x40000000 / sizeof(Type)))
			return 0;

		size_t length = sizeof(Type) * count;

		if (offset + length < offset)
			return 0;

		if (offset > Capacity(flask))
			return 0;

		if (offset + length > Capacity(flask))
			return 0;

		return (Type*)(flask->start + offset);
	}

	template<typename Type>
	Type* ReadBlob(XtUt::Block* flask, FlaskBlob* blob)
	{
		return ReadBlob<Type>(flask, blob->offset, blob->count);
	}

	void DoMsg(XtPackOpenParams* params, const char* text, ...)
	{
		if (!params || !params->callback)
			return;

		char buf[512];
		va_list vl;

		va_start(vl, text);
		vsnprintf(buf, sizeof(buf), text, vl);
		va_end(vl);

		XtPackOpenStatus status;

		status.message = buf;

		params->callback(&status, params->context);
	}
}

struct FiField;
struct FiBase;

struct FiType
{
	XtType*		xt;

	size_t		baseCount;
	FiBase*		bases;

	size_t		fieldCount;
	FiField*	fields;

	size_t		size;

public:
	inline FiType() :
		xt(0),
		baseCount(0),
		bases(0),
		fieldCount(0),
		fields(0),
		size(0)
	{
	}
};

struct FiBase
{
	XtBase*	xt;
	FiType*	type;
	size_t	offset;

public:
	inline FiBase() :
		xt(0),
		type(0),
		offset(0)
	{
	}
};

struct FiField
{
	XtField*	xt;
	FiType*		type;
	size_t		offset;

public:
	inline FiField() :
		xt(0),
		type(0),
		offset(0)
	{
	}
};

struct FiSchema
{
	XtUt::Array<FiType>		types;
	XtUt::Array<FiBase>		bases;
	XtUt::Array<FiField>	fields;
};

enum ValidateFlaskError
{
	kVfeOkay,
	kVfeFailed,
	kVfeHeader,
	kVfeVersion
};

ValidateFlaskError ValidateFlask(XtUt::Block* flask)
{
	FlaskHeader* flaskHeader = ReadBlob<FlaskHeader>(flask, 0, 1);

	if (!flaskHeader || (flaskHeader->tag != kXtFlaskTag))
		return kVfeHeader;

	if (
			(flaskHeader->version != 1) &&	// v1: Initial version
			(flaskHeader->version != 2)		// v2: Encoded strings
		)
	{
		return kVfeVersion;
	}

	FlaskType*		flaskTypes		= ReadBlob<FlaskType>(		flask, &flaskHeader->types		);
	FlaskBase*		flaskBases		= ReadBlob<FlaskBase>(		flask, &flaskHeader->bases		);
	FlaskField*		flaskFields		= ReadBlob<FlaskField>(		flask, &flaskHeader->fields		);
	char*			flaskStrings	= ReadBlob<char>(			flask, &flaskHeader->strings	);
	FlaskRef*		flaskRefs		= ReadBlob<FlaskRef>(		flask, &flaskHeader->refs		);
	FlaskRecord*	flaskRecords	= ReadBlob<FlaskRecord>(	flask, &flaskHeader->records	);
	FlaskComponent*	flaskComponents	= ReadBlob<FlaskComponent>(	flask, &flaskHeader->components	);
	XtU8*			flaskData		= ReadBlob<XtU8>(			flask, &flaskHeader->data		);

	if (!flaskTypes || !flaskBases || !flaskFields || !flaskStrings || !flaskRefs || !flaskRecords || !flaskComponents || !flaskData)
		return kVfeFailed;

	// FlaskTypes

	for(size_t i = 0; i < flaskHeader->types.count; i++)
	{
		FlaskType* type = &flaskTypes[i];

		if (type->name >= flaskHeader->strings.count)
			return kVfeFailed;

		if (type->behaviour >= kXtTypeBehaviourCount)
			return kVfeFailed;

		if (type->behaviour == kXtTbAtom)
		{
			if ((type->atom == kXtNotAtom) || (type->atom >= kXtAtomTypeCount))
				return kVfeFailed;

			if (type->fieldCount > 0)
				return kVfeFailed;
		}
		else
		{
			if (type->atom != kXtNotAtom)
				return kVfeFailed;
		}

		if ((type->behaviour == kXtTbAtom) || (type->behaviour == kXtTbEnum) || (type->behaviour == kXtTbFlags))
		{
			if (type->baseCount > 0)
				return kVfeFailed;
		}
	}

	// FlaskBase and FlaskFields

	size_t baseIndex	= 0;
	size_t fieldIndex	= 0;

	for(size_t i = 0; i < flaskHeader->types.count; i++)
	{
		FlaskType* type = &flaskTypes[i];

		for(size_t j = 0; j < type->baseCount; j++)
		{
			if (baseIndex + j >= flaskHeader->bases.count)
				return kVfeFailed;

			FlaskBase* base = &flaskBases[baseIndex + j];

			if (base->type >= flaskHeader->types.count)
				return kVfeFailed;

			FlaskType* baseType = &flaskTypes[base->type];

			if (baseType->behaviour != kXtTbStruct)
				return kVfeFailed;

			if (base->offset > type->size)
				return kVfeFailed;

			if (base->offset + baseType->size > type->size)
				return kVfeFailed;
		}

		for(size_t j = 0; j < type->fieldCount; j++)
		{
			if (fieldIndex + j >= flaskHeader->fields.count)
				return kVfeFailed;

			FlaskField* field = &flaskFields[fieldIndex + j];

			if (field->name >= flaskHeader->strings.count)
				return kVfeFailed;

			if (field->behaviour >= kXtFieldBehaviourCount)
				return kVfeFailed;

			switch(type->behaviour)
			{
				case kXtTbEnum:
				case kXtTbFlags:
					if (field->type != (XtU16)-1)
						return kVfeFailed;

					if (field->offset != 0)
						return kVfeFailed;

					if (field->behaviour != kXtFbLabel)
						return kVfeFailed;

					if (field->array != 0)
						return kVfeFailed;
				break;

				case kXtTbStruct:
				{
					if (field->type >= flaskHeader->types.count)
						return kVfeFailed;

					FlaskType* fieldType = &flaskTypes[field->type];

					if (field->offset > type->size)
						return kVfeFailed;

					if (field->array)
					{
						if (field->offset + sizeof(FlaskArray) > type->size)
							return kVfeFailed;
					}
					else
					{
						switch(field->behaviour)
						{
							case kXtFbAtom:
							case kXtFbEnum:
							case kXtFbFlags:
							case kXtFbStruct:
								if (field->offset + fieldType->size > type->size)
									return kVfeFailed;
							break;

							case kXtFbPtr:
							case kXtFbHandle:
								if (field->offset + sizeof(FlaskPtr) > type->size)
									return kVfeFailed;
							break;
						}
					}

					switch(field->behaviour)
					{
						case kXtFbAtom:
							if (fieldType->behaviour != kXtTbAtom)
								return kVfeFailed;
						break;

						case kXtFbEnum:
							if (fieldType->behaviour != kXtTbEnum)
								return kVfeFailed;
						break;

						case kXtFbFlags:
							if (fieldType->behaviour != kXtTbFlags)
								return kVfeFailed;
						break;

						case kXtFbStruct:
						case kXtFbPtr:
						case kXtFbHandle:
							if (fieldType->behaviour != kXtTbStruct)
								return kVfeFailed;
						break;

						default:
						return kVfeFailed;
					}
				}
				break;
			}
		}

		baseIndex	+= type->baseCount;
		fieldIndex	+= type->fieldCount;
	}

	if (baseIndex != flaskHeader->bases.count)
		return kVfeFailed;

	if (fieldIndex != flaskHeader->fields.count)
		return kVfeFailed;

	// FlaskRefs

	XtId lastId = 0;

	for(size_t i = 0; i < flaskHeader->refs.count; i++)
	{
		FlaskRef* handleRef = &flaskRefs[i];

		if (handleRef->id <= lastId)
			return kVfeFailed;

		if (handleRef->type >= flaskHeader->types.count)
			return kVfeFailed;

		if ((handleRef->record != (XtU16)-1) && (handleRef->record >= flaskHeader->records.count))
			return kVfeFailed;

		lastId = handleRef->id;
	}

	// FlaskRecords

	size_t	componentIndex	= 0;
	size_t	dataOffset		= 0;

	for(size_t i = 0; i < flaskHeader->records.count; i++)
	{
		FlaskRecord* record = &flaskRecords[i];

		if (record->ref >= flaskHeader->refs.count)
			return kVfeFailed;

		if (record->componentCount <= 0)
			return kVfeFailed;

		XtU32 componentBytes = 0;

		for(size_t j = 0; j < record->componentCount; j++)
		{
			if (componentIndex + j >= flaskHeader->components.count)
				return kVfeFailed;

			FlaskComponent* component = &flaskComponents[componentIndex + j];

			if (component->type >= flaskHeader->types.count)
				return kVfeFailed;

			if (component->behaviour >= kXtComponentBehaviourCount)
				return kVfeFailed;

			XtU32 elementBytes = 0;

			switch(component->behaviour)
			{
				case kXtCbObject:	elementBytes = flaskTypes[component->type].size;	break;
				case kXtCbPtr:		elementBytes = sizeof(FlaskPtr);					break;
				case kXtCbHandle:	elementBytes = sizeof(FlaskHandle);					break;
			}

			elementBytes *= component->count;

			if (componentBytes + elementBytes < componentBytes)
				return kVfeFailed;

			componentBytes = XtUt::AlignUpper(componentBytes + elementBytes, (XtU32)kXtFlaskAlign);	// [Blur2PC][x64] kXtFlaskAlign is size_t
		}

		componentIndex += record->componentCount;

		if (componentBytes != record->dataBytes)
			return kVfeFailed;

		if (dataOffset + record->dataBytes < dataOffset)
			return kVfeFailed;

		if (dataOffset + record->dataBytes > flaskHeader->data.count)
			return kVfeFailed;

		dataOffset += record->dataBytes;

		if (dataOffset + record->stringBytes < dataOffset)
			return kVfeFailed;

		if (dataOffset + record->stringBytes > flaskHeader->data.count)
			return kVfeFailed;

		// Not possible to validate with encoded strings
		/*if (record->stringBytes > 0)
		{
			if (*(flaskData + dataOffset + record->stringBytes - 1) != '\0')
				return kVfeFailed;
		}*/

		dataOffset += record->stringBytes;
	}

	if (componentIndex != flaskHeader->components.count)
		return kVfeFailed;

	if (dataOffset != flaskHeader->data.count)
		return kVfeFailed;

	return kVfeOkay;
}

bool CreateSchema(XtUt::Block* flask, XtSchema* xtSchema, FiSchema* fiSchema, XtPackOpenParams* params)
{
	FlaskHeader*	flaskHeader		= ReadBlob<FlaskHeader>(	flask, 0, 1						);
	FlaskType*		flaskTypes		= ReadBlob<FlaskType>(		flask, &flaskHeader->types		);
	FlaskBase*		flaskBases		= ReadBlob<FlaskBase>(		flask, &flaskHeader->bases		);
	FlaskField*		flaskFields		= ReadBlob<FlaskField>(		flask, &flaskHeader->fields		);
	char*			flaskStringsEnc	= ReadBlob<char>(			flask, &flaskHeader->strings	);

	if (!Allocate(&fiSchema->types, flaskHeader->types.count, flaskHeader->types.count))
		return false;

	if (!Allocate(&fiSchema->bases, flaskHeader->bases.count, flaskHeader->bases.count))
		return false;

	if (!Allocate(&fiSchema->fields, flaskHeader->fields.count, flaskHeader->fields.count))
		return false;

	XtUt::Block strings;

	if (!Allocate(&strings, flaskHeader->strings.count))
		return false;

	if (flaskHeader->version == 1)
		memcpy(strings.start, (XtU8*)flaskStringsEnc, Capacity(&strings));
	else
		XtFlaskDecodeStrings(strings.start, (XtU8*)flaskStringsEnc, Capacity(&strings));

	const char* flaskStrings = (const char*)strings.start;

	for(size_t i = 0; i < fiSchema->types.count; i++)
	{
		FlaskType*	ft		= &flaskTypes[i]; 
		FiType*		fit		= &fiSchema->types[i];
		XtType*		type	= XtFindTypeByName(xtSchema, flaskStrings + ft->name);

		if ((type != 0) && (type->behaviour == ft->behaviour) && ((type->behaviour == kXtTbFlags) || (type->atom == ft->atom)))
			fit->xt = type;

		if (!fit->xt)
			DoMsg(params, "Flask schema contains type '%s' which does not match any registered type.", flaskStrings + ft->name);

		fit->baseCount	= ft->baseCount;
		fit->fieldCount	= ft->fieldCount;
		fit->size		= ft->size;
	}

	size_t baseIndex	= 0;
	size_t fieldIndex	= 0;

	for(size_t i = 0; i < fiSchema->types.count; i++)
	{
		FlaskType*	ft		= &flaskTypes[i]; 
		FiType*		fit		= &fiSchema->types[i];
		XtType*		type	= fit->xt;

		fit->bases	= fit->baseCount ? &fiSchema->bases[baseIndex] : 0;
		fit->fields	= fit->fieldCount ? &fiSchema->fields[fieldIndex] : 0;

		for(size_t j = 0; j < fit->baseCount; j++)
		{
			FlaskBase*	fb		= &flaskBases[baseIndex + j]; 
			FiBase*		fib		= &fit->bases[j];
			FiType*		fibType	= &fiSchema->types[fb->type];

			fib->type	= fibType;
			fib->offset	= fb->offset;

			if (type && fibType->xt)
			{
				for(size_t k = 0; k < type->baseCount; k++)
				{
					XtBase* base = &type->bases[k];

					if (base->type == fibType->xt)
					{
						fib->xt = base;
						break;
					}
				}

				if (!fib->xt)
					DoMsg(params, "Flask schema claims '%s' is based on '%s', but this association has not been registered.", flaskStrings + ft->name, fibType->xt->name);
			}
		}

		for(size_t j = 0; j < fit->fieldCount; j++)
		{
			FlaskField*	ff			= &flaskFields[fieldIndex + j]; 
			FiField*	fif			= &fit->fields[j];
			FiType*		fifType		= (ff->type != (XtU16)-1) ? &fiSchema->types[ff->type] : 0;
			FlaskType*	flaskType	= (ff->type != (XtU16)-1) ? &flaskTypes[ff->type] : 0;
			XtField*	field		= type ? XtFindFieldByName(type, flaskStrings + ff->name) : 0;

			fif->type	= fifType;
			fif->offset	= ff->offset;

			if ((field != 0) && (field->behaviour == ff->behaviour) && (field->array == !!ff->array))
			{
				if (
						(field->behaviour == kXtFbLabel) ||
						(field->type == fifType->xt) ||
						((field->behaviour == kXtFbFlags) && ((XtType*)field->value == fifType->xt))
					)
				{
					fif->xt = field;
				}
			}

			if (type && !fif->xt)
			{
				if (field)
					DoMsg(params, "Flask schema contains field '%s::%s' which is of a different type to the one present in the application schema. (Flask: %s vs App: %s)", type->name, flaskStrings + ff->name, (flaskTypes ? flaskStrings + flaskType->name : "(no-type)"), field->type->name);
				else
					DoMsg(params, "Flask schema contains field '%s::%s' which is not present in the application schema.", type->name, flaskStrings + ff->name);
			}
		}

		baseIndex	+= fit->baseCount;
		fieldIndex	+= fit->fieldCount;
	}

	return true;
}

struct ImportRecordContext
{
	XtU32			version;
	XtDb*			db;
	XtSchema*		xtSchema;
	FiSchema*		fiSchema;
	XtPack*			pack;
	FlaskRecord*	flaskRecord;
	FlaskComponent*	flaskComponents;
	XtUt::Block		flaskData;
	XtUt::Block		flaskStrings;
	XtRecordHeader*	header;
	XtComponent*	components;
	const char*		strings;
	size_t			stringBytes;

public:
	inline ImportRecordContext(
				XtU32 version_, XtDb* db_, XtSchema* xtSchema_, FiSchema* fiSchema_, XtPack* pack_,
				FlaskRecord* flaskRecord_, FlaskComponent* flaskComponents_, XtU8* data_
			) :
		version(version_),
		db(db_),
		xtSchema(xtSchema_),
		fiSchema(fiSchema_),
		pack(pack_),
		flaskRecord(flaskRecord_),
		flaskComponents(flaskComponents_),
		flaskData(data_, flaskRecord_->dataBytes),
		flaskStrings(data_ + flaskRecord_->dataBytes, flaskRecord_->stringBytes),
		header(0),
		components(0),
		strings(0),
		stringBytes(0)
	{
	}

	inline ~ImportRecordContext()
	{
		Abandon(&flaskData);
		Abandon(&flaskStrings);
	}
};

XtComponentBehaviour AsComponentBehaviour(XtFieldBehaviour behaviour)
{
	switch(behaviour)
	{
		case kXtFbPtr:		return kXtCbPtr;
		case kXtFbHandle:	return kXtCbHandle;
		default:			return kXtCbObject;
	}
}

void FlaskDecodeStruct(ImportRecordContext* ctx, FiType* type, XtU8* flask, XtU8* data);

void FlaskDecodeAtom(ImportRecordContext* ctx, FiType* type, XtU8* flask, XtU8* data)
{
	switch(type->xt->atom)
	{
		case kXtBool:	*(bool*)data	= (*(XtU32*)flask) != 0;	break;
		case kXtI8:		*(XtI8*)data	= *(XtI32*)flask;			break;
		case kXtI16:	*(XtI16*)data	= *(XtI32*)flask;			break;
		case kXtI32:	*(XtI32*)data	= *(XtI32*)flask;			break;
		case kXtI64:	*(XtI64*)data	= *(XtI64*)flask;			break;
		case kXtU8:		*(XtU8*)data	= *(XtU32*)flask;			break;
		case kXtU16:	*(XtU16*)data	= *(XtU32*)flask;			break;
		case kXtU32:	*(XtU32*)data	= *(XtU32*)flask;			break;
		case kXtU64:	*(XtU64*)data	= *(XtU64*)flask;			break;
		case kXtF32:	*(XtF32*)data	= *(XtF32*)flask;			break;
		case kXtF64:	*(XtF64*)data	= *(XtF64*)flask;			break;

		case kXtSz8:
		{
			XtU32 offset = *(XtU32*)flask;

			if (offset < ctx->stringBytes)
				*(const char**)data = ctx->strings + offset;
		}
		break;

		case kXtSz16:
		{
			XtU32 offset = *(XtU32*)flask;

			if (offset < ctx->stringBytes)
				*(const wchar_t**)data = (wchar_t*)(ctx->strings + offset);
		}
		break;

		case kXtLocId:
			((XtLocId*)data)->id = *(XtU32*)flask;
		break;
	}
}

void FlaskDecodeEnum(ImportRecordContext* ctx, FiType* type, XtU8* flask, XtU8* data)
{
	XtU32 index = *(XtU32*)flask;

	if (index < type->fieldCount)
	{
		// TODO: Support enum storage types other than XtU32

		if (type->fields[index].xt)
			*(XtU32*)data = type->fields[index].xt->value;
	}
}

void FlaskDecodeFlags(ImportRecordContext* ctx, FiType* type, XtU8* flask, XtU8* data)
{
	XtU32 flaskFlags	= *(XtU32*)flask;
	XtU32 flags			= 0; // TODO: 64 bit flag support (need a WideFlag flask type)

	for(XtU32 i = 0, j = 1; i < type->fieldCount; i++, j <<= 1)
	{
		if (flaskFlags & j)
		{
			if (type->fields[i].xt)
				flags |= type->fields[i].xt->value;
		}
	}

	switch(type->xt->atom)
	{
		case kXtI8:		*(XtI8*)data	= flags; break;
		case kXtI16:	*(XtI16*)data	= flags; break;
		case kXtI32:	*(XtI32*)data	= flags; break;
		case kXtI64:	*(XtI64*)data	= flags; break;
		case kXtU8:		*(XtU8*)data	= flags; break;
		case kXtU16:	*(XtU16*)data	= flags; break;
		case kXtU32:	*(XtU32*)data	= flags; break;
		case kXtU64:	*(XtU64*)data	= flags; break;
	}
}

void FlaskDecodePtr(ImportRecordContext* ctx, FiType* type, XtU8* flask, XtU8* data)
{
	FlaskPtr*	flaskPtr	= (FlaskPtr*)flask;
	void**		ptr			= (void**)data;

	if (flaskPtr->component < ctx->header->count)
	{
		XtComponent* component = &ctx->components[flaskPtr->component];

		if ((component->behaviour == kXtCbObject) && XtIsBaseOf(type->xt, component->type))
		{
			if (flaskPtr->element < component->count)
			{
				*ptr = (XtU8*)component->data + (component->type->size * flaskPtr->element);
			}
		}
	}
}

void FlaskDecodeHandle(ImportRecordContext* ctx, FiType* type, XtU8* flask, XtU8* data)
{
	FlaskHandle*	flaskHandle	= (FlaskHandle*)flask;
	XtHandle<XtU8>*	handle		= (XtHandle<XtU8>*)data;

	if (flaskHandle->id < ctx->pack->count)
	{
		if (XtRecord* record = ctx->pack->records[flaskHandle->id].record)
		{
			Xt::GetRecordHandle(handle, record, type->xt);
		}
	}
}

void FlaskDecodeArray(ImportRecordContext* ctx, XtFieldBehaviour behaviour, FiType* type, XtU8* flask, XtU8* data)
{
	FlaskArray*		flaskArray	= (FlaskArray*)flask;
	XtArray<XtU8>*	array		= (XtArray<XtU8>*)data;

	if (flaskArray->component < ctx->header->count)
	{
		XtComponent* component = &ctx->components[flaskArray->component];

		if ((component->behaviour == AsComponentBehaviour(behaviour)) && (component->type == type->xt))
		{
			if (
					(flaskArray->element < component->count) &&
					(flaskArray->element + flaskArray->length >= flaskArray->element) &&
					(flaskArray->element + flaskArray->length <= component->count)
				)
			{
				XtU32 elementBytes = 0;

				switch(component->behaviour)
				{
					case kXtCbObject:	elementBytes = component->type->size;	break;
					case kXtCbPtr:		elementBytes = sizeof(void*);			break;
					case kXtCbHandle:	elementBytes = sizeof(XtHandle<XtU8>);	break;
				}

				array->elements	= (XtU8*)component->data + (elementBytes * flaskArray->element);
				array->count	= flaskArray->length;
			}
		}
	}
}

void FlaskDecodeField(ImportRecordContext* ctx,XtFieldBehaviour behaviour, FiType* type, XtU8* flask, XtU8* data)
{
	switch(behaviour)
	{
		case kXtFbAtom:		FlaskDecodeAtom(	ctx, type, flask, data); break;
		case kXtFbEnum:		FlaskDecodeEnum(	ctx, type, flask, data); break;
		case kXtFbFlags:	FlaskDecodeFlags(	ctx, type, flask, data); break;
		case kXtFbStruct:	FlaskDecodeStruct(	ctx, type, flask, data); break;
		case kXtFbPtr:		FlaskDecodePtr(		ctx, type, flask, data); break;
		case kXtFbHandle:	FlaskDecodeHandle(	ctx, type, flask, data); break;
	}
}

void FlaskDecodeStruct(ImportRecordContext* ctx, FiType* type, XtU8* flask, XtU8* data)
{
	for(size_t i = 0; i < type->baseCount; i++)
	{
		FiBase* base = &type->bases[i];

		if (!base->xt)
			continue;

		FlaskDecodeStruct(ctx, base->type, flask + base->offset, data + base->xt->offset);
	}

	for(size_t i = 0; i < type->fieldCount; i++)
	{
		FiField* field = &type->fields[i];

		if (!field->xt)
			continue;

		if (field->xt->array)
			FlaskDecodeArray(ctx, field->xt->behaviour, field->type, flask + field->offset, data + field->xt->offset);
		else
			FlaskDecodeField(ctx, field->xt->behaviour, field->type, flask + field->offset, data + field->xt->offset);
	}
}

void FlaskDecodeObject(ImportRecordContext* ctx, FiType* type, XtU8* flask, XtU8* data)
{
	switch(type->xt->behaviour)
	{
		case kXtTbAtom:		FlaskDecodeAtom(	ctx, type, flask, data); break;
		case kXtTbEnum:		FlaskDecodeEnum(	ctx, type, flask, data); break;
		case kXtTbFlags:	FlaskDecodeFlags(	ctx, type, flask, data); break;
		case kXtTbStruct:	FlaskDecodeStruct(	ctx, type, flask, data); break;
	}
}

void FlaskDecodeElement(ImportRecordContext* ctx, FiType* type, XtComponentBehaviour behaviour, XtU8* flask, XtU8* data)
{
	switch(behaviour)
	{
		case kXtCbObject:	FlaskDecodeObject(	ctx, type, flask, data); break;
		case kXtCbPtr:		FlaskDecodePtr(		ctx, type, flask, data); break;
		case kXtCbHandle:	FlaskDecodeHandle(	ctx, type, flask, data); break;
	}
}

bool FlaskDecodeRecord(ImportRecordContext* ctx, XtPackOpenParams* params)
{
	if (ctx->flaskRecord->componentCount == 0)
		return false;

	// Calculate the memory requirement of the complete record

	size_t totalBytes = 0;

	totalBytes += sizeof(XtComponent) * ctx->flaskRecord->componentCount;
	totalBytes += sizeof(XtRecordHeader);

	totalBytes = XtUt::AlignUpper(totalBytes, Xt::AlignmentOf<XtRecordHeader>());

	size_t dataOffset = totalBytes;

	for(size_t i = 0; i < ctx->flaskRecord->componentCount; i++)
	{
		FlaskComponent*	fc	= &ctx->flaskComponents[i];
		FiType*			fit	= &ctx->fiSchema->types[fc->type];

		if (!fit->xt || (fit->xt->isAbstract && (fc->behaviour == kXtCbObject)))
			continue;

		size_t alignment	= 1;
		size_t size			= 0;

		switch(fc->behaviour)
		{
			case kXtCbObject:
				alignment	= fit->xt->alignment;
				size		= fit->xt->size;
			break;

			case kXtCbPtr:
				alignment	= Xt::AlignmentOf<void*>();
				size		= sizeof(void*);
			break;

			case kXtCbHandle:
				alignment	= Xt::AlignmentOf<XtHandle<XtU8> >();
				size		= sizeof(XtHandle<XtU8>);
			break;
		}

		totalBytes = XtUt::AlignUpper(totalBytes, alignment) + (size * fc->count);
	}

	size_t stringOffset = totalBytes;

	totalBytes += Capacity(&ctx->flaskStrings);

	// Allocate the record as a single slab of memory, then run through that slab of memory
	// allocating portions of it to each component the record contains.

	XtU8* data = new XtU8[totalBytes];

	if (!data)
		return false;

	memset(data, 0, stringOffset);

	if (ctx->version == 1)
		memcpy(data + stringOffset, ctx->flaskStrings.start, Capacity(&ctx->flaskStrings));
	else
		XtFlaskDecodeStrings(data + stringOffset, ctx->flaskStrings.start, Capacity(&ctx->flaskStrings));

	XtComponent* components = (XtComponent*)data;

	for(size_t i = 0; i < ctx->flaskRecord->componentCount; i++)
	{
		FlaskComponent*	fc			= &ctx->flaskComponents[i];
		FiType*			fit			= &ctx->fiSchema->types[fc->type];
		XtComponent*	component	= &components[i];

		if (fit->xt && !(fit->xt->isAbstract && (fc->behaviour == kXtCbObject)))
		{
			size_t alignment	= 1;
			size_t size			= 0;

			switch(fc->behaviour)
			{
				case kXtCbObject:
					alignment	= fit->xt->alignment;
					size		= fit->xt->size;
				break;

				case kXtCbPtr:
					alignment	= Xt::AlignmentOf<void*>();
					size		= sizeof(void*);
				break;

				case kXtCbHandle:
					alignment	= Xt::AlignmentOf<XtHandle<XtU8> >();
					size		= sizeof(XtHandle<XtU8>);
				break;
			}

			dataOffset = XtUt::AlignUpper(dataOffset, alignment);

			component->data			= data + dataOffset;
			component->type			= fit->xt;
			component->count		= fc->count;
			component->behaviour	= (XtComponentBehaviour)fc->behaviour;

			dataOffset += size * component->count;
		}
		else
		{
			//DoMsg(params, "Record %08X contains abstract types", ctx->flaskRecord->ref);

			component->data			= data + dataOffset;
			component->type			= &gXtNullType;
			component->count		= 0;
			component->behaviour	= kXtCbObject;
		}
	}

	// Now we have the component headers setup for the record, run through each component and call the constructor
	// for the type the component represents. Then deserialise the data loaded off disk into the fields within
	// those components.

	XtRecordHeader* header = (XtRecordHeader*)(components[0].data - sizeof(XtRecordHeader));

	header->top		= data;
	header->count	= ctx->flaskRecord->componentCount;
	header->size	= totalBytes;

	ctx->header			= header;
	ctx->components		= components;
	ctx->strings		= (const char*)(data + stringOffset);
	ctx->stringBytes	= Capacity(&ctx->flaskStrings);

	size_t flaskOffset = 0;

	for(size_t i = 0; i < header->count; i++)
	{
		FlaskComponent*	fc			= &ctx->flaskComponents[i];
		FiType*			fit			= &ctx->fiSchema->types[fc->type];
		XtComponent*	component	= &components[i];

		size_t flaskElementBytes = 0;

		switch(fc->behaviour)
		{
			case kXtCbObject:	flaskElementBytes = fit->size;				break;
			case kXtCbPtr:		flaskElementBytes = sizeof(FlaskPtr);		break;
			case kXtCbHandle:	flaskElementBytes = sizeof(FlaskHandle);	break;
		}

		size_t xtElementBytes = 0;

		if (component->type)
		{
			switch(fc->behaviour)
			{
				case kXtCbObject:	xtElementBytes = component->type->size;		break;
				case kXtCbPtr:		xtElementBytes = sizeof(void*);				break;
				case kXtCbHandle:	xtElementBytes = sizeof(XtHandle<XtU8>);	break;
			}
		}

		if (fit->xt && !(fit->xt->isAbstract && (fc->behaviour == kXtCbObject)))
		{
			switch(component->behaviour)
			{
				case kXtCbObject:
				{
					XtU8* end = component->data + (component->count * xtElementBytes);

					for(XtU8* data = component->data; data < end; data += xtElementBytes)
						fit->xt->construct(data);
				}
				break;

				case kXtCbHandle:
				{
					XtU8* end = component->data + (component->count * xtElementBytes);

					for(XtU8* data = component->data; data < end; data += xtElementBytes)
						Xt::Construct<XtHandle<XtU8> >(data);
				}
				break;
			}

			// PERF: Push the element loop into the decoding functions

			XtU8*					flaskData	= ctx->flaskData.start + flaskOffset;
			XtU8*					comData		= component->data;
			XtU32					comCount	= component->count;
			XtComponentBehaviour	comBeh		= (XtComponentBehaviour)component->behaviour;

			for(size_t j = 0; j < comCount; j++, flaskData += flaskElementBytes, comData += xtElementBytes)
				FlaskDecodeElement(ctx, fit, comBeh, flaskData, comData);
		}

		flaskOffset = XtUt::AlignUpper(flaskOffset + (flaskElementBytes * fc->count), kXtFlaskAlign);
	}

	return true;
}

XtRecord* XtRecordAlloc(XtDb* db, XtId id, XtType* type, bool hasStrongRef)
{
	if (!id)
		return 0;

	if (!type || type->isAbstract)
		return 0;

	if (db->count >= db->capacity)
		return 0;

	XtRecord* record = db->free_;

	if (!record)
		return 0;

	db->free_ = (XtRecord*)record->object;

	record->strongRef	= hasStrongRef ? 1 : 0;
	record->weakRef		= 1;
	record->object		= 0;
	record->id			= id;
	record->type		= type;

	return record;
}

void XtCallEventCallback(XtEventType event, XtRecord* record, XtCallbackContext* context, XtType* type, void* object)
{
	if (type->event)
	{
		type->event(event, record, object, context);
	}
	else
	{
		for(size_t i = 0; i < type->baseCount; i++)
		{
			XtBase* base = &type->bases[i];

			XtCallEventCallback(event, record, context, base->type, (XtU8*)object + base->offset);
		}
	}
}

void XtRecordFreeObject(XtDb* db, XtRecord* record)
{
	if (!record || !record->object)
		return;

	XtCallEventCallback(kXtEtDestruct, record, db->context, record->type, record->object);

	XtRecordHeader*	header		= XtGetRecordHeader(record);
	XtComponent*	components	= (XtComponent*)header->top;

	for(size_t i = 0; i < header->count; i++)
	{
		XtComponent* component = &components[i];

		if (component->behaviour == kXtCbObject)
		{
			XtType*	type	= component->type;
			XtU8*	end		= component->data + (component->count * type->size);

			for(XtU8* data = component->data; data < end; data += type->size)
				type->destruct(data);
		}
	}

	delete [] header->top;

	record->object = 0;
}

void XtRecordFree(XtDb* db, XtRecord* record)
{
	XtRecordFreeObject(db, record);

	record->salt++;

	record->strongRef	= 0;
	record->weakRef		= 0;
	record->object		= db->free_;
	record->id			= 0;
	record->type		= &gXtNullType;

	db->free_ = record;
}

XtPack* XtPackCreate(size_t count)
{
	XtPack* pack = new XtPack;

	if (!pack)
		return 0;

	if ((pack->records = new XtPackRef[count]) == 0)
	{
		delete pack;
		return 0;
	}

	pack->count = count;

	return pack;
}

void XtPackDestroy(XtDb* db, XtPack* pack)
{
	if (!pack)
		return;

	size_t dbIndex		= 0;
	size_t packIndex	= 0;
	size_t outIndex		= 0;

	while((dbIndex < db->count) && (packIndex < pack->count))
	{
		XtRecord*	dbRecord	= db->index[dbIndex];
		XtPackRef*	packRef		= &pack->records[packIndex];
		XtRecord*	packRecord	= packRef->record;

		if (!packRecord)
		{
			packIndex++;
			continue;
		}

		if (dbRecord->id < packRecord->id)
		{
			db->index[outIndex++] = dbRecord;
			dbIndex++;
		}
		else
		{
			if (packRef->flags & kXtPackRefFlag_Strong)
			{
				if (--packRecord->strongRef <= 0)
					XtRecordFreeObject(db, packRecord);
			}

			if (--packRecord->weakRef <= 0)
			{
				if (dbRecord->id == packRecord->id)
					dbIndex++;

				XtRecordFree(db, packRecord);
			}

			packIndex++;
		}
	}

	for(; dbIndex < db->count; dbIndex++)
		db->index[outIndex++] = db->index[dbIndex];

	for(; packIndex < pack->count; packIndex++)
	{
		XtPackRef* packRef = &pack->records[packIndex];

		if (packRef->flags & kXtPackRefFlag_Strong)
		{
			if (--packRef->record->strongRef <= 0)
				XtRecordFreeObject(db, packRef->record);
		}

		if (--packRef->record->weakRef <= 0)
			XtRecordFree(db, packRef->record);
	}

	for(size_t i = outIndex; i < db->count; i++)
		db->index[i] = 0;

	db->count = outIndex;

	delete pack;
}

XtPack* XtFlaskImport(XtDb* db, XtSchema* xtSchema, XtUt::Block* flask, XtPackOpenParams* params)
{
	ValidateFlaskError vfe = ValidateFlask(flask);

	if (vfe != kVfeOkay)
	{
		switch(vfe)
		{
			case kVfeHeader:	DoMsg(params, "Flask failed validation, header is not valid."); break;
			case kVfeVersion:	DoMsg(params, "Flask failed validation, unsupported version."); break;
			default:			DoMsg(params, "Flask failed validation, the file may be corrupt."); break;
		}

		return 0;
	}

	FlaskHeader*	flaskHeader		= ReadBlob<FlaskHeader>(	flask, 0, 1						);
	FlaskRef*		flaskRefs		= ReadBlob<FlaskRef>(		flask, &flaskHeader->refs		);

	if (flaskHeader->refs.count <= 0)
	{
		DoMsg(params, "Empty pack file, no records available.");
		return 0;
	}

	FiSchema fiSchema;

	if (!CreateSchema(flask, xtSchema, &fiSchema, params))
	{
		DoMsg(params, "Out of memory during schema allocation.");

		return 0;
	}

	FlaskRecord*	flaskRecords	= ReadBlob<FlaskRecord>(	flask, &flaskHeader->records	);
	FlaskComponent*	flaskComponents	= ReadBlob<FlaskComponent>(	flask, &flaskHeader->components	);
	XtU8*			flaskData		= ReadBlob<XtU8>(			flask, &flaskHeader->data		);

	if (db->count + flaskHeader->refs.count > db->capacity)
	{
		DoMsg(params, "Unable to merge %i record(s) into database, only space for %i record(s) available.",
				flaskHeader->refs.count,
				db->capacity - db->count
			);

		return 0;
	}

	XtPack* pack = XtPackCreate(flaskHeader->refs.count);

	if (!pack)
	{
		DoMsg(params, "Out of memory during pack allocation.");

		return 0;
	}

	size_t dbIndex			= 0;
	size_t flaskIndex		= 0;
	size_t newRecordCount	= 0;

	while((dbIndex < db->count) && (flaskIndex < flaskHeader->refs.count))
	{
		XtRecord*	record		= db->index[dbIndex];
		FlaskRef*	ref			= &flaskRefs[flaskIndex];
		XtType*		type		= fiSchema.types[ref->type].xt;
		bool		isStrongRef	= ref->record != (XtU16)-1;

		if (record->id < ref->id)
		{
			dbIndex++;
			continue;
		}

		if (record->id == ref->id)
		{
			if (isStrongRef)
				XtRecordFreeObject(db, record);

			if (type)
			{
				if (record->type != type)
				{
					record->type = type;
					record->salt++;
				}

				if (isStrongRef)
					record->strongRef++;

				record->weakRef++;
			}
			else
				record = 0;

			dbIndex++;
		}
		else
		{
			if ((record = XtRecordAlloc(db, ref->id, type, isStrongRef)) != 0)
				newRecordCount++;
		}

		pack->records[flaskIndex].flags		= isStrongRef ? kXtPackRefFlag_Strong : 0;
		pack->records[flaskIndex].record	= record;

		flaskIndex++;
	}

	for(; flaskIndex < flaskHeader->refs.count; flaskIndex++)
	{
		FlaskRef*	ref			= &flaskRefs[flaskIndex];
		XtType*		type		= fiSchema.types[ref->type].xt;
		bool		isStrongRef	= ref->record != (XtU16)-1;

		pack->records[flaskIndex].flags		= isStrongRef ? kXtPackRefFlag_Strong : 0;
		pack->records[flaskIndex].record	= XtRecordAlloc(db, ref->id, type, isStrongRef);

		if (pack->records[flaskIndex].record != 0)
			newRecordCount++;
	}

	size_t outIndex = db->count + newRecordCount;

	dbIndex		= db->count;
	flaskIndex	= pack->count;

	while((dbIndex > 0) && (flaskIndex > 0))
	{
		XtRecord* dbRecord		= db->index[dbIndex - 1];
		XtRecord* flaskRecord	= pack->records[flaskIndex - 1].record;
		XtRecord* outRecord;

		if (flaskRecord == 0)
		{
			flaskIndex--;
			continue;
		}

		if (dbRecord->id > flaskRecord->id)
		{
			outRecord = dbRecord;
			dbIndex--;
		}
		else if (dbRecord->id < flaskRecord->id)
		{
			outRecord = flaskRecord;
			flaskIndex--;
		}
		else
		{
			assert(dbRecord == flaskRecord);

			outRecord = dbRecord;
			flaskIndex--;
			dbIndex--;
		}

		assert(outIndex > 0);

		db->index[--outIndex] = outRecord;
	}

	assert(!((dbIndex != 0) && (flaskIndex != 0)));

	outIndex -= dbIndex;

	for(; flaskIndex > 0; flaskIndex--)
	{
		XtRecord* flaskRecord	= pack->records[flaskIndex - 1].record;		

		if (!flaskRecord)
			continue;

		assert(outIndex > 0);

		db->index[--outIndex] = flaskRecord;
	}

	assert(outIndex == 0);

	db->count += newRecordCount;

	size_t componentIndex = 0;

	for(size_t i = 0; i < flaskHeader->records.count; i++)
	{
		FlaskRecord*	flaskRecord	= &flaskRecords[i];
		XtRecord*		dbRecord	= pack->records[flaskRecord->ref].record;

		if (dbRecord)
		{
			ImportRecordContext ctx(flaskHeader->version, db, xtSchema, &fiSchema, pack, flaskRecord, flaskComponents + componentIndex, flaskData);

			if (FlaskDecodeRecord(&ctx, params))
				dbRecord->object = ctx.components[0].data;
		}

		componentIndex	+= flaskRecord->componentCount;
		flaskData		+= flaskRecord->dataBytes + flaskRecord->stringBytes;
	}

	for(size_t i = 0; i < flaskHeader->records.count; i++)
	{
		FlaskRecord*	flaskRecord	= &flaskRecords[i];
		XtRecord*		dbRecord	= pack->records[flaskRecord->ref].record;

		if (dbRecord && dbRecord->object)
			XtCallEventCallback(kXtEtConstruct, dbRecord, db->context, dbRecord->type, dbRecord->object);
	}

	return pack;
}

XtDb* XtDbCreate(XtSchema* schema, size_t maxRecords, XtCallbackContext* context)
{
	if (!schema || !context)
		return 0;

	XtDb* db = new XtDb;

	if (!db)
		return 0;

	if (
			((db->records = new XtRecord[maxRecords]) == 0) ||
			((db->index = new XtRecord*[maxRecords]) == 0)
		)
	{
		delete db;
		return 0;
	}

	db->schema		= schema;
	db->capacity	= maxRecords;
	db->count		= 0;
	db->free_		= db->records;
	db->context		= context;

	for(size_t i = 1; i < maxRecords; i++)
		db->records[i - 1].object = &db->records[i];

	return db;
}

void XtDbDestroy(XtDb* db)
{
	if (!db)
		return;

	for(size_t i = 0; i < db->count; i++)
	{
		XtRecord* record = &db->records[i];

		if (record->id)
			XtRecordFreeObject(db, record);
	}
	
	delete db;
}

XtRecord* XtGetRecord(XtDb* db, XtId id)
{
	if (!db || !id)
		return 0;

	int top = 0;
	int bot = db->count;

	while(top < bot)
	{
		int			mid = (top + bot) / 2;
		XtRecord*	rec = db->index[mid];

		if (id < rec->id)
			bot = mid;
		else if (id > rec->id)
			top = mid + 1;
		else
			return rec;
	}

	return 0;
}

XtPack* XtPackOpen(XtDb* db, void* flask, size_t length, XtPackOpenParams* params)
{
	XtUt::WeakBlock block((XtU8*)flask, length);

	return XtFlaskImport(db, db->schema, &block, params);
}

void XtPackClose(XtDb* db, XtPack* pack)
{
	XtPackDestroy(db, pack);
}

XtPackOpenStatus::XtPackOpenStatus() :
	message(0)
{
}

XtPackOpenParams::XtPackOpenParams() :
	callback(0),
	context(0)
{
}
