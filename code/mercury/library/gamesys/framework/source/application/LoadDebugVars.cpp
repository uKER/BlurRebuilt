#include "FrameworkPch.h"
#include <framework/application/LoadDebugVars.h>
#include <System/Serialise/I_FileSystem.h>
#include <system/serialise/TextSerialise.h>

namespace Framework
{

Util::C_String FieldValue(const Util::C_String& line, Util::C_String key)
{
	key += "=\"";
	S32 start = line.Search(key);
	if (start < 0)
		return "";
	
	start += key.Length();
	S32 end = line.Search('\"', start);

	return line(start, end);
}

static void SetDebugVar(Debug::VarBase& varbase, const Util::C_String& type, const Util::C_String& value)
{
	if (type == "bool")
	{
		Debug::Bool& var = (Debug::Bool&)varbase;
		if (value == "true")
			var = true;
		else if (value == "false")
			var = false;
		else
			var = atoi(value.Chars()) != 0;
	}
	else if (type == "int")
	{
		Debug::Int& var = (Debug::Int&)varbase;
		var = atoi(value.Chars());
	}
	else if (type == "float")
	{
		Debug::Float& var = (Debug::Float&)varbase;
		var = (F32)atof(value.Chars());
	}
	else if (type == "enum")
	{
		Debug::EnumBase& var = (Debug::EnumBase&)varbase;
		var.SetValueByName(value.Chars());
	}
	else if (type == "vector2")
	{	
		Debug::Vector2& var = (Debug::Vector2&)varbase;
		F32 vx = 0, vy = 0;
		sscanf(value.Chars(), "%f %f", &vx, &vy);
		var.SetValue(vx, vy);
	}
	else if (type == "vector3")
	{	
		Debug::Vector3& var = (Debug::Vector3&)varbase;
		F32 vx = 0, vy = 0, vz = 0;
		sscanf(value.Chars(), "%f %f %f", &vx, &vy, &vz);
		var.SetValue(vx, vy, vz);
	}
	else if (type == "vector4")
	{	
		Debug::Vector4& var = (Debug::Vector4&)varbase;
		F32 vx = 0, vy = 0, vz = 0, vw = 0;
		sscanf(value.Chars(), "%f %f %f %f", &vx, &vy, &vz, &vw);
		var.SetValue(vx, vy, vz, vw);
	}
	else if (type == "colour")
	{	
		Debug::Colour& var = (Debug::Colour&)varbase;
		F32 r = 0, g = 0, b = 0, a = 0;
		sscanf(value.Chars(), "%f %f %f %f", &r, &g, &b, &a);
		var.SetValue(r, g, b, a);
	}
	else if (type == "string")
	{	
		Debug::String& var = (Debug::String&)varbase;
		var = value.Chars();
	}
}

void LoadDebugVars(const Util::C_String& filename)
{
	using namespace Util;

	Sys::I_FileSystem& fs = *Sys::GetDefaultFileSystem();

	if (!fs.FileExists(filename))
		return;

	Sys::C_OpenParams params;
	params.m_IsCritical = false;
	Sys::I_File* file = fs.Open(filename, Sys::E_FileAccess_Read, params);
	if (file == 0)
		return;

	Sys::TextDeserialiser deserial(*file);

	do
	{
		C_String line;
		deserial >> line;

		// Parse the line...
		C_String name = FieldValue(line, "name");
		C_String type = FieldValue(line, "type");
		C_String value = FieldValue(line, "value");

		// Find the variable and set its value
		for (Debug::Iterator it; it; ++it)
		{
			if (name == it->Name())
				SetDebugVar(*it, type, value);
		}		
	}
	while (file->GetPosition() < file->Length());

	delete file;
	file = 0;
}

void LoadDebugVars()
{
	LoadDebugVars("User\\debug_values.xml");
	LoadDebugVars("debug_values.xml");
}

void SaveDebugVars()
{
	using namespace Util;

	Sys::I_FileSystem& fs = *Sys::GetDefaultFileSystem();

	Sys::C_OpenParams params;
	params.m_IsCritical = false;
	Sys::I_File* file = fs.Open("debug_values.xml", Sys::E_FileAccess_Write, params);
	if (file == 0)
		return;
	
	Sys::TextSerialiser serial(*file);

	serial << C_String("<Values>\n");
	
	// Search for all persistant debug variables:
	for (Debug::Iterator it; it; ++it)
	{
		if ((it->Flags() & Debug::kVfPersist) == 0)
			continue;

		const char *name = it->Name();

		C_String string;
		string.Printf("\t<Variable name=\"%s\" ", it->Name());
		serial << string;
		
		string = C_String();
		switch (it->Type())
		{
			case Debug::kVtBool:
			{
				const Debug::Bool& var = (Debug::Bool&)*it;
				string.Printf("type=\"bool\" value=\"%s\"", var.Value() ? "true" : "false");
			}
			break;
			case Debug::kVtInt:
			{
				const Debug::Int& var = (Debug::Int&)*it;
				string.Printf("type=\"int\" value=\"%d\"", var.Value());
			}
			break;
			case Debug::kVtFloat:
			{
				const Debug::Float& var = (Debug::Float&)*it;
				string.Printf("type=\"float\" value=\"%f\"", var.Value());
			}
			break;
			case Debug::kVtEnum:
			{
				const Debug::EnumBase& var = (Debug::EnumBase&)*it;
				string.Printf("type=\"enum\" value=\"%s\"", var.ValueAsName());
			}
			break;
			case Debug::kVtVector2:
			{
				const Debug::Vector2& var = (Debug::Vector2&)*it;
				string.Printf("type=\"vector2\" value=\"%f %f\"", var->vx, var->vy);
			}
			break;
			case Debug::kVtVector3:
			{
				const Debug::Vector3& var = (Debug::Vector3&)*it;
				string.Printf("type=\"vector3\" value=\"%f %f %f\"", var->vx, var->vy, var->vz);
			}
			break;
			case Debug::kVtVector4:
			{
				const Debug::Vector4& var = (Debug::Vector4&)*it;
				string.Printf("type=\"vector4\" value=\"%f %f %f %f\"", var->vx, var->vy, var->vz, var->vw);
			}
			break;
			case Debug::kVtColour:
			{
				const Debug::Colour& var = (Debug::Colour&)*it;
				string.Printf("type=\"colour\" value=\"%f %f %f %f\"", var->r, var->g, var->b, var->a);
			}
			break;
			case Debug::kVtString:
			{
				const Debug::String& var = (Debug::String&)*it;
				string.Printf("type=\"string\" value=\"%s\"", var.Value());
			}
			break;
		}

		serial << string << "/>\n";
	}

	serial << C_String("</Values>\n");

	delete file;
	file = 0;
}

}
