#ifndef DMAN_NAME_HELPERS_H
#define DMAN_NAME_HELPERS_H

#ifdef _MSC_VER
#pragma once
#endif

namespace DMan
{
	class C_MacroExpander;

	namespace NameHelpers
	{
		struct C_Conversation
		{
			C_Conversation() {}
			C_Conversation(const C_Conversation& rhs) { *this = rhs; }
			const C_Conversation& operator=(const C_Conversation& rhs)
			{
				firstRowUid = rhs.firstRowUid;
				context = rhs.context;
				rowIndices = rhs.rowIndices;
				return *this;
			}

			U32 firstRowUid;
			Btl::String context;
			Btl::Vector<U32> rowIndices;
		};

		//ConversationGroupId->Conversation
		typedef Btl::Map<Btl::String, C_Conversation> ConversationGroups;
		//Row->ConversationGroupId
		typedef Btl::Map<U32, Btl::String> ConversationGroupMapping;

		Btl::OpenHashSet<Character> GetCharactersInRecord(C_Record* pRecord);

		void BuildConversationGroups(
			ConversationGroups& groups, 
			ConversationGroupMapping& groupMapping,
			C_RowSet* pRowSet);

		void AddFieldValue(C_MacroExpander& expander, 
			C_ColumnSet* pColumnSet, C_Row* pRow);

		Util::C_String GetFullWavFileName(C_MacroExpander& expander);

		bool GenerateNames(
			C_Record* pDialogueRecord,
			C_RowSet* pRowSet,
			const ConversationGroups& groups, 
			const ConversationGroupMapping& groupMapping, 
			const wxString& format);

		bool GenerateWavNames(
			C_Record* pDialogueRecord,
			C_RowSet* pRowSet,
			const ConversationGroups& groups, 
			const ConversationGroupMapping& groupMapping, 
			const wxString& format);
	}
}

#endif
