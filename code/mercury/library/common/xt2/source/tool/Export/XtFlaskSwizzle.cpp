#include "Common/XtPch.h"
#include "Xt/XtTool.h"
#include "XtFlaskHelpers.h"
#include "Xt/Impl/Grimoire.h"

bool gXtInvertEndianHack = false;

void SwizzleStruct(ExportRecord* record, ExportType* type, int offset);

template<typename Type>
void Swizzle(ExportRecord* record, int offset)
{
	Type* value = (Type*)(record->data.start + offset);

	*value = Swizzle(*value);
}

template<>
void Swizzle<XtF32>(ExportRecord* record, int offset)
{
	Swizzle<XtU32>(record, offset);
}

template<>
void Swizzle<XtF64>(ExportRecord* record, int offset)
{
	Swizzle<XtU64>(record, offset);
}

template<typename StructType, typename MemberType>
void Swizzle(ExportRecord* record, int offset, MemberType (StructType::*member))
{
	Swizzle<MemberType>(record, offset + (int)(XtIntPtr)&(((StructType*)0)->*member));
}

void SwizzleFieldAtom(ExportRecord* record, ExportType* type, int offset)
{
	switch(type->atom)
	{
		case kXtBool:	Swizzle<XtI32>(record, offset); break;
		case kXtI8:		Swizzle<XtI32>(record, offset); break;
		case kXtI16:	Swizzle<XtI32>(record, offset); break;
		case kXtI32:	Swizzle<XtI32>(record, offset); break;
		case kXtI64:	Swizzle<XtI64>(record, offset); break;
		case kXtU8:		Swizzle<XtU32>(record, offset); break;
		case kXtU16:	Swizzle<XtU32>(record, offset); break;
		case kXtU32:	Swizzle<XtU32>(record, offset); break;
		case kXtU64:	Swizzle<XtU64>(record, offset); break;
		case kXtF32:	Swizzle<XtF32>(record, offset); break;
		case kXtF64:	Swizzle<XtF64>(record, offset); break;
		case kXtSz8:	Swizzle<XtU32>(record, offset); break;
		case kXtSz16:	Swizzle<XtU32>(record, offset); break;
		case kXtLocId:	Swizzle<XtU32>(record, offset); break;
	}
}

void SwizzleFieldEnum(ExportRecord* record, ExportType* type, int offset)
{
	Swizzle<XtU32>(record, offset);
}

void SwizzleFieldFlags(ExportRecord* record, ExportType* type, int offset)
{
	Swizzle<XtU32>(record, offset);
}

void SwizzleFieldStruct(ExportRecord* record, ExportType* type, int offset)
{
	SwizzleStruct(record, type, offset);
}

void SwizzleFieldPtr(ExportRecord* record, ExportType* type, int offset)
{
	Swizzle(record, offset, &FlaskPtr::component);
	Swizzle(record, offset, &FlaskPtr::element);
}

void SwizzleFieldHandle(ExportRecord* record, ExportType* type, int offset)
{
	Swizzle(record, offset, &FlaskHandle::id);
}

void SwizzleFieldArray(ExportRecord* record, ExportType* type, int offset)
{
	Swizzle(record, offset, &FlaskArray::component);
	Swizzle(record, offset, &FlaskArray::element);
	Swizzle(record, offset, &FlaskArray::length);
}

void SwizzleFieldWeak(ExportRecord* record, ExportType* type, int offset)
{
	Swizzle(record, offset, &FlaskPtr::component);
	Swizzle(record, offset, &FlaskPtr::element);
}

void SwizzleField(ExportRecord* record, XtFieldBehaviour behaviour, ExportType* type, int offset)
{
	switch(behaviour)
	{
		case kXtFbAtom:			SwizzleFieldAtom(	record, type, offset); break;
		case kXtFbEnum:			SwizzleFieldEnum(	record, type, offset); break;
		case kXtFbFlags:		SwizzleFieldFlags(	record, type, offset); break;
		case kXtFbStruct:		SwizzleFieldStruct(	record, type, offset); break;
		case kXtFbPtr:			SwizzleFieldPtr(	record, type, offset); break;
		case kXtFbHandle:		SwizzleFieldHandle(	record, type, offset); break;
		case kXtFbImplArray:	SwizzleFieldArray(	record, type, offset); break;
		case kXtFbImplWeak:		SwizzleFieldWeak(	record, type, offset); break;
	}
}

void SwizzleStruct(ExportRecord* record, ExportType* type, int offset)
{
	for(size_t i = 0; i < type->bases.count; i++)
	{
		ExportBase* base = type->bases[i];

		SwizzleStruct(record, base->type, offset + base->offset);
	}

	for(size_t i = 0; i < type->fields.count; i++)
	{
		ExportField* field = type->fields[i];

		SwizzleField(record, field->array ? kXtFbImplArray : field->behaviour, field->type, offset + field->offset);
	}
}

void SwizzleElement(ExportRecord* record, ExportComponent* component, int offset)
{
	XtFieldBehaviour behaviour = kXtFbImplEnd;

	switch(component->behaviour)
	{
		case kXtCbObject:
			switch(component->type->behaviour)
			{
				case kXtTbAtom:		behaviour = kXtFbAtom;		break;
				case kXtTbEnum:		behaviour = kXtFbEnum;		break;
				case kXtTbFlags:	behaviour = kXtFbFlags;		break;
				case kXtTbStruct:	behaviour = kXtFbStruct;	break;
			}
		break;

		case kXtCbPtr:		behaviour = kXtFbPtr;		break;
		case kXtCbHandle:	behaviour = kXtFbHandle;	break;
	}

	SwizzleField(record, behaviour, component->type, offset);
}

void SwizzleRecord(ExportRecord* record)
{
	for(size_t i = 0; i < record->components.count; i++)
	{
		ExportComponent*	component		= &record->components[i];
		size_t				elementBytes	= SizeOf(component);

		for(size_t j = 0; j < component->count; j++)
			SwizzleElement(record, component, component->offset + (j * elementBytes));
	}
}