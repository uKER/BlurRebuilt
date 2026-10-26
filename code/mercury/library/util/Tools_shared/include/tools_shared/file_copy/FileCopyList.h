#ifndef FILECOPY_LIST_INCLUDE
#define FILECOPY_LIST_INCLUDE


#include "FileCopyPlatforms.h"

namespace Tools
{
	namespace FileCopy
	{
		struct C_PlatformCopyLocations
		{
			Util::C_String workingPath;
			Util::C_String copyPath;
			//Devkit doesn't necessarily mean Xbox360, concievably other consoles might allow copying directly
			//to local storage.
			Util::C_String devKit;
		};

		class C_FileCopyItem
		{
		public:

			enum E_FileCopyItemType
			{
				E_FileCopyItemType_File,
				E_FileCopyItemType_Directory
			};

		public:
			///\brief Default Constructor
			C_FileCopyItem();

			///\brief Constructor.
			///
			///@param src Source of item to be FileCopyed
			///@param dst Destination of FileCopyed item
			///@param type type of FileCopy item (single file or an entire directory)
			C_FileCopyItem(
				Util::C_String src, 
				Util::C_String dst, 
				E_FileCopyItemType type = E_FileCopyItemType_File);

			///\brief Copy Constructor
			///
			///@param item The FileCopy item to copy
			C_FileCopyItem(const C_FileCopyItem &item);

			///\brief Destructor
			~C_FileCopyItem();

			///\brief Assignment operator
			///
			///@param item The item to copy
			///@return A reference to this item
			const C_FileCopyItem& operator =(const C_FileCopyItem &item);

			///\brief Equality operator
			///
			///@param item The item to compare
			///@return bool
			bool operator ==(const C_FileCopyItem &item) const;

			///\brief Inequality operator
			///
			///@param item The item to compare
			///@return bool
			bool operator !=(const C_FileCopyItem &item) const;

		public:
			Util::C_String		m_FileCopySource;
			Util::C_String		m_FileCopyDestination;
			E_FileCopyItemType	m_FileCopyType;

		};


		class C_FileCopyOption
		{
		public:
			///\brief Default Constructor
			C_FileCopyOption();

			///\brief Constructor
			///
			///@param option
			///@param value
			C_FileCopyOption(Util::C_String option, Util::C_String value);

			///\brief Copy Constructor
			///
			///@param item The FileCopy list to copy
			C_FileCopyOption(const C_FileCopyOption &option);

			///\brief Assignment operator
			///
			///@param list The option to copy
			///@return A reference to this option
			const C_FileCopyOption& operator =(const C_FileCopyOption &option);

			///\brief Equality operator
			///
			///@param option The item to compare
			///@return bool
			bool operator ==(const C_FileCopyOption &option) const;

			///\brief Inequality operator
			///
			///@param option The item to compare
			///@return bool
			bool operator !=(const C_FileCopyOption &option) const;

		public:
			Util::C_String	m_Option;
			Util::C_String	m_Value;
		};


		typedef Ut::UtVector< Ut::DynamicVector<C_FileCopyItem> >	FileCopyItemArray;
		typedef Ut::UtVector< Ut::DynamicVector<C_FileCopyOption> >	FileCopyOptionArray;


		class C_FileCopyList
		{
		public:

			///\brief Default Constructor
			///
			C_FileCopyList();

			///\brief Copy Constructor
			///
			///@param item The FileCopy list to copy
			C_FileCopyList(const C_FileCopyList &list);

			///\brief Destructor
			~C_FileCopyList();

			///\brief Assignment operator
			///
			///@param list The list to copy
			///@return A reference to this list
			const C_FileCopyList& operator =(const C_FileCopyList &list);

			///\brief Get the array of FileCopyItems
			///
			///@return A reference to the array
			FileCopyItemArray& GetFileCopyItemArray(Util::E_Platform platform);

			///\brief Get the array of FileCopyItems
			///
			///@return A reference to this the array
			const FileCopyItemArray& GetFileCopyItemArray(Util::E_Platform platform) const;

			///\brief Adds an item to the array of FileCopyItems
			///
			///@return An index to where the item was added
			U32 AddFileCopyItem(Util::E_Platform platform, C_FileCopyItem &item);

			///\brief Gets an item already in the array of FileCopyItems
			///
			C_FileCopyItem& GetFileCopyItem(Util::E_Platform platform, U32 instance);

			///\brief Sets an item already in the array of FileCopyItems
			///
			const C_FileCopyItem& GetFileCopyItem(Util::E_Platform platform, U32 instance) const;

			///\brief Sets an item already in the array of FileCopyItems
			///
			void SetFileCopyItem(Util::E_Platform platform, U32 instance, C_FileCopyItem &item);

			///\brief Gets the number of FileCopyItems in the list
			///
			///@return The number of FileCopy items in the list
			U32 GetNumFileCopyItems(Util::E_Platform platform);

			///\brief Gets the number of FileCopyItems in the list
			///
			///@return The number of FileCopyItems in the list
			U32 GetNumFileCopyItems(Util::E_Platform platform) const;

			///\brief Get the array of FileCopyOptions
			///
			///@return A reference to the array
			FileCopyOptionArray& GetFileCopyOptionArray(Util::E_Platform platform);

			///\brief Get the array of FileCopyOptions
			///
			///@return A reference to the array
			const FileCopyOptionArray& GetFileCopyOptionArray(Util::E_Platform platform) const;

			///\brief Add a FileCopyOption to the list
			///
			///@return An index to where the item was added
			U32 AddFileCopyOption(Util::E_Platform platform, const C_FileCopyOption &option);

			///\brief Gets an item already in the array of FileCopyOptions
			///
			C_FileCopyOption& GetFileCopyOption(Util::E_Platform platform, U32 instance);

			///\brief Gets an item already in the array of FileCopyOptions
			///
			const C_FileCopyOption& GetFileCopyOption(Util::E_Platform platform, U32 instance) const;

			///\brief Gets an item already in the array of FileCopyOptions
			///
			C_FileCopyOption* FindFileCopyOptionByName(Util::E_Platform platform, const char* name);

			///\brief Gets an item already in the array of FileCopyOptions
			///
			const C_FileCopyOption* FindFileCopyOptionByName(Util::E_Platform platform, const char* name) const;

			///\brief Sets an item already in the array of FileCopyOptions
			///
			void SetFileCopyOption(Util::E_Platform platform, U32 instance, const C_FileCopyOption &option);

			///\brief Gets the number of FileCopyOptions for a particular platform
			///
			U32 GetNumFileCopyOptions(Util::E_Platform platform);

			///\brief Gets the number of FileCopyOptions for a particular platform
			///
			U32 GetNumFileCopyOptions(Util::E_Platform platform) const;

			///\brief Removes everything from out file copy list.
			///
			void ClearItems();

		private:
			Ut::UtVector< Ut::DynamicVector<FileCopyItemArray> > m_FileCopyItems;

			//Per platform copy options
			Ut::UtVector< Ut::DynamicVector<FileCopyOptionArray> > m_FileCopyOptions;

		};
	}
}

#endif