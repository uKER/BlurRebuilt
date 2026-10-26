#ifndef _COMPONENT_STRINGS_H_


struct ComponentStringElement
{
	Audio::Component::Types eType;
	char* strName;
};


ComponentStringElement	aComponentStringLookup[];


U32						GetComponentType(const char* strLookup);
char*					GetComponentString(Audio::Component::Types eType);


#endif	//	_COMPONENT_STRINGS_H_