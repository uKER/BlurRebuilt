#ifndef _DEFINITIONMANAGER_H_
#define _DEFINITIONMANAGER_H_

namespace Audio
{

////////////////////////////////////////////////////////////////////////////////
//	IDefinitionManager
////////////////////////////////////////////////////////////////////////////////
 //   class IDefinitionManager
 //   {
 //   public:
 //       audio_virtual ~IDefinitionManager() {}

	//private:
 //       audio_virtual IDefinition* Create(const char* pchFilename) = 0;
 //       audio_virtual Result Destroy(IDefinition* pDefinition) = 0;
 //       audio_virtual Result DestroyAll() = 0;

	//	friend class ObjectManager;
	//	friend class Object;
 //   };

////////////////////////////////////////////////////////////////////////////////
//	DefinitionManager
////////////////////////////////////////////////////////////////////////////////
    class DefinitionManager : public IDefinitionManager
    {
    public:
        DefinitionManager() {}
        ~DefinitionManager() {}

        Definition* Create(const char* pchFilename);
		Result Release(Definition*& pDefinition);

		U32 Count() { return m_apDefinitions.Size(); }
		
		Core::Lock* GetLock() { return &m_Lock; }

		void DumpStats();
		void RenderStats();

	private:
        Definition* Lookup(const char* pchFilename);

		Result RemoveFromList(Definition* pDefinition);
        Result Destroy(Definition*& pDefinition);

        Core::Array<Definition*> m_apDefinitions;

		Core::Lock m_Lock;
	};
}

#endif // _DEFINITIONMANAGER_H_

