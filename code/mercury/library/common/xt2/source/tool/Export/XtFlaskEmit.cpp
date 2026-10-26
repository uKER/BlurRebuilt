#include "Common/XtPch.h"
#include "Xt/XtTool.h"
#include "XtFlaskHelpers.h"
#include "Xt/Impl/Grimoire.h"

extern void DebugLn(const char*, ...);

CompileId* FindId(CompileContext* ctx, XtId id);

bool EmitStruct(CompileContext* ctx, ExportType* type, Xt::DomElement* elDom, CompileField* fields, int offset);

template<typename Type>
void Emit(CompileContext* ctx, int offset, const Type& value)
{
	size_t written = Write(&ctx->record->data, offset, &value, sizeof(value));

	assert(written == sizeof(value));
}

template<typename StructType, typename MemberType, typename ValueType>
void Emit(CompileContext* ctx, int offset, MemberType (StructType::*member), const ValueType& value)
{
	XT_STATIC_ASSERT(sizeof(MemberType) == sizeof(ValueType)); // MemberType and ValueType must be the same size

	Emit(ctx, offset + (int)(XtIntPtr)&(((StructType*)0)->*member), reinterpret_cast<const MemberType&>(value));
}

bool EmitFieldAtom(CompileContext* ctx, ExportField* field, ExportType* type, Xt::DomElement* dom, CompileField* cf, int offset)
{
	if (type->atom == kXtLocId)
	{
		XtId value = 0;

		if (Xt::DomBlock* block = AsBlock(dom))
		{
			static XtString kRecord	= "record";
			static XtString kId		= "id";

			if (Xt::ParseId(AsText(At(block, kRecord))) != 0)
			{
				value = Xt::ParseId(AsText(At(block, kId)));
			}
		}

		Emit<XtU32>(ctx, offset, value);
	}
	else
	{
		XtString value;

		if (field && field->prep)
			value = field->prep->Eval(dom);
		else
			value = AsText(dom);

		int radix = field ? field->radix : 10;

		switch(type->atom)
		{
			case kXtBool:	Emit<XtI32>(ctx, offset, XtI32(_strtoi64(value, 0, radix)));	break;
			case kXtI8:		Emit<XtI32>(ctx, offset, XtI32(_strtoi64(value, 0, radix)));	break;
			case kXtI16:	Emit<XtI32>(ctx, offset, XtI32(_strtoi64(value, 0, radix)));	break;
			case kXtI32:	Emit<XtI32>(ctx, offset, XtI32(_strtoi64(value, 0, radix)));	break;
			case kXtI64:	Emit<XtI64>(ctx, offset, XtI64(_strtoi64(value, 0, radix)));	break;
			case kXtU8:		Emit<XtU32>(ctx, offset, XtU32(_strtoui64(value, 0, radix)));	break;
			case kXtU16:	Emit<XtU32>(ctx, offset, XtU32(_strtoui64(value, 0, radix)));	break;
			case kXtU32:	Emit<XtU32>(ctx, offset, XtU32(_strtoui64(value, 0, radix)));	break;
			case kXtU64:	Emit<XtU64>(ctx, offset, XtU64(_strtoui64(value, 0, radix)));	break;
			case kXtF32:	Emit<XtF32>(ctx, offset, XtF32(atof(value)));					break;
			case kXtF64:	Emit<XtF64>(ctx, offset, XtF64(atof(value)));					break;

			case kXtSz8:
			{
				const ExportString* str = Add(&ctx->strings, value, kEseAnsi);

				if (!str)
					return false;

				Emit<XtU32>(ctx, offset, str->offset);
			}
			break;

			case kXtSz16:
			{
				const ExportString* str = Add(&ctx->strings, value, kEseUnicode);

				if (!str)
					return false;

				Emit<XtU32>(ctx, offset, str->offset);
			}
			break;
		}
	}

	return true;
}

bool EmitFieldEnum(CompileContext* ctx, ExportType* type, Xt::DomElement* dom, CompileField* cf, int offset)
{
	const char* value = AsText(dom);

	if (ExportField* enumValue = FindFieldByName(type, value))
		Emit<XtU32>(ctx, offset, XtU32(enumValue->index));

	return true;
}

bool EmitFieldFlags(CompileContext* ctx, ExportType* type, Xt::DomElement* dom, CompileField* cf, int offset)
{
	if (Xt::DomBlock* fieldDom = AsBlock(dom))
	{
		XtU32 flags = 0;

		for(size_t j = 0; j < type->fields.count; j++)
		{
			ExportField* flag = type->fields[j];

			if (atoi(AsText(At(fieldDom, flag->name))))
				flags |= 1 << j;
		}

		Emit<XtU32>(ctx, offset, flags);
	}

	return true;
}

bool EmitFieldStruct(CompileContext* ctx, ExportType* type, Xt::DomElement* dom, CompileField* cf, int offset)
{
	return EmitStruct(ctx, type, dom, cf, offset);
}

bool EmitFieldPtr(CompileContext* ctx, ExportType* type, Xt::DomElement* dom, CompileField* cf, int offset)
{
	FlaskPtr ptr = { 0xFFFF, 0xFFFF };

	if (cf && cf->element)
	{
		ptr.component	= cf->element->component->index;
		ptr.element		= cf->element->index;
	}

	Emit(ctx, offset, &FlaskPtr::component,	ptr.component);
	Emit(ctx, offset, &FlaskPtr::element,	ptr.element);

	return true;
}

bool EmitFieldHandle(CompileContext* ctx, ExportType* type, Xt::DomElement* dom, CompileField* cf, int offset)
{
	FlaskHandle hdl = { 0 };

	hdl.id = XtU32(Xt::ParseId(AsText(dom)));

	if (hdl.id != 0)
	{
		ExportHandleFixup* fixup = AppendN(&ctx->record->handleFixups, 1);

		if (!fixup)
			return false;

		fixup->offset = offset;
	}
	else
		hdl.id = (XtId)-1;

	Emit(ctx, offset, &FlaskHandle::id, hdl.id);

	return true;
}

bool EmitFieldArray(CompileContext* ctx, ExportType* type, Xt::DomElement* dom, CompileField* cf, int offset)
{
	FlaskArray arr = { 0xFFFF, 0xFFFF, 0x00000000 };

	if (cf && cf->element)
	{
		arr.component	= cf->element->component->index;
		arr.element		= cf->element->index;

		if (Xt::DomArray* arrayDom = AsArray(dom))
			arr.length = Count(arrayDom);
	}

	Emit(ctx, offset, &FlaskArray::component,	arr.component);
	Emit(ctx, offset, &FlaskArray::element,		arr.element);
	Emit(ctx, offset, &FlaskArray::length,		arr.length);

	return true;
}

bool EmitFieldWeak(CompileContext* ctx, ExportType* type, Xt::DomElement* dom, CompileField* cf, int offset)
{
	FlaskPtr ptr = { 0xFFFF, 0xFFFF };

	if (CompileId* cid = FindId(ctx, AsId(dom)))
	{
		ptr.component	= cid->element->component->index;
		ptr.element		= cid->element->index;
	}

	Emit(ctx, offset, &FlaskPtr::component,	ptr.component);
	Emit(ctx, offset, &FlaskPtr::element,	ptr.element);

	return true;
}

bool EmitField(CompileContext* ctx, XtFieldBehaviour behaviour, ExportField* field, ExportType* type, Xt::DomElement* dom, CompileField* cf, int offset)
{
	switch(behaviour)
	{
		case kXtFbAtom:			return EmitFieldAtom(	ctx, field, type, dom, cf, offset);
		case kXtFbEnum:			return EmitFieldEnum(	ctx, type, dom, cf, offset);
		case kXtFbFlags:		return EmitFieldFlags(	ctx, type, dom, cf, offset);
		case kXtFbStruct:		return EmitFieldStruct(	ctx, type, dom, cf, offset);
		case kXtFbPtr:			return EmitFieldPtr(	ctx, type, dom, cf, offset);
		case kXtFbHandle:		return EmitFieldHandle(	ctx, type, dom, cf, offset);
		case kXtFbImplArray:	return EmitFieldArray(	ctx, type, dom, cf, offset);
		case kXtFbImplWeak:		return EmitFieldWeak(	ctx, type, dom, cf, offset);

		default:
			assert(0);
		return false;
	}
}

bool EmitStruct(CompileContext* ctx, ExportType* type, Xt::DomElement* elDom, CompileField* fields, int offset)
{
	Xt::DomBlock* dom = AsBlock(elDom);

	if (type->bases.count > 0)
	{
		for(size_t i = 0; i < type->bases.count; i++)
		{
			ExportBase* base = type->bases[i];

			if (!EmitStruct(ctx, base->type, dom, fields + base->fieldOffset, offset + base->offset))
				return false;
		}
	}

	for(size_t i = 0; i < type->fields.count; i++)
	{
		ExportField* field = type->fields[i];

		if (!EmitField(
				ctx,
				field->array ? kXtFbImplArray : field->behaviour,
				field,
				field->type,
				dom ? At(dom, field->name) : 0,
				fields + field->fieldIndex,
				offset + field->offset
			))
		{
			return false;
		}
	}

	return true;
}

bool EmitElement(CompileContext* ctx, CompileComponent* component, CompileElement* element, int offset)
{
	XtFieldBehaviour	behaviour	= kXtFbImplEnd;
	Xt::DomElement*		dom			= element->dom;

	switch(component->behaviour)
	{
		case kXtCbObject:
			switch(component->type->behaviour)
			{
				case kXtTbAtom:		behaviour = kXtFbAtom;	break;
				case kXtTbEnum:		behaviour = kXtFbEnum;	break;
				case kXtTbFlags:	behaviour = kXtFbFlags;	break;

				case kXtTbStruct:
					behaviour = kXtFbStruct;

					if (Xt::DomComponent* domCom = AsComponent(dom))
						dom = &domCom->data;
					else
						dom = 0;
				break;

				default: assert(0); return false;
			}
		break;

		case kXtCbPtr:
			behaviour = (element->behaviour == kCeWeak) ? kXtFbImplWeak : kXtFbPtr;
		break;

		case kXtCbHandle:
			behaviour = kXtFbHandle;
		break;

		default: assert(0); return false;
	}

	return EmitField(ctx, behaviour, 0, component->type, dom, element->fields.elements, offset);
}

bool EmitRecord(CompileContext* ctx)
{
	for(size_t i = 0; i < ctx->components.count; i++)
	{
		CompileComponent*	component		= ctx->components[i];
		size_t				elementBytes	= SizeOf(component);

		for(size_t j = 0; j < component->elements.count; j++)
		{
			if (!EmitElement(ctx, component, component->elements[j], component->offset + (j * elementBytes)))
				return false;
		}
	}

	return true;
}

bool EmitComponents(CompileContext* ctx)
{
	if (!Allocate(&ctx->record->components, ctx->components.count, ctx->components.count))
		return false;

	for(size_t i = 0; i < ctx->components.count; i++)
	{
		CompileComponent*	src	= ctx->components[i];
		ExportComponent*	dst	= &ctx->record->components[i];

		dst->type		= src->type;
		dst->count		= src->elements.count;
		dst->behaviour	= src->behaviour;
		dst->offset		= src->offset;
	}

	return true;
}