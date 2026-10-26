#ifndef FILECOPY_LIST_XML
#define FILECOPY_LIST_XML

#include "system\xml\XMLParser.h"
#include "FileCopyList.h"

namespace Tools
{
	namespace FileCopy
	{
		class C_XmlElement;
		class C_FileCopyList;
		typedef Ut::UtVector<Ut::DynamicVector<C_XmlElement*> > XmlElementList;

		class C_XmlElement
		{
		private:

			class C_RefCount
			{
			public:
				C_RefCount() : counter(1) {}
				inline U32 AddRef()
				{
					++counter;
					return counter;
				}
				inline U32 DecRef()
				{
					--counter;
					return counter;
				}
				inline U32 GetCount() const { return counter; }
			private:
				U32 counter;
			};

		public:
			///\Brief Default constructor
			C_XmlElement();

			///\Brief Destructor
			virtual ~C_XmlElement();

			///\Brief Get name of element
			///
			///@return Name of element
			Util::C_String& GetName();

			///\Brief Get name of element
			///
			///@return Name of element
			const Util::C_String& GetName() const;

			///\Brief Get name of element
			///@param name New name of element
			void SetName(const Util::C_String &name);

			///\Brief Get attribute list
			///
			///@return attribute list
			Sys::C_XmlAttributeList& GetAttributeList();

			///\Brief Get attribute list
			///
			///@return attribute list
			const Sys::C_XmlAttributeList& GetAttributeList() const;

			///\Brief Set attribute list
			///
			void SetAttributeList(const Sys::C_XmlAttributeList &list);

			///\Brief Get child list
			///
			///@return child list
			XmlElementList& GetChildList();

			///\Brief Get child list
			///
			///@return child list
			const XmlElementList& GetChildList() const;

			///\Brief Get reference count
			///
			///@return reference count
			U32 GetReferenceCount() const
			{
				return m_RefCount.GetCount();
			}

			///\Brief increment reference count
			U32 AddRef()
			{
				return m_RefCount.AddRef();
			}

			///\Brief decrement reference count & delete object if no longer referenced
			U32 DecRef()
			{
				if (m_RefCount.DecRef() == 0)
				{
					delete this;
					return 0;
				}
				return m_RefCount.GetCount();
			}
			
		private:

			Util::C_String m_Name;
			Sys::C_XmlAttributeList m_Attributes;
			XmlElementList m_Children;
			C_RefCount m_RefCount;
		};


		class C_FileCopyListXml : public Sys::C_XmlContent
		{
		public:
			///\Brief Default Constructor
			C_FileCopyListXml();

			///\Brief Destructor
			virtual ~C_FileCopyListXml();

			///\Brief Default Constructor
			void Clear();

			///\Brief Used by Sys::XmlParse()
			///
			///@return bool
			virtual bool StartDocument();

			///\Brief Used by Sys::XmlParse()
			///
			///@return bool
			virtual bool EndDocument();

			///\Brief Used by Sys::XmlParse()
			///
			///@return bool
			virtual bool StartElement(const Util::C_String& name, const Sys::C_XmlAttributeList& attributes);

			///\Brief Used by Sys::XmlParse()
			///
			///@return bool
			virtual bool EndElement(const Util::C_String& name);

			///\Brief Populates contents from FileCopyList
			///
			///@param list Reference to list to read from
			///@return bool
			bool ReadFromFileCopyList(
				const C_FileCopyList &list,
				const Ut::UtVector<Ut::ArrayVector<FileCopy::C_PlatformCopyLocations, Util::E_Platform_Count> >& 
					locations);

			///\Brief Generates FileCopyList from contents
			///
			///@param list Reference to list to be populated
			///@return bool
			bool CopyToFileCopyList(C_FileCopyList &list) const;

			///\Brief Get root node of xml
			///
			///@return C_XmlElement
			C_XmlElement* GetRoot();

			///\Brief Get root node of xml
			///
			///@return C_XmlElement
			const C_XmlElement* GetRoot() const;

		private:

			C_XmlElement *m_pRootElement;
			Ut::UtList<C_XmlElement*>	m_Stack;
		};

		bool XmlWrite(const C_FileCopyListXml& content, const char* document);
	}
}
#endif
