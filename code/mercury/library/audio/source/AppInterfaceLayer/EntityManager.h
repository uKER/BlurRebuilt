#ifndef _ENTITY_MANAGER_H_
#define _ENTITY_MANAGER_H_


#include "PagedArray.h"


namespace Audio
{

#define	MAX_NUM_SCRIPT_LINES			32
#define	MAX_NUM_ENVIRONMENT_PARAMETERS	32


	struct EntityCallback
	{
		EntityInstance* 						m_pEntity;
		EntityDataMemberHandle					m_hDataMember;	
	};


	class EntityManager /*: public IEntityManager*/
	{

	public:
		EntityManager();
		~EntityManager();

		EntityDefinitionHandle				CreateDefinition();
		bool								ReleaseDefinition(EntityDefinitionHandle hDefinition);
		EntityDefinition*					GetDefinitionFromHandle(EntityDefinitionHandle hDef);
		EntityType*							SpawnEntityType(EntityDefinitionHandle hDef);
		EntityInstance*						SpawnEntityInstance(EntityType* pEntityType);
		U32									AddCallbackData(EntityInstance* pEntity, EntityDataMemberHandle hDataMember);

	private:
		PagedArray<EntityDefinition*>		m_apDefinition;
		Ut::Vector<EntityCallback*>			m_apCallback;
	};

}

#endif	//	#ifndef _ENTITY_MANAGER_H_
