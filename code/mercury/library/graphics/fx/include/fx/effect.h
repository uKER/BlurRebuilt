#ifndef __FX_EFFECT_H__
#define __FX_EFFECT_H__

#ifndef FX_ALLOW_INCLUDE
#error "Illegal include of FX library internals.h"
#endif

#include "fx/I_Effect.h"
#include "fx/Shaders.h"
#include "fx/Enums.h"
#include <btl/Containers/bitvector.h>
#include <system/serialise/btl.h>
#include <Display/context/SamplerUsageFlags.h>
#include <system/misc/SharedString.h>
#include <core/utility/PointerSlot32.h> // [Blur2PC][x64]


namespace Debug
{
class Logger;
}

namespace FX
{

class Effect;
class SharedParameterGroup;
struct Function;
struct SamplerState;
struct Struct;
struct gpuTexture;
void OnEffectLoad(Effect*);
void OnEffectRelease(Effect*);
void RegisterStruct(Struct *pDecl);
extern Debug::Logger &g_Log;
typedef U32 TechniqueHandle;

typedef Btl::BitVectorTemplate<Btl::BitArrayBase<Btl::Vector<U32, Btl::VectorParameters_Array<4> > > > SharedBindingBits;

/// list of shared parameter group resources
enum E_SPGResource
{
	E_SPGResource_PixelConstant,	// on the PS3, pixel shader constants are not stored in shader parameter groups (SPGs)
	E_SPGResource_VertexConstant,
	E_SPGResource_Texture,
	E_SPGResource_VertexTexture,
	E_SPGResource_Max
};

///
/// Annotation data types.
///
enum nglFXAnnotationType
{
	NGLFX_ANNOTATION_NONE,
	NGLFX_ANNOTATION_BOOL,
	NGLFX_ANNOTATION_INT,
	NGLFX_ANNOTATION_FLOAT,
	NGLFX_ANNOTATION_VECTOR,
	NGLFX_ANNOTATION_MATRIX,
	NGLFX_ANNOTATION_STRING,
};

///
/// Annotations are read-only data structures describing effects, techniques, passes and parameters.
///
/// They are parsed from the FX file and made available to the application for processing at runtime.
///
/// Example: sampler2D s < string DescriptionAnnotation = ""; >;
///
struct Annotation
{
	SharedString::String Name;  ///< Annotation name.
	U32 Type;        ///< Data type.  One of nglFXAnnotationType.

	/// Data value.  Actual contents depends on Type.
	union
	{
		int Bool;
		int Int;
		float Vector[4];
		float Matrix[16];
		float Float;
		const char* String; 
	};

	Annotation()
	{
		Type = NGLFX_ANNOTATION_NONE;
	}

	~Annotation()
	{
	}

#if CFG_CONVERTER
	// copy constructor, duplicates string parameter if necessary
	Annotation(Annotation const&t)
	{
		Name = t.Name;
		Type = t.Type;
		if (Type == NGLFX_ANNOTATION_STRING)
			String = t.String;
		else
			Int = t.Int;
	}
#endif

	template<typename Archive>
	void Serialise(Archive &ar)
	{
		ar << Name << (int&)Type;
		int c = 0;
		switch (Type)
		{
		case NGLFX_ANNOTATION_BOOL:
			c = 1;
			break;
		case NGLFX_ANNOTATION_INT:
		case NGLFX_ANNOTATION_FLOAT:
			c = 4;
			break;
		case NGLFX_ANNOTATION_VECTOR:
			c = 4;
			break;
		case NGLFX_ANNOTATION_MATRIX:
			c = 16;
			break;
		case NGLFX_ANNOTATION_STRING:
			int n = strlen((char*)String)+1;
			ar << n;
			ar.Write(String, n);
			return;
		}
		ar.Write(&Int, c);
	}

	template<typename Archive>
	void Deserialise(Archive &ar)
	{
		int c = 0;
		ar >> Name >> (int&)Type;
		switch (Type)
		{
		case NGLFX_ANNOTATION_BOOL:
			c = 1;
			break;
		case NGLFX_ANNOTATION_INT:
		case NGLFX_ANNOTATION_FLOAT:
			c = 4;
			break;
		case NGLFX_ANNOTATION_VECTOR:
			c = 4;
			break;
		case NGLFX_ANNOTATION_MATRIX:
			c = 16;
			break;
		case NGLFX_ANNOTATION_STRING:
			{
			Util::C_String str;
			ar >> str;
			String = SharedString::GetString(str).Chars();
			return;
			}
		}
		ar.Read(&Int, c);
	}
};

///
/// A binding source (or parameter data source) is an address to draw parameter data from.
///
enum BindingSource
{
	NGLFX_BIND_PARAMETER,        ///< The most common option, draw the parameter data from the parameter itself.
	NGLFX_BIND_SHARED,			 ///< Parameter is bound to g_SharedEffect
};

///
/// Currently active base pointer for each FX data source.  todo- These should be passed into nglFXBeginPass to allow thread safety.
///

/// Alternate means for setting sources.
void nglFXSetSource(int Source, const void* Data);

/// Basic data type for a parameter binding.  These correspond to hardware shader constant register types.
enum BindingType
{
	NGLFX_BIND_FLOAT,
	NGLFX_BIND_INT,
	NGLFX_BIND_BOOL,
	NGLFX_BIND_SAMPLER,
	NGLFX_BIND_NONE,
};

///
/// Bindings represent the connection between effect parameters and hardware register resources (constant registers or samplers).
///
/// Each pixel or vertex shader function has a table of these bindings that is used to set the registers associated with that shader.
///
struct FunctionBinding
{
	FunctionBinding() { Name = NULL; Shared = 0; Exported = 0; }
	~FunctionBinding() { }
	FunctionBinding(const FunctionBinding &t)
	{
		ParameterIdx = t.ParameterIdx;
		Name = t.Name;
		Handle = t.Handle;
		Type = t.Type;
		Source = t.Source;
		Offset = t.Offset;
		Count = t.Count;
		Shared = t.Shared;
	}

#if CFG_CONVERTER
	U32 ParameterIdx;	///< Index of the effect parameter this binding is bound to.
	U32 Count;          ///< Number of linearly allocated hardware registers this binding represents.
	U32 Handle;			///< Index of the hardware register.
	U32 Type;			///< Hardware register type.  One of BindingType.
	U32 Source;			///< Source to draw data from, defaults to NGLFX_BIND_PARAMETER.  Initialized at load time using callbacks.
	U32 Shared;
	U32 Exported;
	U32 Offset;			///< Offset from the source.
	const char* Name;	///< Name of the variable representing the hardware register.

#else
	U32 ParameterIdx : 8;
	U32 Count : 8;
	U32 Handle : 9;
	U32 Type : 4;
	U32 Source : 1;
	U32 Shared : 1;
	U32 Exported : 1;
	U32 Offset;
	const char* Name;
#endif

	template<typename Archive>
	void Serialise(Archive &ar)
	{
		ar << ParameterIdx << Handle << Type << Source << Offset << Count;
		if (Name)
		{
			int n = strlen(Name)+1;
			ar << n;
			for (int i=0; i<n; i++)
				ar << Name[i];
		}
		else
			 ar << 0;
	}

	template<typename Archive>
	void Deserialise(Archive &ar)
	{
		int iParameterIdx, iHandle, iType, iSource, iCount;
		ar >> iParameterIdx >> iHandle >> iType >> iSource >> Offset >> iCount;
		/*BIZ_ASSERT(iParameterIdx<255);*/	ParameterIdx = iParameterIdx;
		/*BIZ_ASSERT(iHandle<511);*/		Handle = iHandle;
		/*BIZ_ASSERT(iCount<255);*/			Count = iCount;
		/*BIZ_ASSERT(iType<15);*/			Type = iType;
		Source = NGLFX_BIND_PARAMETER;
		Util::C_String str;
		ar >> str;
		Name = SharedString::GetString(str).Chars();
	}
};

#if PLATFORM_PPU_SPU
struct PatchBinding
{
	U32 Source : 1;
	U32 PatchOffset : 16;
	U32 Offset;
};
#endif

///
/// The vertex or pixel function for a given pass.
///
/// This structure contains the microcode for a pixel or vertex shader along with supporting data structures.
///
struct Function
{
	Function()
	{
		Microcode = NULL;
		pShader = NULL;
	}

	~Function()
	{
		delete (U8*)Microcode;
	}

	void* Microcode;           ///< Raw microcode for the shader.
	U32 cMicrocode;			   ///< Microcode size
	Shader *pShader;           ///< Pointer to the compiled shader.

	Btl::Vector<FunctionBinding> ALIGN16 Bindings;
	Btl::Vector<FunctionBinding> SharedBindings;

#if PLATFORM_PPU_SPU
	Btl::Vector<PatchBinding> ALIGN16 PatchBindings;
#endif

	template<typename Archive>
	void Serialise(Archive &ar)
	{
		ar << cMicrocode << Bindings;
		ar.Write(Microcode, cMicrocode);
	}

	template<typename Archive>
	void Deserialise(Archive &ar)
	{
		ar >> cMicrocode >> Bindings;
		if (cMicrocode > 0)
		{
			Microcode = new U8[cMicrocode];
			ar.Read(Microcode, cMicrocode);
		}
	}
};


///
/// Constructs a scene matrix ID from three matrices, inverse and transpose flags.
///
#define NGLFX_SCENE_MATRIX_CODE(a,b,c,i,t) (0x80000000|(a)|((b)<<2)|((c)<<4)|((i)<<6)|((t)<<7))

///
/// Effect parameter type.
///
/// The effect parameter type is roughly related to the hardware register type, but is expanded and supports more
/// formats.  Eventually this will be combined with BindingType.
///
enum ParameterType
{
	NGLFX_PARAM_NONE,

	// Basic data types.
	NGLFX_PARAM_BOOL,
	NGLFX_PARAM_INT,
	NGLFX_PARAM_FLOAT,
	NGLFX_PARAM_VECTOR,
	NGLFX_PARAM_MATRIX,

	NGLFX_PARAM_BOOL_ARRAY,
	NGLFX_PARAM_INT_ARRAY,
	NGLFX_PARAM_FLOAT_ARRAY,
	NGLFX_PARAM_VECTOR_ARRAY,
	NGLFX_PARAM_MATRIX_ARRAY,

	// Texture parameters represent texture resources.
	NGLFX_PARAM_TEXTURE,
	NGLFX_PARAM_PROCEDURAL_TEXTURE,
	NGLFX_PARAM_TEMP_TEXTURE,

	// Sampler parameters reference texture parameters, and are what is actually assigned to the shader.
	NGLFX_PARAM_SAMPLER,
};

///
/// \defgroup NGLFX_PARAMS Parameter data structures
///
/// These contain the data for different kinds of parameters.
///
/// @{
///
struct nglFXParam_Bool
{
	int Value;
};

struct nglFXParam_Int
{
	int Value;
};

struct nglFXParam_Float
{
	float Value;
};

struct nglFXParam_Vector
{
	Maths::HVec4 Value;
};

struct nglFXParam_Matrix
{
	Maths::HMat44 Value;
};

struct nglFXParam_BoolArray
{
	int Count;
	int Pad[3];
	//int Value[];
};

struct nglFXParam_IntArray
{
	int Count;
	int Pad[3];
	//int Value[];
};

struct nglFXParam_FloatArray
{
	int Count;
	int Pad[3];
	//float Value[];
};

struct nglFXParam_VectorArray
{
	int Count;
	int Pad[3];
	//math::Vector4 Value[];
};

struct nglFXParam_MatrixArray
{
	int Count;
	int Pad[3];
	//math::Mat44 Value[];
};

// [Blur2PC][x64] Texture is a Core::PointerToSlot32 slot: the sampler keeps the
// two-word layout the effect converter baked into every ParameterData block.
struct nglFXParam_Sampler
{
	U32 Texture;
	int iSamplerState;
};

///
/// @}
///

///
/// Effect parameters are runtime controlled settings.
///
/// An effect parameter corresponds to a declared variable in the FX file.  Only simple plain-old-data parameter types
/// are supported, including bool, int, float, vector, matrix and texture.  All vector types are represented as float4,
/// and all matrix types are represented as float4x4.
/// 
/// Parameters can be changed manually at runtime using Effect API calls.  Rendering with a material also sets effect
/// parameters to those stored in the material.  Others are updated automatically by the FX runtime.
/// 
struct Parameter
{
	Parameter()
#if !CFG_GOLD
	: bOverride(0)
#endif
	{
	}

	SharedString::String Name;        ///< Name of the parameter.  Same as the name of the variable in the FX file.
	SharedString::String Semantic;    ///< Semantic value (specified after the ':' when declaring the variable).

	/// Data type of the parameter.  One of ParameterType.
	ParameterType Type;

	/// Pointer to the parameter data.
	union
	{
		void* Data;
		nglFXParam_Bool* Bool;
		nglFXParam_Int* Int;
		nglFXParam_Float* Float;
		nglFXParam_Vector* Vector;
		nglFXParam_Matrix* Matrix;
		nglFXParam_BoolArray* BoolArray;
		nglFXParam_IntArray* IntArray;
		nglFXParam_FloatArray* FloatArray;
		nglFXParam_VectorArray* VectorArray;
		nglFXParam_MatrixArray* MatrixArray;
		nglFXParam_Sampler* Sampler;
	};
	/// Size of the parameter data (in DWORDS)
	int Size;

	/// Shared parameter
	U8 bShared;
	U8 bExported;
#if !CFG_GOLD
	U8 bOverride;		// setting this will disable the SetParameterImmediate function
#endif

	/// Annotations from the FX file.
#if CFG_CONVERTER || !CFG_GOLD
	Btl::Vector<Annotation> Annotations;
#endif

#if CFG_CONVERTER
	/// offset of the Data pointer into the effect's ParameterData structure (in bytes)
	int Offset;
#endif

#if CFG_CONVERTER
	template<typename Archive>
	void Serialise(Archive &ar)
	{
		ar << Name << Semantic << (int&)Type << Size << Offset << bShared << Annotations;
	}
#endif

	template<typename Archive>
	void Deserialise(Archive &ar)
	{
		int iOffset;
		ar >> Name >> Semantic >> (int&)Type >> Size >> iOffset >> bShared;
		Data = (void*)iOffset;
#if CFG_CONVERTER
		Offset = iOffset;
#endif

#if !CFG_CONVERTER && CFG_GOLD
		Btl::Vector<Annotation> Annotations;
#endif
		ar >> Annotations;

		// check for 'source' annotation, and if it has one set the bExported flag
		bExported = false;
		for (int i=0; i<Annotations.Size(); i++)
		{
			if (!stricmp("uninitialised", Annotations[i].Name) && Annotations[i].Type == FX::NGLFX_ANNOTATION_BOOL)
			{
				bExported = Annotations[i].Bool?1:0;
				break;
			}
		}
	}
};

///
/// A platform independent structure describing the render state contents of a Pass.
///
struct nglFXRenderStates
{
	U32 AlphaBlendEnable;						//--
	E_Blend SrcBlend;
	E_Blend DestBlend;
	E_BlendOp BlendOp;
	U32 TextureFactor;

	U32 AlphaTestEnable;						//--
	E_CmpFunc AlphaFunc; // gpuCmpMode
	U32 AlphaRef;								//--

	float DepthBias;
	float SlopeScaleDepthBias;

	U32 DitherEnable;
	E_CullMode CullMode; // gpuCullMode
	E_FillMode FillMode; // gpuFillMode
	U32 ShadeMode; // gpuShadeMode
	U32 ColorWriteEnable; // gpuWriteMask

	U32 ZTestEnable;
	E_CmpFunc ZFunc; // gpuCmpMode
	U32 ZWriteEnable;

	U32 StencilEnable;
	E_StencilOp StencilPass;  // gpuStencilOp
	E_StencilOp StencilFail;  // gpuStencilOp
	E_StencilOp StencilZFail; // gpuStencilOp
	E_CmpFunc StencilFunc;  // gpuCmpMode
	U32 StencilMask;
	U32 StencilRef;
	U32 StencilWriteMask;

	U32 PointSpriteEnable;
	float PointSize;
	float PointSize_Min;
	float PointSize_Max;

	U32 Tessellate;
	float TessellationLevel;

	U32 AlphaToMaskEnable;						//--
	U32 AlphaToMaskOffsets;

	U32 HiZEnable;


	template<typename Archive>
	void Serialise(Archive &ar)
	{
		int *p = (int*)this;
		for (int x=0; x<sizeof(*this)/4; x++, p++)
			ar << *p;
	}

	template<typename Archive>
	void Deserialise(Archive &ar)
	{
		ar.Read(this, sizeof(*this));

		if (ar.m_stream.GetVersionNumber() <= 8)
		{
			if (AlphaToMaskEnable == 0)
				AlphaToMaskEnable = E_Bool_Unknown;
			else
				AlphaToMaskEnable = E_Bool_True;
			if (AlphaBlendEnable == 0)
				AlphaBlendEnable = E_Bool_Unknown;
			else
				AlphaBlendEnable = E_Bool_True;
			if (AlphaTestEnable == 0)
				AlphaTestEnable = E_Bool_Unknown;
			else
				AlphaTestEnable = E_Bool_True;
		}
	}
};

///
/// A pass represents all the render states for a single render pass over the geometry, including the pixel and vertex shader.
///
struct ALIGN16 Pass
{
	Pass()
	{
	}

	SharedString::String Name;   ///< Name of the pass from the FX file.

	/// Render states to be set up before rendering this pass.
	nglFXRenderStates RenderStates;

	SharedBindingBits ALIGN16 SharedBindingUsage;	// bit vector representing which shared bindings this shader requires (1=used)
	SharedBindingBits ALIGN16 SharedBindingMask;	// bit vector representing which shared bindings this shader overwrites (0=overwrites)

	Function VertexShader;		///< Vertex shader compiled for the pass.
	Function PixelShader;		///< Pixel shader compiled for the pass.

	/// Annotations from the FX file.
	Btl::Vector<Annotation> Annotations;

	Display::C_SamplerUsageFlags m_SamplerUsageFlags;
	U32 m_VtxAttrUsageFlags;

	template<typename Archive>
	void Serialise(Archive &ar)
	{
		ar << Name << RenderStates << VertexShader << PixelShader;
		ar << Annotations;
		// TODO: serialise compiledData
	}

	template<typename Archive>
	void Deserialise(Archive &ar)
	{
		ar >> Name >> RenderStates >> VertexShader >> PixelShader;
		ar >> Annotations;
	}
};

///
/// Techniques are used to provide multiple versions of the same effect, for example depth-only rendering.
///
struct ALIGN16 Technique
{
	///< Array of passes.
	Btl::Vector<Pass> Passes;

	/// Any annotations from the FX file.
	Btl::Vector<Annotation> Annotations;

	/// bitfield of subtechniques that represent this technique
	TechniqueHandle hTechnique;

	/// Name of the technique from the FX file.
	SharedString::String Name;

	template<typename Archive>
	void Serialise(Archive &ar)
	{
		ar << Name << Passes << Annotations;
	}

	template<typename Archive>
	void Deserialise(Archive &ar)
	{
		ar >> Name >> Passes >> Annotations;
	}
};


enum E_FXSemantic
{
	E_FXSemantic_Position,
	E_FXSemantic_BlendWeight,
	E_FXSemantic_BlendIndices,
	E_FXSemantic_Normal,
	E_FXSemantic_PSize,
	E_FXSemantic_TexCoord,
	E_FXSemantic_Tangent,
	E_FXSemantic_Binormal,
	E_FXSemantic_TessFactor,
	E_FXSemantic_Color,
	E_FXSemantic_Fog,
	E_FXSemantic_Depth,
	E_FXSemantic_Sample,
	E_FXSemantic_Invalid = -1,
};

/// a single vertex input semantic plus it's annotations
struct InputSemantic
{
	E_FXSemantic Usage;
	int UsageIndex;
	Btl::Vector<Annotation> Annotations;

	template<typename Archive>
	void Serialise(Archive &ar)
	{
		ar << (int)Usage << UsageIndex << Annotations;
	}

	template<typename Archive>
	void Deserialise(Archive &ar)
	{
		int iUsage;
		ar >> iUsage >> UsageIndex >> Annotations;
		Usage = (E_FXSemantic)iUsage;
	}
};

/// description of a single struct member
struct StructMember
{
	SharedString::String Name;	// name of the parameter
	int Size;				// number of floats
	Btl::Vector<Annotation> Annotations;

	template<typename Archive>
	void Serialise(Archive &ar)
	{
		ar << Name << Size << Annotations;
	}

	template<typename Archive>
	void Deserialise(Archive &ar)
	{
		ar >> Name >> Size >> Annotations;
	}
};

/// list of members inside a particular struct. this is used for debugging purposes
struct Struct
{
	SharedString::String Name;
	Btl::Vector<StructMember> Members;

	template<typename Archive>
	void Serialise(Archive &ar)
	{
		ar << Name << Members;
	}

	template<typename Archive>
	void Deserialise(Archive &ar)
	{
		ar >> Name >> Members;
	}
};

/// An effect represents a certain way to render geometry.
///
/// This can consist of multiple passes over the geometry, each with different render states and shaders.
/// Parameters and techniques can be used by the client to customize the rendering process.
///
/// To render an effect, first choose a technique and query the number of passes.  Then, iterate over the
/// passes calling nglFXBeginPass, drawing your geometry, then calling nglFXEndPass.
///
/// The effect runtime is highly optimized per platform, but the data comes from a common cross platform
/// FX file format, and results are intended to be identical.

#define NGL_FX_VERSION 9
#define NGL_FX_SUPPORTED_VERSION 8

TechniqueHandle RegisterSubtechnique(const char *pszName);

void NativeFX11ReleaseCache(const void* effect); // [Blur1PCDX11] NativeFX11.cpp

class Effect : public I_Effect
{
public:
	Effect()
	{
	}

	~Effect()
	{
		NativeFX11ReleaseCache(this);
	}

	Btl::Vector<Technique> ALIGN16 Techniques;
	Btl::Vector<Parameter> ALIGN16 Parameters;

	Btl::Vector<U32> ALIGN16 ParameterData;				///< Memory for parameter data.
#if !CFG_CONVERTER
	Btl::Vector<U32> ALIGN16 DefaultParameterData;		///< Copy of the original parameter data so it can be restored
#endif

	Btl::Vector<InputSemantic> VtxInputDeclaration;		///< A list of the vertex input semantics required for this shader

	SharedString::String Name;								///< Name of the effect, usually the filename (minus path and .fx).
	// [Blur1PCDX11] Internal FXB Name omits permutation suffixes. Runtime-only
	// identity set before deserialisation; never part of the PCFX wire format.
	SharedString::String NativeFilename;

#if CFG_CONVERTER
	Btl::Vector<Struct> Structs;						///< A description of the contents of some structs. used for debugging

	/// serialisation
	template<typename Archive>
	void Serialise(Archive &ar) const
	{
		ar << (int)NGL_FX_VERSION;
		ar << Name << Techniques << Parameters << ParameterData << VtxInputDeclaration << Structs;
	}
#endif

	template<typename Archive>
	void Deserialise(Archive &ar)
	{
		int iVersion;
		ar >> iVersion;

		int iParentVersion = ar.m_stream.GetVersionNumber();
		ar.m_stream.SetVersionNumber(iVersion);

#if !PLATFORM_SPU
		if (iVersion > NGL_FX_VERSION || iVersion<NGL_FX_SUPPORTED_VERSION)
		{
			LOGFATAL( Debug::GetLogger("graphics.fx"), "Wrong FX file version number (%d, expected %d). Application will probably crash.", iVersion, NGL_FX_VERSION);
			BIZ_ASSERT(0 && "Your shaders have the wrong version number");
		}
#endif
		ar >> Name >> Techniques >> Parameters >> ParameterData >> VtxInputDeclaration;

		// load the structs and register them with the struct manager
		int nStructs;
		ar >> nStructs;
		for (int i=0; i<nStructs; i++)
		{
			Struct *pStruct = new Struct;
			ar >> *pStruct;	// ownership of the Struct pointer passes to the RegisterStruct function
#if !CFG_CONVERTER
//			RegisterStruct(pStruct);
#else
			delete pStruct;
#endif
		}

#if 0
		for (int i = 0; i < Techniques.Size(); i++)
		{
			RegisterSubtechnique(Techniques[i].Name);
		}
#endif

		// fixup fx parameter offsets
		for (int i=0; i<Parameters.Size(); i++)
			Parameters[i].Data = ((U8*)&ParameterData[0]) + (int)(INTPTR)Parameters[i].Data;	// [Blur2PC][x64] Data holds the byte offset here

#if !CFG_CONVERTER
		int c = ParameterData.Size();
		if (c>0)
		{
			DefaultParameterData.Resize(c);
			memcpy(&DefaultParameterData[0], &ParameterData[0], sizeof(U32)*c);
		}

		OnEffectLoad(this);
#endif

		ar.m_stream.SetVersionNumber(iParentVersion);
	}

};

} // namespace FX

#endif
