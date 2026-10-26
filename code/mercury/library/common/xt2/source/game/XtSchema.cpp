#include "Common/XtPch.h"
#include "XtGameImpl.h"
#include <Core/BizAssert.h>

bool XtIsNameStart(char c)
{
	return (((c >= 'a') && (c <= 'z')) || ((c >= 'A') && (c <= 'Z')) || (c == '_'));
}

bool XtIsNameBody(char c)
{
	return (((c >= 'a') && (c <= 'z')) || ((c >= 'A') && (c <= 'Z')) || ((c >= '0') && (c <= '9')) || (c == '_'));
}

bool XtIsValidName(const char* text, bool system)
{
	if (!text || !*text)
		return false;

	if (!XtIsNameStart(*text))
		return false;

	for(text++; *text != '\0'; text++)
	{
		if (!XtIsNameBody(*text))
			return false;

		if (!system && (*text == '_') && (*(text - 1) == '_'))
			return false;
	}

	return true;
}

XtSchema* XtCreateSchema(int typeCount, int baseCount, int fieldCount, int stringsLength)
{
	XtSchema* schema = (XtSchema*)new XtU8[
			sizeof(XtSchema) +
			(typeCount * sizeof(XtType)) +
			(baseCount * sizeof(XtBase)) + 
			(fieldCount * sizeof(XtField)) +
			stringsLength
		];

	if (!schema)
		return 0;

	schema->typeCount	= typeCount;
	schema->baseCount	= baseCount;
	schema->fieldCount	= fieldCount;

	schema->types	= (XtType*)(schema + 1);
	schema->bases	= (XtBase*)(schema->types + typeCount);
	schema->fields	= (XtField*)(schema->bases + baseCount);
	schema->strings	= (XtU8*)(schema->fields + fieldCount);

	schema->endian	= XtGetEndian();

	return schema;
}

void XtDestroySchema(XtSchema* schema)
{
	delete [] (XtU8*)schema;
}

XtSchema* XtGenerateApplicationSchema()
{
	bool failed = false;

	// Register all application types and dependencies

	void XtLibrary_Application();
	XtLibrary_Application();

	// Determine behaviour for each type

	for(int i = 0; i < gXt2.typeCount; i++)
	{
		Xt::AnnotationType&	annType	= *gXt2.types[i];

		bool isAtom		= false;
		bool isEnum		= false;
		bool isFlags	= false;
		bool isStruct	= false;

		for(int j = 0; annType.fields[j].behaviour != kXtFbImplEnd; j++)
		{
			Xt::AnnotationField& annField = annType.fields[j];

			switch(annField.behaviour)
			{
				// XT_FIELD(...)
				// XT_FIELD(XtArray<...>)
				// XT_FIELD(XtEnum<...>)
				// XT_FIELD(XtFlag<...>)
				// XT_BASE(...)
				// XT_CALLBACK(...)

				case kXtFbAtom:
				case kXtFbEnum:
				case kXtFbFlags:
				case kXtFbStruct:
				case kXtFbPtr:
				case kXtFbHandle:
				case kXtFbImplInfer:
				case kXtFbImplBase:
				case kXtFbImplCallback:
					isStruct = true;
				break;

				// XT_OPTION(...)
				// XT_FLAG(...)

				case kXtFbImplEnumLabel:
					isEnum				= true;
					annField.behaviour	= kXtFbLabel;
				break;

				case kXtFbImplFlagLabel:
					isFlags				= true;
					annField.behaviour	= kXtFbLabel;
				break;

				// XT_ATOM(...)

				case kXtFbImplAtom:
					isAtom = true;
				break;

				// XT_FLAG_STORAGE(...)

				case kXtFbImplFlagStorage:
					isFlags = true;
				break;

				default:
					assert(0);
				break;
			}
		}

		// Determine the behaviour of this XtType, verify the type hasn't specified multiple behaviours

		if (isAtom && !isEnum && !isFlags && !isStruct)
			annType.behaviour = kXtTbAtom;
		else if (!isAtom && isEnum && !isFlags && !isStruct)
			annType.behaviour = kXtTbEnum;
		else if (!isAtom && !isEnum && isFlags && !isStruct)
			annType.behaviour = kXtTbFlags;
		else if (!isAtom && !isEnum && !isFlags && isStruct)
			annType.behaviour = kXtTbStruct;
		else if (!isAtom && !isEnum && !isFlags && !isStruct)
			annType.behaviour = kXtTbStruct;
		else
		{
			DebugLn("%s(%i): '%s' contains mixed annotation.", annType.debugFile, annType.debugLine, annType.name ? annType.name : "");
			failed = true;
		}
	}

	// Tally total number of types, bases and fields

	int totalTypeCount	= gXt2.typeCount;
	int totalBaseCount	= 0;
	int totalFieldCount	= 0;

	for(int i = 0; i < totalTypeCount; i++)
	{
		Xt::AnnotationType& annType = *gXt2.types[i];

		for(int j = 0; annType.fields[j].behaviour != kXtFbImplEnd; j++)
		{
			Xt::AnnotationField& annField = annType.fields[j];

			if (annField.behaviour == kXtFbImplInfer)
			{
				if (annField.type)
				{
					switch(annField.type->behaviour)
					{
						case kXtTbAtom:		annField.behaviour = kXtFbAtom;		break;
						case kXtTbEnum:		annField.behaviour = kXtFbEnum;		break;
						case kXtTbFlags:	annField.behaviour = kXtFbFlags;	break;
						case kXtTbStruct:	annField.behaviour = kXtFbStruct;	break;
					}
				}
				else
					annField.behaviour = kXtFbStruct;
			}

			switch(annField.behaviour)
			{
				case kXtFbImplBase:
					totalBaseCount++;
				break;

				case kXtFbAtom:
				case kXtFbEnum:
				case kXtFbFlags:
				case kXtFbStruct:
				case kXtFbPtr:
				case kXtFbHandle:
				case kXtFbLabel:
					totalFieldCount++;
				break;

				case kXtFbImplAtom:
				case kXtFbImplCallback:
				case kXtFbImplFlagStorage:
				break;

				default:
					assert(0);
				break;
			}
		}
	}

	// Setup schema

	XtSchema* schema = XtCreateSchema(totalTypeCount, totalBaseCount, totalFieldCount, 0);

	if (!schema)
	{
		DebugLn("Out of memory allocating application XtSchema");
		return 0;
	}

	XtType*		types	= schema->types;
	XtBase*		bases	= schema->bases;
	XtField*	fields	= schema->fields;

	XtBase*		basePtr		= bases;
	XtField*	fieldPtr	= fields;

	// Set Xt::AnnotationType<Type>::type to the relevant XtType in the new schema so XtGetType<Type>() works

	for(int i = 0; i < totalTypeCount; i++)
	{
		Xt::AnnotationType&	annType	= *gXt2.types[i];
		XtType&				type	= types[i];

		if (annType.type != 0)
		{
			DebugLn("%s(%i): %s has been registered multiple times.", annType.debugFile, annType.debugLine, annType.name);
			failed = true;
		}

		annType.type = &type;
	}

	// Sort out atom types

	for(int i = 0; i < totalTypeCount; i++)
	{
		Xt::AnnotationType&	annType	= *gXt2.types[i];
		XtType&				type	= types[i];

		// Copy Xt::AnnotationType into XtType for new schema

		type.name		= annType.name;
		type.size		= annType.size;
		type.isAbstract	= annType.isAbstract;
		type.behaviour	= annType.behaviour;
		type.atom		= kXtNotAtom;
		type.alignment	= annType.alignment;
		type.construct	= annType.construct;
		type.event		= 0;
		type.destruct	= annType.destruct;

		if (annType.behaviour == kXtTbAtom)
		{
			const char* safeTypeName = type.name ? type.name : "";

			XtAtomType	atom		= kXtNotAtom;
			int			debugLine	= 0;

			for(int j = 0; annType.fields[j].behaviour != kXtFbImplEnd; j++)
			{
				Xt::AnnotationField& annField = annType.fields[j];

				if (annField.behaviour == kXtFbImplAtom)
				{
					atom		= (XtAtomType)annField.value;
					debugLine	= annField.debugLine;
					break;
				}
			}

			if (atom != kXtNotAtom)
			{
				type.atom = atom;
			}
			else
			{
				DebugLn("%s(%i): %s has no atom type specified.", annType.debugFile, annType.debugLine, safeTypeName);
				failed = true;
			}
		}
	}

	// Sort out flag atom type

	for(int i = 0; i < totalTypeCount; i++)
	{
		Xt::AnnotationType&	annType	= *gXt2.types[i];
		XtType&				type	= types[i];

		if (annType.behaviour == kXtTbFlags)
		{
			const char* safeTypeName = type.name ? type.name : "";

			XtAtomType	atom		= kXtNotAtom;
			int			debugLine	= 0;

			for(int j = 0; annType.fields[j].behaviour != kXtFbImplEnd; j++)
			{
				Xt::AnnotationField& annField = annType.fields[j];

				if (annField.behaviour == kXtFbImplFlagStorage)
				{
					atom		= annField.type->type ? annField.type->type->atom : kXtNotAtom;
					debugLine	= annField.debugLine;
					break;
				}
			}

			if (atom == kXtNotAtom)
			{
				DebugLn("%s(%i): %s has no flag storage type specified.", annType.debugFile, annType.debugLine, safeTypeName);
				failed = true;
			}
			else
			{
				switch(atom)
				{
					case kXtI8:
					case kXtI16:
					case kXtI32:
					case kXtI64:
					case kXtU8:
					case kXtU16:
					case kXtU32:
					case kXtU64:
						type.atom = atom;
					break;

					default:
						DebugLn("%s(%i): %s has an illegal flag storage type specified.", annType.debugFile, debugLine, safeTypeName);
						failed = true;
					break;
				}
			}
		}
	}

	for(int i = 0; i < totalTypeCount; i++)
	{
		Xt::AnnotationType&	annType	= *gXt2.types[i];
		XtType&				type	= types[i];

		int baseCount	= 0;
		int fieldCount	= 0;

		const char* safeTypeName = type.name ? type.name : "";

		if (!XtIsValidName(type.name, false))
		{
			DebugLn("%s(%i): '%s' contains illegal characters.", annType.debugFile, annType.debugLine, safeTypeName);
			failed = true;
		}

		for(int j = 0; annType.fields[j].behaviour != kXtFbImplEnd; j++)
		{
			Xt::AnnotationField& annField = annType.fields[j];

			const char* safeFieldName = annField.name ? annField.name : "";

			switch(annField.behaviour)
			{
				// XT_BASE(...)

				case kXtFbImplBase:
				{
					XtBase& base = basePtr[baseCount++];

					base.type	= annField.type->type;
					base.offset	= annField.offset;

					if (base.type == 0)
					{
						DebugLn("%s(%i): '%s' is based on type '%s' which has not been registered.", annType.debugFile, annField.debugLine, safeTypeName, annField.type->name ? annField.type->name : "");
						failed = true;
					}

					if (annField.type->behaviour != kXtTbStruct)
					{
						DebugLn("%s(%i): '%s' is based on type '%s' which is not a struct.", annType.debugFile, annField.debugLine, safeTypeName, annField.type->name ? annField.type->name : "");
						failed = true;
					}
				}
				break;

				// XT_FIELD(...)
				// XT_OPTION(...)
				// XT_FLAG(...)

				case kXtFbAtom:
				case kXtFbEnum:
				case kXtFbFlags:
				case kXtFbStruct:
				case kXtFbPtr:
				case kXtFbHandle:
				case kXtFbLabel:
				{
					XtField& field = fieldPtr[fieldCount++];

					field.name		= annField.name;
					field.type		= annField.type ? annField.type->type : 0;
					field.offset	= annField.offset;
					field.array		= annField.array;
					field.behaviour	= (XtFieldBehaviour)annField.behaviour;

					if (annField.behaviour == kXtFbFlags)
						field.value = annField.value ? (XtIntPtr)((Xt::AnnotationType*)annField.value)->type : 0;
					else
						field.value = annField.value;

					if (!XtIsValidName(field.name, false))
					{
						DebugLn("%s(%i): '%s::%s' contains illegal characters.", annType.debugFile, annField.debugLine, safeTypeName, safeFieldName);
						failed = true;
					}

					// Check that all fields which require a type have one

					switch(field.behaviour)
					{
						case kXtFbAtom:
						case kXtFbEnum:
						case kXtFbFlags:
						case kXtFbStruct:
						case kXtFbPtr:
						case kXtFbHandle:
							if (field.type == 0)
							{
								DebugLn("%s(%i): '%s::%s' declared as type '%s' which has not been registered.", annType.debugFile, annField.debugLine, safeTypeName, safeFieldName, annField.type->name ? annField.type->name : "");
								failed = true;
							}
						break;

						case kXtFbLabel:
							if (field.type != 0)
							{
								DebugLn("%s(%i): '%s::%s' should not have a type.", annType.debugFile, annField.debugLine, safeTypeName, safeFieldName);
								failed = true;
							}
						break;
					}

					// Check for field storage type mismatch

					bool checkAuxType = true;

					if (field.type)
					{
						switch(field.behaviour)
						{
							case kXtFbAtom:
								if (field.type->behaviour != kXtTbAtom)
								{
									DebugLn("%s(%i): '%s::%s' declared as type '%s' which is not an atom.", annType.debugFile, annField.debugLine, safeTypeName, safeFieldName, annField.type->name ? annField.type->name : "");
									failed = true;
								}
							break;

							case kXtFbEnum:
								if (field.type->behaviour != kXtTbEnum)
								{
									DebugLn("%s(%i): '%s::%s' declared as type '%s' which is not an enum.", annType.debugFile, annField.debugLine, safeTypeName, safeFieldName, annField.type->name ? annField.type->name : "");
									failed = true;
								}
							break;

							case kXtFbFlags:
								if (field.type->behaviour == kXtTbFlags)
								{
									DebugLn("%s(%i): '%s::%s' declared as '%s' without using XtFlags<...> syntax", annType.debugFile, annField.debugLine, safeTypeName, safeFieldName, annField.type->name ? annField.type->name : "");
									failed			= true;
									checkAuxType	= false;
								}
								else if (field.type->behaviour != kXtTbAtom)
								{
									DebugLn("%s(%i): '%s::%s' declared with storage type '%s' which is not an atom.", annType.debugFile, annField.debugLine, safeTypeName, safeFieldName, annField.type->name ? annField.type->name : "");
									failed = true;
								}
							break;

							case kXtFbStruct:
							case kXtFbPtr:
							case kXtFbHandle:
								if (field.type->behaviour != kXtTbStruct)
								{
									DebugLn("%s(%i): '%s::%s' declared as type '%s' which is not a struct.", annType.debugFile, annField.debugLine, safeTypeName, safeFieldName, annField.type->name ? annField.type->name : "");
									failed = true;
								}
							break;
						}
					}

					// Check for field auxiliary type mismatch

					if (checkAuxType)
					{
						switch(field.behaviour)
						{
							case kXtFbFlags:
								if (field.value == 0)
								{
									DebugLn("%s(%i): '%s::%s' is declared with flag type '%s' which has not been registered.", annType.debugFile, annField.debugLine, safeTypeName, safeFieldName, annField.value ? ((Xt::AnnotationType*)annField.value)->name : "");
									failed = true;
								}
								else if (((XtType*)field.value)->behaviour != kXtTbFlags)
								{
									DebugLn("%s(%i): '%s::%s' is declared with flag type '%s' which is not a flags list.", annType.debugFile, annField.debugLine, safeTypeName, safeFieldName, annField.type->name ? annField.type->name : "");
									failed = true;
								}
								else if (((XtType*)field.value)->atom != field.type->atom)
								{
									DebugLn("%s(%i): '%s::%s' storage type does not match storage type of flag list '%s'.", annType.debugFile, annField.debugLine, safeTypeName, safeFieldName, annField.type->name ? annField.type->name : "");
									failed = true;
								}
							break;
						}
					}
				}
				break;

				// XT_CALLBACK(...)

				case kXtFbImplCallback:
					type.event = (XtEventCallback)annField.value;
				break;

				// XT_ATOM(...)
				// XT_FLAG_STORAGE(...)

				case kXtFbImplAtom:
				case kXtFbImplFlagStorage:
				break;

				default:
					assert(0);
				break;
			}

			// Check for duplicate fields

			switch(annField.behaviour)
			{
				case kXtFbAtom:
				case kXtFbEnum:
				case kXtFbFlags:
				case kXtFbStruct:
				case kXtFbPtr:
				case kXtFbHandle:
				case kXtFbLabel:
					for(int k = 0; k < j; k++)
					{
						Xt::AnnotationField& annOtherField = annType.fields[k];

						const char* safeOtherFieldName = annOtherField.name ? annOtherField.name : "";

						switch(annOtherField.behaviour)
						{
							case kXtFbAtom:
							case kXtFbEnum:
							case kXtFbFlags:
							case kXtFbStruct:
							case kXtFbPtr:
							case kXtFbHandle:
							case kXtFbLabel:
								if (strcmp(annField.name, annOtherField.name) == 0)
								{
									DebugLn("%s(%i): '%s::%s' duplicate field name.", annType.debugFile, annField.debugLine, safeTypeName, safeOtherFieldName);
									DebugLn("%s(%i): ... original field.", annType.debugFile, annOtherField.debugLine);
									failed = true;
								}
							break;
						}

						switch(annOtherField.behaviour)
						{
							case kXtFbAtom:
							case kXtFbEnum:
							case kXtFbFlags:
							case kXtFbStruct:
							case kXtFbPtr:
							case kXtFbHandle:
								if (annField.offset == annOtherField.offset)
								{
									DebugLn("%s(%i): '%s::%s' duplicate field.", annType.debugFile, annField.debugLine, safeTypeName, safeOtherFieldName);
									DebugLn("%s(%i): ... original field '%s::%s'.", annType.debugFile, annOtherField.debugLine, safeTypeName, safeFieldName);
									failed = true;
								}
							break;
						}
					}
				break;
			}
		}

		// Move past bases and fields claimed by this type in the new schema lists

		type.bases		= (baseCount > 0) ? basePtr : 0;
		type.fields		= (fieldCount > 0) ? fieldPtr : 0;
		type.fieldCount	= fieldCount;
		type.baseCount	= baseCount;

		basePtr		+= baseCount;
		fieldPtr	+= fieldCount;
	}

	if (!failed)
		return schema;

	// The new schema didn't validate, so clear out Xt::AnnotationType<Type>::type to prevent
	// XtGetType<Type>() from returning rubbish

	for(int i = 0; i < totalTypeCount; i++)
		gXt2.types[i]->type = 0;

	XtDestroySchema(schema);

	return 0;
}

//
// Helpers
//

XtType* XtFindTypeByName(const XtSchema* schema, const char* name)
{
	for(int i = 0; i < schema->typeCount; i++)
	{
		XtType& type = schema->types[i];

		if (strcmp(type.name, name) == 0)
			return &type;
	}

	return 0;
}

XtField* XtFindFieldByName(const XtType* type, const char* name)
{
	for(XtU32 i = 0; i < type->fieldCount; i++)
	{
		XtField& field = type->fields[i];

		if (strcmp(field.name, name) == 0)
			return &field;
	}

	return 0;
}

bool XtIsBaseOf(const XtType* base, const XtType* derived)
{
	if (base == derived)
		return true;

	for(XtU32 i = 0; i < derived->baseCount; i++)
		if (XtIsBaseOf(base, derived->bases[i].type))
			return true;

	return false;
}

void XtCastImpl(const XtHandle<XtU8>* handle, XtHandle<XtU8>* result, XtType* fromType, XtType* toType, int offset)
{
	if (fromType == toType)
	{
		result->_record	= handle->_record;
		result->_salt	= handle->_salt;
		result->_offset	= offset;
	}
	else
	{
		for(XtU32 i = 0; i < fromType->baseCount; i++)
			XtCastImpl(handle, result, fromType->bases[i].type, toType, offset + fromType->bases[i].offset);
	}
}

XtHandle<XtU8> Xt::XtCastImpl(const XtHandle<XtU8>* handle, XtType* fromType, XtType* toType)
{
	XtHandle<XtU8> result;

	XtCastImpl(handle, &result, fromType, toType, 0);

	return result;
}

void Xt::GetRecordHandle(XtHandle<XtU8>* result, XtRecord* record, XtType* type)
{
	if (record)
	{
		XtHandle<XtU8> handle(record, record->salt, 0);

		XtCastImpl(&handle, result, record->type, type, 0);
	}
}

XtRecordHeader* XtGetRecordHeader(const XtRecord* record)
{
	return (XtRecordHeader*)((XtU8*)record->object - sizeof(XtRecordHeader));
}

XtType* XtGetTypeByPtr(XtRecord* record, void* ptr)
{
	if (!ptr || !record || !record->object)
		return 0;

	XtRecordHeader*	header		= XtGetRecordHeader(record);
	XtComponent*	components	= (XtComponent*)header->top;

	for(size_t i = 0; i < header->count; i++)
	{
		XtComponent* component = &components[i];

		// TODO: Consider comparing [0].data and [1].data rather than calculating range, also as .data should always be increasing
		// it should be possible to binary search this. Might need to add a sentinel "top" value though.

		switch(component->behaviour)
		{
			case kXtCbObject:
				if ((ptr >= component->data) && (ptr < (component->data + (component->type->size * component->count))))
					return component->type;
			break;

			case kXtCbPtr:
				if ((ptr >= component->data) && (ptr < (component->data + (sizeof(XtIntPtr) * component->count))))
					return component->type;
			break;

			case kXtCbHandle:
				if ((ptr >= component->data) && (ptr < (component->data + (sizeof(XtHandle<XtU8>) * component->count))))
					return component->type;
			break;
		}
	}

	return 0;
}

XtU8* XtGetRecordBaseAddr(const XtRecord* record)
{
	return record ? (XtU8*)record->object : 0;
}

XtU32 XtGetRecordSize(const XtRecord* record)
{
	if (!record)
		return 0;

	const XtRecordHeader*	header		= XtGetRecordHeader(record);
	XtComponent*			components	= (XtComponent*)header->top;
	XtU8*					baseAddr	= (XtU8*)record->object;

	if (header->count <= 0)
		return 0;

	XtComponent* component = &components[header->count - 1];

	switch(component->behaviour)
	{
		case kXtCbObject:	return (component->data + (component->type->size * component->count)) - baseAddr;
		case kXtCbPtr:		return (component->data + (sizeof(XtIntPtr) * component->count)) - baseAddr;
		case kXtCbHandle:	return (component->data + (sizeof(XtHandle<XtU8>) * component->count)) - baseAddr;
	}

	return 0;
}

XtU32 XtGetTypeFixupCount(XtType* type)
{
	if (type->behaviour != kXtTbStruct)
		return 0;

	XtU32 count = 0;

	for(size_t i = 0; i < type->baseCount; i++)
		count += XtGetTypeFixupCount(type->bases[i].type);

	for(size_t i = 0; i < type->fieldCount; i++)
	{
		XtField* field = &type->fields[i];

		if (field->array)
			count++;
		else if (field->behaviour == kXtFbPtr)
			count++;
		else if ((field->behaviour == kXtFbAtom) && ((field->type->atom == kXtSz8) || (field->type->atom == kXtSz16)))
			count++;
		else if (field->behaviour == kXtFbStruct)
			count += XtGetTypeFixupCount(field->type);
	}

	return count;
}

XtU32 XtGetRecordFixupCount(const XtRecord* record)
{
	if (!record)
		return 0;

	XtRecordHeader*	header		= XtGetRecordHeader(record);
	XtComponent*	components	= (XtComponent*)header->top;

	XtU32 count = 0;

	for(size_t i = 0; i < header->count; i++)
	{
		XtComponent* component = &components[i];

		switch(component->behaviour)
		{
			case kXtCbObject:	count += XtGetTypeFixupCount(component->type) * component->count;	break;
			case kXtCbPtr:		count += component->count;											break;
			case kXtCbHandle:	break;
		}
	}

	return count;
}

void XtGenerateTypeFixups(XtType* type, XtU32 offset, XtU32*& fixup)
{
	if (type->behaviour != kXtTbStruct)
		return;

	for(size_t i = 0; i < type->baseCount; i++)
		XtGenerateTypeFixups(type->bases[i].type, offset + type->bases[i].offset, fixup);

	for(size_t i = 0; i < type->fieldCount; i++)
	{
		XtField* field = &type->fields[i];

		if (field->array)
			*fixup++ = offset + field->offset;
		else if (field->behaviour == kXtFbPtr)
			*fixup++ = offset + field->offset;
		else if ((field->behaviour == kXtFbAtom) && ((field->type->atom == kXtSz8) || (field->type->atom == kXtSz16)))
			*fixup++ = offset + field->offset;
		else if (field->behaviour == kXtFbStruct)
			XtGenerateTypeFixups(field->type, offset + field->offset, fixup);
	}
}

void XtGenerateRecordFixups(const XtRecord* record, XtU32* fixups, XtU32 fixupCount)
{
	if (!record || !fixups || !fixupCount)
		return;

	const XtRecordHeader*	header		= XtGetRecordHeader(record);
	XtComponent*			components	= (XtComponent*)header->top;
	XtU8*					baseAddr	= (XtU8*)record->object;

	// TODO: Rather than a linear list of all fixups, break the list into groups per component type with a repeat count

	XtU32* fixup = fixups;

	for(size_t i = 0; i < header->count; i++)
	{
		XtComponent* component = &components[i];

		XtU32 baseOffset = component->data - baseAddr;

		switch(component->behaviour)
		{
			case kXtCbObject:
				for(size_t j = 0; j < component->count; j++)
					XtGenerateTypeFixups(component->type, baseOffset + (j * component->type->size), fixup);
			break;

			case kXtCbPtr:
				for(size_t j = 0; j < component->count; j++)
					*fixup++ = baseOffset + (j * sizeof(XtIntPtr));
			break;

			case kXtCbHandle:
			break;
		}
	}

#if defined(PLATFORM_PC)
//	BIZ_ASSERT_MSG(fixup == (fixups + fixupCount), "XtGenerateRecordFixups: Mismatch between generated fixups and fixup count");
#endif
}
