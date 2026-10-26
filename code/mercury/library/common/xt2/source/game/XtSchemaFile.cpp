#include "Common/XtPch.h"
#include "XtGameImpl.h"
#include <Xt/Impl/XtUt.h>

// XtExportSchema / XtImportSchema

const XtU32 kXtSfTag			= 'XTSC';
const XtU16 kXtSfVersion		= 4;

const XtU32 kXtSfMaxTypes	= 4 * 1024;
const XtU32 kXtSfMaxBases	= 4 * 1024;
const XtU32 kXtSfMaxFields	= 16 * 1024;
const XtU32 kXtSfMaxStrings	= 256 * 1024;

struct XtSfSchema
{
	XtU32	tag;
	XtU16	version;
	XtU16	endian;
	XtU32	typeCount;
	XtU32	baseCount;
	XtU32	fieldCount;
	XtU32	stringsLength;
};

struct XtSfType
{
	XtU32	name;
	XtU32	size;
	XtU8	isAbstract;
	XtU8	baseCount;
	XtU16	fieldCount;
	XtU32	bases;
	XtU32	fields;
	XtU8	behaviour;
	XtU8	atom;
	XtU16	alignment;
};

struct XtSfBase
{
	XtU32	type;
	XtU32	offset;
};

struct XtSfField
{
	XtU32	name;
	XtU32	type;
	XtU32	value;
	XtU32	offset;
	XtU16	weak;
	XtU16	ptr;
	XtU16	array;
	XtU16	behaviour;
};

template<typename Type>
Type EndianSwizzleLe(Type value)
{
	if (XtGetEndian() == kXtLittleEndian)
		return value;

	return (Type)Swizzle(value);
}

bool XtExportSchema(const XtSchema* schema, XtU8*& schemaData, int& schemaLength)
{
	if (!schema)
		return false;

	// Tally total bytes required to store all type and field names

	int stringsLength = 0;

	for(int i = 0; i < schema->typeCount; i++)
		stringsLength += (int)strlen(schema->types[i].name) + 1;

	for(int i = 0; i < schema->fieldCount; i++)
	{
		XtField& field = schema->fields[i];

		if (field.name)
			stringsLength += (int)strlen(field.name) + 1;
	}

	// Compute total bytes required to store entire schema

	int totalLength = 0;

	totalLength += sizeof(XtSfSchema);
	totalLength += schema->typeCount * sizeof(XtSfType);
	totalLength += schema->baseCount * sizeof(XtSfBase);
	totalLength += schema->fieldCount * sizeof(XtSfField);
	totalLength += stringsLength;

	// Allocate and setup pointers into the schema file buffer

	XtU8* data = new XtU8[totalLength];

	if (!data)
		return false;

	XtSfSchema*	sfSchema	= (XtSfSchema*)data;
	XtSfType*	sfTypes		= (XtSfType*)(sfSchema + 1);
	XtSfBase*	sfBases		= (XtSfBase*)(sfTypes + schema->typeCount);
	XtSfField*	sfFields	= (XtSfField*)(sfBases + schema->baseCount);
	XtU8*		sfStrings	= (XtU8*)(sfFields + schema->fieldCount);

	// Schema file header

	sfSchema->tag			= EndianSwizzleLe<XtU32>(kXtSfTag);
	sfSchema->version		= EndianSwizzleLe<XtU16>(kXtSfVersion);
	sfSchema->endian		= EndianSwizzleLe<XtU16>(schema->endian);
	sfSchema->typeCount		= EndianSwizzleLe<XtU32>(schema->typeCount);
	sfSchema->baseCount		= EndianSwizzleLe<XtU32>(schema->baseCount);
	sfSchema->fieldCount	= EndianSwizzleLe<XtU32>(schema->fieldCount);
	sfSchema->stringsLength	= EndianSwizzleLe<XtU32>(stringsLength);

	int basesIndex		= 0;
	int fieldsIndex		= 0;
	int stringsIndex	= 0;

	for(int i = 0; i < schema->typeCount; i++)
	{
		XtType&		type	= schema->types[i];
		XtSfType&	sfType	= sfTypes[i];

		// Schema file type

		sfType.name			= EndianSwizzleLe<XtU32>(stringsIndex);
		sfType.size			= EndianSwizzleLe<XtU32>(type.size);
		sfType.isAbstract	= EndianSwizzleLe<XtU8>(type.isAbstract);
		sfType.baseCount	= EndianSwizzleLe<XtU8>(type.baseCount);
		sfType.fieldCount	= EndianSwizzleLe<XtU16>(type.fieldCount);
		sfType.bases		= EndianSwizzleLe<XtU32>((type.baseCount > 0) ? basesIndex : -1);
		sfType.fields		= EndianSwizzleLe<XtU32>((type.fieldCount > 0) ? fieldsIndex : -1);
		sfType.behaviour	= EndianSwizzleLe<XtU8>(type.behaviour);
		sfType.atom			= EndianSwizzleLe<XtU8>(type.atom);
		sfType.alignment	= EndianSwizzleLe<XtU16>(type.alignment);

		// Copy type name into string table

		int nameLength = (int)strlen(type.name) + 1;

		memcpy(sfStrings + stringsIndex, type.name, nameLength);

		// Claim base and field entries for this type

		basesIndex		+= type.baseCount;
		fieldsIndex		+= type.fieldCount;
		stringsIndex	+= nameLength;
	}

	for(int i = 0; i < schema->baseCount; i++)
	{
		XtBase&		base	= schema->bases[i];
		XtSfBase&	sfBase	= sfBases[i];

		// Schema file base

		sfBase.type		= EndianSwizzleLe<XtU32>(base.type ? (base.type - schema->types) : -1);
		sfBase.offset	= EndianSwizzleLe<XtU32>(base.offset);
	}

	for(int i = 0; i < schema->fieldCount; i++)
	{
		XtField&	field	= schema->fields[i];
		XtSfField&	sfField	= sfFields[i];

		// Schema file field

		sfField.name		= EndianSwizzleLe<XtU32>(field.name ? stringsIndex : -1);
		sfField.type		= EndianSwizzleLe<XtU32>(field.type ? (field.type - schema->types) : -1);
		sfField.offset		= EndianSwizzleLe<XtU32>(field.offset);
		sfField.array		= EndianSwizzleLe<XtU16>(field.array);
		sfField.behaviour	= EndianSwizzleLe<XtU16>(field.behaviour);

		switch(field.behaviour)
		{
			case kXtFbAtom:
			case kXtFbEnum:
			case kXtFbStruct:
			case kXtFbPtr:
			case kXtFbHandle:
			case kXtFbLabel:
				sfField.value = EndianSwizzleLe<XtU32>(field.value);
			break;

			case kXtFbFlags:
				sfField.value = EndianSwizzleLe<XtU32>(field.value ? (((XtType*)field.value) - schema->types) : -1);
			break;

			default:
				assert(0);
			break;
		}

		// Copy field name into string table

		if (field.name)
		{
			int nameLength = (int)strlen(field.name) + 1;

			memcpy(sfStrings + stringsIndex, field.name, nameLength);

			stringsIndex += nameLength;
		}
	}

	schemaData		= data;
	schemaLength	= totalLength;

	return true;
}

bool XtImportSchemaValidate(const XtU8* schemaData, int schemaLength)
{
	// First make sure the buffer is large enough to contain the schema header, then check the tag and version match

	if (schemaLength < (int)sizeof(XtSfSchema))
		return false;

	XtSfSchema* sfSchema = (XtSfSchema*)schemaData;

	XtU32 schemaTag		= EndianSwizzleLe<XtU32>(sfSchema->tag);
	XtU16 schemaVersion	= EndianSwizzleLe<XtU16>(sfSchema->version);

	if ((schemaTag != kXtSfTag) || (schemaVersion != kXtSfVersion))
		return false;

	// All counts must be within sensible limits

	XtU32 schemaTypeCount		= EndianSwizzleLe(sfSchema->typeCount);
	XtU32 schemaBaseCount		= EndianSwizzleLe(sfSchema->baseCount);
	XtU32 schemaFieldCount		= EndianSwizzleLe(sfSchema->fieldCount);
	XtU32 schemaStringsLength	= EndianSwizzleLe(sfSchema->stringsLength);

	if (
			(schemaTypeCount > kXtSfMaxTypes) ||
			(schemaBaseCount > kXtSfMaxBases) ||
			(schemaFieldCount > kXtSfMaxFields) ||
			(schemaStringsLength > kXtSfMaxStrings)
		)
	{
		return false;
	}

	// Compute how large the buffer should be, and check to see if it's at least that big

	int requiredSchemaLength = (
			sizeof(XtSfSchema) +
			(schemaTypeCount * sizeof(XtSfType)) +
			(schemaBaseCount * sizeof(XtSfBase)) +
			(schemaFieldCount * sizeof(XtSfField)) +
			schemaStringsLength
		);

	if (schemaLength < requiredSchemaLength)
		return false;

	// Generate some pointers into the buffer for convenience

	XtSfType*	sfTypes		= (XtSfType*)(sfSchema + 1);
	XtSfBase*	sfBases		= (XtSfBase*)(sfTypes + schemaTypeCount);
	XtSfField*	sfFields	= (XtSfField*)(sfBases + schemaBaseCount);
	XtU8*		sfStrings	= (XtU8*)(sfFields + schemaFieldCount);

	XtU32 baseIndex		= 0;
	XtU32 fieldIndex	= 0;
	XtU32 stringIndex	= 0;

	for(XtU32 i = 0; i < schemaTypeCount; i++)
	{
		XtSfType& sfType = sfTypes[i];

		XtU32	typeName		= EndianSwizzleLe(sfType.name);
		XtU16	typeBaseCount	= EndianSwizzleLe(sfType.baseCount);
		XtU16	typeFieldCount	= EndianSwizzleLe(sfType.fieldCount);
		XtU32	typeBases		= EndianSwizzleLe(sfType.bases);
		XtU32	typeFields		= EndianSwizzleLe(sfType.fields);
		XtU8	typeBehaviour	= EndianSwizzleLe(sfType.behaviour);

		// All type names must be correctly sequenced in the string table

		if (typeName != stringIndex)
			return false;

		for(;;)
		{
			if (stringIndex >= schemaStringsLength)
				return false;

			if (sfStrings[stringIndex++] == '\0')
				break;
		}

		// Type behaviour should be within sensible limits

		if (typeBehaviour >= kXtTypeBehaviourCount)
			return false;

		// If we have them, base types must all be correctly sequenced in the base type table

		if ((typeBases != (XtU32)-1) || (typeBaseCount != 0))
		{
			if (typeBases != baseIndex)
				return false;

			if ((baseIndex += typeBaseCount) > schemaBaseCount)
				return false;
		}

		// If we have them, fields must all be correctly sequenced in the field type table

		if ((typeFields != (XtU32)-1) || (typeFieldCount != 0))
		{
			if (typeFields != fieldIndex)
				return false;

			if ((fieldIndex += typeFieldCount) > schemaFieldCount)
				return false;
		}
	}

	// Verify all base types point within the type table

	for(XtU32 i = 0; i < schemaBaseCount; i++)
	{
		XtSfBase& sfBase = sfBases[i];

		XtU32 baseType = EndianSwizzleLe(sfBase.type);

		if (baseType >= schemaTypeCount)
			return false;
	}

	for(XtU32 i = 0; i < schemaFieldCount; i++)
	{	
		XtSfField& sfField = sfFields[i];

		XtU32 fieldName			= EndianSwizzleLe(sfField.name);
		XtU32 fieldType			= EndianSwizzleLe(sfField.type);
		XtU32 fieldValue		= EndianSwizzleLe(sfField.value);
		XtU32 fieldBehaviour	= EndianSwizzleLe(sfField.behaviour);

		// All field names must be correctly sequenced in the string table

		if (fieldName != stringIndex)
			return false;

		for(;;)
		{
			if (stringIndex >= schemaStringsLength)
				return false;

			if (sfStrings[stringIndex++] == '\0')
				break;
		}

		// No special fields please

		if (fieldBehaviour >= kXtFieldBehaviourCount)
			return false;

		switch(fieldBehaviour)
		{
			// Field types must point within type table

			case kXtFbAtom:
			case kXtFbEnum:
			case kXtFbFlags:
			case kXtFbStruct:
			case kXtFbPtr:
			case kXtFbHandle:
				if (fieldType >= schemaTypeCount)
					return false;
			break;

			// Labels don't have types

			case kXtFbLabel:
				if (fieldType != (XtU32)-1)
					return false;
			break;

			default:
				assert(0);
			break;
		}

		// Flags store their flag type pointer in value, so make sure it points within the type table

		if (fieldBehaviour == kXtFbFlags)
		{
			if (fieldValue >= schemaTypeCount)
				return false;
		}
	}

	return true;
}

XtSchema* XtImportSchema(const XtU8* schemaData, int schemaLength)
{
	// Schema file must validate if we intend to import it

	if (!XtImportSchemaValidate(schemaData, schemaLength))
		return 0;

	// Generate some points into the buffer for convenience

	XtSfSchema* sfSchema = (XtSfSchema*)schemaData;

	XtU32 schemaTypeCount		= EndianSwizzleLe(sfSchema->typeCount);
	XtU32 schemaBaseCount		= EndianSwizzleLe(sfSchema->baseCount);
	XtU32 schemaFieldCount		= EndianSwizzleLe(sfSchema->fieldCount);
	XtU32 schemaStringsLength	= EndianSwizzleLe(sfSchema->stringsLength);

	XtSfType*	sfTypes		= (XtSfType*)(sfSchema + 1);
	XtSfBase*	sfBases		= (XtSfBase*)(sfTypes + schemaTypeCount);
	XtSfField*	sfFields	= (XtSfField*)(sfBases + schemaBaseCount);
	XtU8*		sfStrings	= (XtU8*)(sfFields + schemaFieldCount);

	XtSchema* schema = XtCreateSchema(schemaTypeCount, schemaBaseCount, schemaFieldCount, schemaStringsLength);

	if (!schema)
		return 0;

	// XtSchema

	schema->endian = (XtEndian)EndianSwizzleLe(sfSchema->endian);

	memcpy(schema->strings, sfStrings, schemaStringsLength);

	for(XtU32 i = 0; i < schemaTypeCount; i++)
	{
		XtType&		type	= schema->types[i];
		XtSfType&	sfType	= sfTypes[i];

		// XtType

		XtU32 typeBases		= EndianSwizzleLe(sfType.bases);
		XtU32 typeFields	= EndianSwizzleLe(sfType.fields);

		type.name		= (const char*)schema->strings + EndianSwizzleLe(sfType.name);
		type.size		= EndianSwizzleLe(sfType.size);
		type.isAbstract	= EndianSwizzleLe(sfType.isAbstract);
		type.baseCount	= EndianSwizzleLe(sfType.baseCount);
		type.fieldCount	= EndianSwizzleLe(sfType.fieldCount);
		type.bases		= (typeBases != (XtU32)-1) ? (schema->bases + typeBases) : 0;
		type.fields		= (typeFields != (XtU32)-1) ? (schema->fields + typeFields) : 0;
		type.behaviour	= (XtTypeBehaviour)EndianSwizzleLe(sfType.behaviour);
		type.atom		= (XtAtomType)EndianSwizzleLe(sfType.atom);
		type.alignment	= EndianSwizzleLe(sfType.alignment);
		type.construct	= 0;
		type.event		= 0;
		type.destruct	= 0;
	}

	for(XtU32 i = 0; i < schemaBaseCount; i++)
	{
		XtBase&		base	= schema->bases[i];
		XtSfBase&	sfBase	= sfBases[i];

		// XtBase

		XtU32 baseType = EndianSwizzleLe(sfBase.type);

		base.type	= (baseType != (XtU32)-1) ? (schema->types + baseType) : 0;
		base.offset	= EndianSwizzleLe(sfBase.offset);
	}

	for(XtU32 i = 0; i < schemaFieldCount; i++)
	{
		XtField&	field	= schema->fields[i];
		XtSfField&	sfField	= sfFields[i];

		// XtField

		XtU32 fieldType		= EndianSwizzleLe(sfField.type);
		XtU32 fieldValue	= EndianSwizzleLe(sfField.value);

		field.name			= (const char*)schema->strings + EndianSwizzleLe(sfField.name);
		field.type			= (fieldType != (XtU32)-1) ? (schema->types + fieldType) : 0;
		field.offset		= EndianSwizzleLe(sfField.offset);
		field.array			= EndianSwizzleLe(sfField.array);
		field.behaviour		= (XtFieldBehaviour)EndianSwizzleLe(sfField.behaviour);

		switch(field.behaviour)
		{
			case kXtFbAtom:
			case kXtFbEnum:
			case kXtFbStruct:
			case kXtFbPtr:
			case kXtFbHandle:
			case kXtFbLabel:
				field.value = fieldValue;
			break;

			case kXtFbFlags:
				field.value = (fieldValue != (XtU32)-1) ? (XtIntPtr)(schema->types + fieldValue) : 0;
			break;

			default:
				assert(0);
			break;
		}
	}

	return schema;
}