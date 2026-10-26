#ifndef _ENTITY_DEFINITION_H_
#define _ENTITY_DEFINITION_H_

#include "../../include/StringHash.h"
#include <btl/allocators/stackallocator.h>
#include <btl/containers/openhashmap.h>

#define USE_ALLOCATOR_FOR_ENTITY_DEFINITIONS
#define USE_STRING_HASHES_FOR_ENTITY_DEFINITIONS

// saves about 7%. 
#define ENTITYDEF_USE_U16

// this optimises memory footprint by quite a lot but there will be a performance overhead associated with it
#define TRIM_ARRAYS

namespace Audio
{
	// forwards decl's
	class EntityType;
	class EntityInstance;
	struct EntityDeclaration;

	typedef U32 CommonFunctionHandle;
	typedef U32 LocalFunctionHandle;
	typedef U32 EntityTypeDataMemberHandle;
	typedef U32 EntityDataMemberHandle;

#ifdef ENTITYDEF_USE_U16
	typedef U16 EntityDefIndex;
#else
	typedef U32 EntityDefIndex;
#endif

	typedef PagedArrayHandle EntityInstanceHandle;
	typedef PagedArrayHandle EntityTypeHandle;
	typedef PagedArrayHandle EntityDefinitionHandle;

	typedef Btl::OpenHashMap<U32, EntityDataMemberHandle> C_DeclarationHashMap; // maps string hashes to instance components.

	//struct ScriptLineDef
	//{
	//	char*					m_strParam1;
	//	char*					m_strParam2;
	//	U16						m_u16Param1Index;
	//	U16						m_u16Param2Index;
	//};

	struct EntityDefString
	{
		bool operator == ( const char* pchStr );
		void Set(const char* pchStr);
		bool IsValid();

		#ifdef USE_STRING_HASHES_FOR_ENTITY_DEFINITIONS
		StringHash m_Str;
		#else
		Util::C_String m_Str;
		#endif
	};;

	enum EntityCommand
	{
		eInvalidCommand = 0,
		// connect commands should be consecutive in this list
		//eConnectMono,
		//eConnectStereo,
		//eConnectSurround,
		//eConnectSurroundLFE,
		//eConnectControl,
		//eConnectEvent,
		eConnectInternal,
		eConnectToParameterised,
		eConnectExternal = eConnectToParameterised,	// deprecated
		eConnectFromParameterised,
		eSpawnInstance,
		eLoadObject,
		eGetObjectComponent,
		eGetInstanceComponent,

		TotalNumEntityCommands
	};


	enum EntityVariableType
	{
		eObject = 0,
		eInstance,
		eObjectComponent,
		eInstanceComponent
		// careful if adding type. this is packed into 2 bits
	};


	// m_u32Index will be an index into the appropriate vector, which will be determined by m_eType
	// eg, if m_eType is eObject or eObjectComponent, m_u32Index will be an index into the script
	// definition's m_CommonData vector.
	// this means that the definition's data member vectors must be completely populated before
	// the EntityDeclarations are set up
	struct DeclBits
	{
		unsigned variableType : 2;
		unsigned bActionOut : 1;
		unsigned pad : 13;
	};

	struct EntityDeclaration
	{
	public:
		EntityDefString							m_Name;

		EntityDefIndex							m_u32Index;
		DeclBits								m_uBitField;

	public:
		void									SetActionOut(bool bTrue) { m_uBitField.bActionOut = bTrue; }
		void									SetVariableType(EntityVariableType eType) { m_uBitField.variableType = eType; }

		bool									IsActionOut() { return m_uBitField.bActionOut; }
		EntityVariableType						VariableType() { return static_cast<EntityVariableType>(m_uBitField.variableType); }
	};


	class EntityInstruction
	{
	public:
		static const EntityDefIndex Undefined = ((EntityDefIndex)-1);

		EntityCommand Command() { return static_cast<EntityCommand>(m_uCommand); }
		EntityInstruction() : m_uCommand(eInvalidCommand) {}

		U16					m_uCommand;
	};

	
	class EntityConnectInternalInstruction : public EntityInstruction
	{
	public:
		EntityConnectInternalInstruction() :	m_u32DownstreamConnectionIndex(Undefined),
												m_u32UpstreamConnectionIndex(Undefined)/*,
												m_u32ConnectorIndex(Undefined)*/
												{ m_uCommand = eConnectInternal; }
		bool IsValid() 
		{
			return (m_u32DownstreamConnectionIndex != Undefined && m_u32UpstreamConnectionIndex != Undefined/* && m_u32ConnectorIndex != Undefined*/);   
		}

		EntityDefIndex							m_u32DownstreamConnectionIndex;
		EntityDefIndex							m_u32UpstreamConnectionIndex;
		/*EntityDefIndex							m_u32ConnectorIndex;*/ // doesnt seem to be used anywhere
	};


	class EntityConnectToParameterisedInstruction : public EntityInstruction
	{

	public:
		EntityConnectToParameterisedInstruction() :	m_u32DownstreamConnectionIndex(Undefined),
													m_u32UpstreamConnectionIndex(Undefined)
													{ m_uCommand = eConnectToParameterised; }
		bool IsValid() 
		{
			return (m_u32DownstreamConnectionIndex != Undefined && m_u32UpstreamConnectionIndex != Undefined);   
		}

		EntityDefIndex							m_u32DownstreamConnectionIndex;
		EntityDefIndex							m_u32UpstreamConnectionIndex;
	};


	class EntityConnectFromParameterisedInstruction : public EntityInstruction
	{

	public:
		EntityConnectFromParameterisedInstruction() :	m_u32DownstreamConnectionIndex(Undefined),
														m_u32UpstreamConnectionIndex(Undefined)
														{ m_uCommand = eConnectFromParameterised; }
		bool IsValid() 
		{
			return (m_u32DownstreamConnectionIndex != Undefined && m_u32UpstreamConnectionIndex != Undefined);   
		}

		EntityDefIndex							m_u32DownstreamConnectionIndex;
		EntityDefIndex							m_u32UpstreamConnectionIndex;
	};


	class EntitySpawnInstanceInstruction : public EntityInstruction
	{

	public:
		EntitySpawnInstanceInstruction() :	m_u32InstanceIndex(Undefined),
											m_u32ObjectIndex(Undefined)	
											{ m_uCommand = eSpawnInstance; }
		bool IsValid()
		{
			return (m_u32InstanceIndex != Undefined && m_u32ObjectIndex != Undefined);   
		}

		EntityDefIndex						m_u32InstanceIndex;
		EntityDefIndex						m_u32ObjectIndex;
	};


	class EntityLoadObjectInstruction : public EntityInstruction
	{

	public:
		EntityLoadObjectInstruction() :	m_u32ObjectIndex(Undefined)
										{ m_uCommand = eLoadObject;  }
		bool IsValid() 
		{
			return (m_u32ObjectIndex != Undefined && m_strFilename != "");   
		}

		EntityDefIndex							m_u32ObjectIndex;
		Util::C_String							m_strFilename;
	};

	class EntityGetObjectComponentInstruction : public EntityInstruction
	{

	public:
		EntityGetObjectComponentInstruction() : m_u32ObjectIndex(Undefined),
												m_u32ComponentIndex(Undefined)
												{ m_uCommand = eGetObjectComponent;  }
		bool IsValid()
		{ 
			return (m_u32ObjectIndex != Undefined && m_ComponentName.IsValid() && m_u32ComponentIndex != Undefined);   
		}

		EntityDefIndex							m_u32ObjectIndex;
		EntityDefString							m_ComponentName;
		EntityDefIndex							m_u32ComponentIndex;
	};


	class EntityGetInstanceComponentInstruction : public EntityInstruction
	{

	public:
		EntityGetInstanceComponentInstruction() : m_u32InstanceIndex(Undefined),
												m_u32ObjectComponentIndex(Undefined),
												m_u32InstanceComponentIndex(Undefined)
												{ m_uCommand = eGetInstanceComponent; }
		bool IsValid()
		{
			return (m_u32InstanceIndex != Undefined && m_u32ObjectComponentIndex != Undefined && m_u32InstanceComponentIndex != Undefined);   
		}

		EntityDefIndex							m_u32InstanceIndex;
		EntityDefIndex							m_u32ObjectComponentIndex;
		EntityDefIndex							m_u32InstanceComponentIndex;
	};

	typedef Ut::Vector<EntityInstruction*> InstructionList;

	struct EntityFunction
	{
		EntityInstruction*						GetInstruction(U32 index);
		void									AddInstruction(EntityInstruction* pInst);

		Util::C_String							m_strName;

		InstructionList							m_Instructions;
		
		bool									m_bAcceptParameter;
		U32										m_u32ParameterId;
	};

/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	EntityDefinition
//
/////////////////////////////////////////////////////////////////////////////////////////////////////
	class EntityDefinition
	{

		friend struct XtEntityDef;
		friend class InterfaceController;
		friend class EntityManager;
		friend class Subsystem;

	public:

		template <typename T>
		static T* Allocate()
		{
			#ifdef USE_ALLOCATOR_FOR_ENTITY_DEFINITIONS
			//MEMORY_TRACKER_PUSH("Audio.EntityDefinitions.Classes.Allocator");
			T* pInst = m_Allocator.Allocate<T>();
			//MEMORY_TRACKER_POP("Audio.EntityDefinitions.Classes.Allocator");
			#else
			T* pInst = new T;
			#endif

			return pInst;
		}

		EntityDefinition();
		~EntityDefinition();

		// for creating a definiton..
	public:
		bool									Init(U32 u32UniqueId) { if (m_bInited) return false; m_u32UniqueId = u32UniqueId; m_bInited = true; return true; }
		CommonFunctionHandle					AddCommonFunction(const char* strName);
		LocalFunctionHandle						AddLocalFunction(const char* strName);
		LocalFunctionHandle						AddParameterisedLocalFunction(const char* strName, U32 u32ParameterId);	// ? or, could have AddLocalConnectFunction, AddLocalCreateFunction
		bool									AddInstructionToCommonFunction(CommonFunctionHandle hFunction, EntityInstruction* pInstruction);
		bool									AddInstructionToLocalFunction(LocalFunctionHandle hFunction, EntityInstruction* pInstruction);
		bool									AddInstructionToParameterisedLocalFunction(LocalFunctionHandle hFunction, EntityInstruction* pInstruction);
		bool									DeclareObject(const char* strName, EntityTypeDataMemberHandle hObject);
		bool									DeclareObjectComponent(const char* strName, EntityTypeDataMemberHandle hObjectComponent);
		bool									DeclareInstance(const char* strName, EntityDataMemberHandle hInstance);
		bool									DeclareParameterInInstanceComponent(const char* strName, EntityDataMemberHandle hInstanceComponent);
		bool									DeclareActionInInstanceComponent(const char* strName, EntityDataMemberHandle hInstanceComponent);
		bool									DeclareParameterOutInstanceComponent(const char* strName, EntityDataMemberHandle hInstanceComponent);
		bool									DeclareActionOutInstanceComponent(const char* strName, EntityDataMemberHandle hInstanceComponent);
		bool									DeclareVectorInInstanceComponent(const char* strName, EntityDataMemberHandle hInstanceComponent);
		bool									DeclareAudioOutInstanceComponent(const char* strName, EntityDataMemberHandle hInstanceComponent);
//		S32										GetParameterOutIndex(EntityDataMemberHandle hInstanceComponent);
		EntityTypeDataMemberHandle				AddObjectMember();
		EntityTypeDataMemberHandle				AddObjectComponentMember();
		EntityDataMemberHandle					AddInstanceMember();
		EntityDataMemberHandle					AddInstanceComponentMember();
		inline bool								EndSetup() { if (! m_bSettingUp) return false; m_bSettingUp = false; TrimArrays(); return true; }

		// for spawning objects from the definition...
		EntityType*								SpawnEntityType();
		EntityInstance*							SpawnEntityInstance();

		// for retrieving data once definition has been set up..
	public:
		CommonFunctionHandle					GetCommonFunction(const char* strName);
		LocalFunctionHandle						GetLocalFunction(const char* strName);

		S32										GetNumParameterisedFunctions(const char* strName);
		LocalFunctionHandle						GetParameterisedFunctionByIndex(const char* strName, const S32 sIndex);
		LocalFunctionHandle						GetParameterisedFunctionById(const char* strName, const U32 uParameterId);

		EntityTypeDataMemberHandle				LookupObject(const char* strName);
		EntityTypeDataMemberHandle				LookupObjectComponent(const char* strName);
		EntityDataMemberHandle					LookupInstance(const char* strName);
		EntityDataMemberHandle					LookupInstance( StringHash m_Str );
		EntityDataMemberHandle					LookupInstanceComponent(const char* strName);
		EntityDataMemberHandle					LookupInstanceComponent(StringHash hash);
		inline U32								GetNumObjects() { return m_u32NumObjectDataMembers; }
		inline U32								GetNumObjectComponents() { return m_u32NumObjectComponentDataMembers; }
		inline U32								GetNumInstances() { return m_u32NumInstanceDataMembers; }
		inline U32								GetNumInstanceComponents() { return m_u32NumInstanceComponentDataMembers; }
		U32										GetId() { return m_u32UniqueId; }
		bool									RunReverseEntityTypeFunction(CommonFunctionHandle hFunction, EntityType* pEntityType, bool bMaintainHandles = false);
		bool									RunReverseEntityInstanceFunction(LocalFunctionHandle hFunction, EntityInstance* pEntityInstance, bool bMaintainHandles = false);
		bool									RunReverseEntityInstanceParameterisedFunction(LocalFunctionHandle hFunction, EntityInstance* pEntityInstance, EntityInstance* pParameterInstance, bool bMaintainHandles = false);

		void									TrimArrays();
		// for running functions
	private:
		bool									RunEntityTypeFunction(CommonFunctionHandle hFunction, EntityType* pEntityType, bool bReuseHandles = false);
		bool									RunEntityInstanceFunction(LocalFunctionHandle hFunction, EntityInstance* pEntityInstance, bool bReuseHandles = false);
		bool									RunEntityInstanceParameterisedFunction(LocalFunctionHandle hFunction, EntityInstance* pEntityInstance, EntityInstance* pParameterInstance, bool bReuseHandles = false);

		// misc
		void									SetName(const char* pchName) { m_strName = pchName; }
		void									SetGroupName(const char* pchName) { m_strGroupName = pchName; }
	public:
		static Btl::DynamicStackAllocator&		Allocator() { return m_Allocator; }

		const Btl::String&						Name() const { return m_strName; }
		const Btl::String&						GroupName() const { return m_strGroupName; }

	private:
		bool									DeclareInstanceComponent(const char* strName, EntityDataMemberHandle hInstanceComponent);
		
		Ut::Vector<EntityFunction>				m_CommonFunctions;
		Ut::Vector<EntityFunction>				m_LocalFunctions;
		Ut::Vector<EntityFunction>				m_ParameterisedLocalFunctions;
		Ut::Vector<EntityDeclaration*>			m_CommonDeclarations;
		Ut::Vector<EntityDeclaration*>			m_LocalDeclarations;

		C_DeclarationHashMap					m_LocalDeclHashMap;

		U32										m_u32NumObjectDataMembers;
		U32										m_u32NumObjectComponentDataMembers;
		U32										m_u32NumInstanceDataMembers;
		U32										m_u32NumInstanceComponentDataMembers;
//		U32										m_u32NumDeclaredParameterOuts;
		U32										m_u32NumDeclaredActionOuts;
		U32										m_u32UniqueId;
		bool									m_bSettingUp;
		bool									m_bInited;
		Btl::String								m_strName;
		Btl::String								m_strGroupName;

		static Btl::DynamicStackAllocator		m_Allocator;
	};
}


#endif	//	#ifndef _ENTITY_DEFINITION_H_
