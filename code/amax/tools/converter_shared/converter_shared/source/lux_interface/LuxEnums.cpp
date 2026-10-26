#include "stdafx.h"

#include <converter_shared/lux_interface/LuxEnums.h>

namespace LuxInterface
{

const char g_NamesBool[E_Bool_Count][6] = 
{
	"False",
	"True"
};

const char g_NamesYesNo[E_OnOff_Count][4] = 
{
	"No",
	"Yes"
};

const char g_NamesOnOff[E_OnOff_Count][4] = 
{
	"Off",
	"On"
};

const char g_NamesLightingModelType[E_LightingModelType_Count][20] = 
{
	"Unknown",
	"GlobalIllumination"
};

const char g_NamesInstanceMode[E_InstanceMode_Count][12] = 
{
	"Instance",
	"Instantiate",
	"Delete"
};

const char g_NamesBakeTarget[E_BakeTarget_Count][9] = 
{
	"Vertex",
	"Lightmap"
};

const char g_NamesTextureFormat[E_TextureFormat_Count][9] = 
{
	"DontCare",
	"A8L8",
	"DXT4",
	"RGBA32",
	"DXT1",
	"RGB16",
	"YUV"
};

const char g_NamesTextureType[E_TextureType_Count][13] = 
{
	"Unknown",
	"RGB",
	"Gotham_Night",
	"BasisV2",
	"Transfer"
};

const char g_NamesLightType[E_LightType_Count][13] = 
{
	"Unknown",
	"Directional",
	"Hemisphere",
	"Hemisphere2",
	"BounceGather",
	"HDR"
	//"PointLight",	
	//"SpotLight",
	//"Area",
};

const char g_NamesSetAttribute[E_SetAttribute_Count][15] = 
{
	"Light",
	"DontLight",
	"CastShadow",
	"DontCastShadow"
};

const char g_NamesSamplePointsType[E_SamplePointsType_Count][5] = 
{
	"None",
	"SH"
};

const char g_NamesSamplePointsOutputFormat[E_SamplePointsOutputFormat_Count][7] = 
{
	"Binary",
	"Text"
};

const char g_NamesGamma[E_Gamma_Count][5] = 
{
	"User",
	"On",
	"Off"
};

const char g_NamesTimeOfDay[E_TimeOfDay_Count][9] = 
{
	"All",
	"Sunny",
	"Overcast",
	"Night"
};

// TODO : validate this is what is actually used
const char g_NamesChannel[E_Channel_Count][12] = 
{
	"None",
	"Red", 
	"Green", 
	"Blue", 
	"RGB", 
	"Alpha",	
	"Sun", 
	"Sky",		
	"AnimatedOn",
	"AnimatedOff"
};

const char g_NamesForceGothamChannels[E_ForceGothamChannels_Count][12] = 
{
	"Off",
	"On",
	"version_two"
};

const char g_NamesFixSeams[E_FixSeams_Count][4] = 
{
	"Off",
	"On",
	"All"
};

void GetString( const S32 value, Util::C_String& str )
{
	str.Printf( "%d", value );
}

void GetString( const F32 value, Util::C_String& str )
{
	str.Printf( "%f", value );
}

void GetString( const Util::C_String& value, Util::C_String& str )
{
	str = value;
}

void GetString( const Maths::HVec3& value, Util::C_String& str )
{
	str.Printf("%f %f %f", (F32)value.X(), (F32)value.Y(), (F32)value.Z() );
}

void GetString( const Maths::C_RGBA_F32& value, Util::C_String& str )
{
	str.Printf("%f %f %f %f", (F32)value.R(), (F32)value.G(), (F32)value.B(), (F32)value.A() );
}

void GetString( const E_Bool value, Util::C_String& str )
{
	assert( value < E_Bool_Count );
	str = g_NamesBool[value];
}

void GetString( const E_OnOff value, Util::C_String& str )
{
	assert( value < E_OnOff_Count );
	str = g_NamesOnOff[value];
}

void GetString( const E_LightingModelType value, Util::C_String& str )
{
	assert( value < E_LightingModelType_Count );
	str = g_NamesLightingModelType[value];
}

void GetString( const E_InstanceMode value, Util::C_String& str )
{
	assert( value < E_InstanceMode_Count );
	str = g_NamesInstanceMode[value];
}

void GetString( const E_BakeTarget value, Util::C_String& str )
{
	assert( value < E_BakeTarget_Count );
	str = g_NamesBakeTarget[value];
}

void GetString( const E_TextureFormat value, Util::C_String& str )
{
	assert( value < E_TextureFormat_Count );
	str = g_NamesTextureFormat[value];
}

void GetString( const E_TextureType value, Util::C_String& str )
{
	assert( value < E_TextureType_Count );
	str = g_NamesTextureType[value];
}

void GetString( const E_LightType value, Util::C_String& str )
{
	assert( value < E_LightType_Count );
	str = g_NamesLightType[value];
}

void GetString( const E_SetAttribute value, Util::C_String& str )
{
	assert( value < E_SetAttribute_Count );
	str = g_NamesSetAttribute[value];
}

void GetString( const E_SamplePointsType value, Util::C_String& str )
{
	assert( value < E_SamplePointsType_Count );
	str = g_NamesSamplePointsType[value];
}

void GetString( const E_SamplePointsOutputFormat value, Util::C_String& str )
{
	assert( value < E_SamplePointsOutputFormat_Count );
	str = g_NamesSamplePointsOutputFormat[value];
}

void GetString( const E_Gamma value, Util::C_String& str )
{
	assert( value < E_Gamma_Count );
	str = g_NamesGamma[value];
}

void GetString( const E_TimeOfDay value, Util::C_String& str )
{
	assert( value < E_TimeOfDay_Count );
	str = g_NamesTimeOfDay[value];
}

void GetString( const E_Channel value, Util::C_String& str )
{
	assert( value < E_Channel_Count );
	str = g_NamesChannel[value];
}

void GetString( const E_ForceGothamChannels value, Util::C_String& str )
{
	assert( value < E_ForceGothamChannels_Count );
	str = g_NamesForceGothamChannels[value];
}

void GetString( const E_FixSeams value, Util::C_String& str )
{
	assert( value <E_FixSeams_Count );
	str = g_NamesFixSeams[value];
}

bool GetValue( const Util::C_String& str, S32& result )
{
	result = atoi(str.Chars());
	return true;
}

bool GetValue( const Util::C_String& str, F32& result )
{
	result = (F32)atof(str.Chars());
	return true;
}

bool GetValue( const Util::C_String& str, Util::C_String& result )
{
	result = str;
	return true;
}

bool GetValue( const Util::C_String& str, Maths::HVec3& result )
{
	F32 x, y, z;
	sscanf_s( str.Chars(), "%f %f %f", &x, &y, &z );	
	result = Maths::HVec3(x, y, z);
	return true;
}

bool GetValue( const Util::C_String& str, Maths::C_RGBA_F32& result )
{
	F32 r,g,b,a;
	sscanf_s( str.Chars(), "%f %f %f %f", &r, &g, &b, &a );	
	result = Maths::C_RGBA_F32(r,g,b,a);
	return true;
}

bool GetValue( const Util::C_String& str, E_Bool& result )
{
	for(S32 i=0; i<E_Bool_Count; i++)
	{
		if(_stricmp(str.Chars(), g_NamesOnOff[i]) == 0)
		{
			result = (E_Bool)i;
			return true;
		}

		if(_stricmp(str.Chars(), g_NamesYesNo[i]) == 0)
		{
			result = (E_Bool)i;
			return true;
		}

		if(_stricmp(str.Chars(), g_NamesBool[i]) == 0)
		{
			result = (E_Bool)i;
			return true;
		}
	}

	return false;
}

bool GetValue( const Util::C_String& str, E_OnOff& result )
{
	for(S32 i=0; i<E_OnOff_Count; i++)
	{
		if(_stricmp(str.Chars(), g_NamesOnOff[i]) == 0)
		{
			result = (E_OnOff)i;
			return true;
		}

		if(_stricmp(str.Chars(), g_NamesYesNo[i]) == 0)
		{
			result = (E_OnOff)i;
			return true;
		}

		if(_stricmp(str.Chars(), g_NamesBool[i]) == 0)
		{
			result = (E_OnOff)i;
			return true;
		}
	}

	return false;
}

bool GetValue( const Util::C_String& str, E_LightingModelType& result )
{
	for(S32 i=0; i<E_LightingModelType_Count; i++)
	{
		if(_stricmp(str.Chars(), g_NamesLightingModelType[i]) == 0)
		{
			result = (E_LightingModelType)i;
			return true;
		}
	}

	return false;
}

bool GetValue( const Util::C_String& str, E_InstanceMode& result )
{
	for(S32 i=0; i<E_InstanceMode_Count; i++)
	{
		if(_stricmp(str.Chars(), g_NamesInstanceMode[i]) == 0)
		{
			result = (E_InstanceMode)i;
			return true;
		}
	}

	return false;
}

bool GetValue( const Util::C_String& str, E_BakeTarget& result )
{
	for(S32 i=0; i<E_BakeTarget_Count; i++)
	{
		if(_stricmp(str.Chars(), g_NamesBakeTarget[i]) == 0)
		{
			result = (E_BakeTarget)i;
			return true;
		}
	}

	return false;
}

bool GetValue( const Util::C_String& str, E_TextureFormat& result )
{
	for(S32 i=0; i<E_TextureFormat_Count; i++)
	{
		if(_stricmp(str.Chars(), g_NamesTextureFormat[i]) == 0)
		{
			result = (E_TextureFormat)i;
			return true;
		}
	}

	return false;
}

bool GetValue( const Util::C_String& str, E_TextureType& result )
{
	for(S32 i=0; i<E_TextureType_Count; i++)
	{
		if(_stricmp(str.Chars(), g_NamesTextureType[i]) == 0)
		{
			result = (E_TextureType)i;
			return true;
		}
	}

	return false;
}

bool GetValue( const Util::C_String& str, E_LightType& result )
{
	for(S32 i=0; i<E_LightType_Count; i++)
	{
		if(_stricmp(str.Chars(), g_NamesLightType[i]) == 0)
		{
			result = (E_LightType)i;
			return true;
		}
	}

	return false;
}

bool GetValue( const Util::C_String& str, E_SetAttribute& result )
{
	for(S32 i=0; i<E_SetAttribute_Count; i++)
	{
		if(_stricmp(str.Chars(), g_NamesSetAttribute[i]) == 0)
		{
			result = (E_SetAttribute)i;
			return true;
		}
	}

	return false;
}

bool GetValue( const Util::C_String& str, E_SamplePointsType& result )
{
	for(S32 i=0; i<E_SamplePointsType_Count; i++)
	{
		if(_stricmp(str.Chars(), g_NamesSamplePointsType[i]) == 0)
		{
			result = (E_SamplePointsType)i;
			return true;
		}
	}

	return false;
}

bool GetValue( const Util::C_String& str, E_SamplePointsOutputFormat& result )
{
	for(S32 i=0; i<E_SamplePointsOutputFormat_Count; i++)
	{
		if(_stricmp(str.Chars(), g_NamesSamplePointsOutputFormat[i]) == 0)
		{
			result = (E_SamplePointsOutputFormat)i;
			return true;
		}
	}

	return false;
}

bool GetValue( const Util::C_String& str, E_Gamma& result )
{
	for(S32 i=0; i<E_Gamma_Count; i++)
	{
		if(_stricmp(str.Chars(), g_NamesGamma[i]) == 0)
		{
			result = (E_Gamma)i;
			return true;
		}
	}

	return false;
}

bool GetValue( const Util::C_String& str, E_TimeOfDay& result )
{
	for(S32 i=0; i<E_TimeOfDay_Count; i++)
	{
		if(_stricmp(str.Chars(), g_NamesTimeOfDay[i]) == 0)
		{
			result = (E_TimeOfDay)i;
			return true;
		}
	}

	return false;
}

bool GetValue( const Util::C_String& str, E_Channel& result )
{
	for(S32 i=0; i<E_Channel_Count; i++)
	{
		if(_stricmp(str.Chars(), g_NamesChannel[i]) == 0)
		{
			result = (E_Channel)i;
			return true;
		}
	}

	return false;
}

bool GetValue( const Util::C_String& str, E_ForceGothamChannels& result )
{
	for(S32 i=0; i<E_ForceGothamChannels_Count; i++)
	{
		if(_stricmp(str.Chars(), g_NamesForceGothamChannels[i]) == 0)
		{
			result = (E_ForceGothamChannels)i;
			return true;
		}
	}

	return false;
}

bool GetValue( const Util::C_String& str, E_FixSeams& result )
{
	for(S32 i=0; i<E_FixSeams_Count; i++)
	{
		if(_stricmp(str.Chars(), g_NamesFixSeams[i]) == 0)
		{
			result = (E_FixSeams)i;
			return true;
		}
	}

	return false;
}


} // LuxInterface

