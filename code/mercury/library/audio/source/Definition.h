#ifndef _DEFINITION_H_
#define _DEFINITION_H_


namespace Audio
{
    class DefinitionManager;
	class Object;
////////////////////////////////////////////////////////////////////////////////
//	Definition
////////////////////////////////////////////////////////////////////////////////
    class Definition// : public IDefinition
    {
    public:
#ifndef PLATFORM_SPU
		static void* operator new(size_t sz);
		static void operator delete(void* ptr);
#endif

	public:
        Definition();
        ~Definition();

        Result Init(const char* pchName, void* pData, U32 u32Size);

		const Util::C_String& FileName() const; 
		inline const char* Name() const { return m_pchObjectName; }
        inline const char* StringTable() const { return m_pchStringTable; }
        inline U32 FrameSkip() { return m_uFrameSkip; }
        inline U32 NumComponents() { return m_uNumComponents; }
        inline U32* ComponentData() { return m_pComponentData; }
		inline void* Data() { return m_pData; }
		inline U32 RefCount() { return m_uRefCount; }
		inline void IncRefCount() { m_uRefCount++; }
		inline void DecRefCount() { Assert(m_uRefCount != 0); m_uRefCount--; }
#ifdef NO_STRING_TABLE_ON_SPU
		inline U32 GetSpuDataSize() { return m_u32DataSizeSpu; }
#endif
		inline U32 GetDataSize() { return m_u32DataSize; }
		inline void SetObject(Object* pObject) { m_pObject = pObject; }
		inline Object* GetObject() { return m_pObject; }
    private:
		Object* m_pObject;
		Util::C_String m_strFilename;
        void* m_pData;
		U32 m_u32DataSize;
#ifdef NO_STRING_TABLE_ON_SPU
		U32 m_u32DataSizeSpu;
#endif
		U32 m_uRefCount;				// how many load requests has this object had?
        const char* m_pchStringTable;
        U32 m_uNumComponents;
        const char* m_pchObjectName;
		U32 m_uFrameSkip;
		U32* m_pComponentData;
    };
}

#endif // _DEFINITION_H_

