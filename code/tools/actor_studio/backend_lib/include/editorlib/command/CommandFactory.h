#pragma once

#ifndef EDITOR_LIB_COMMAND_FACTORY_H
#define EDITOR_LIB_COMMAND_FACTORY_H

namespace EditorLib
{
	class I_Command;

	class C_CommandFactory
	{
	public:
		typedef I_Command*	(*CreateFunc)(U32 type);
		typedef C_Syntax*	(*CreateSyntaxFunc)(void);

		struct CreateInfo
		{
			CreateInfo()
			{
				create = 0;
				createSyntax = 0;
				id = 0xffffffff;
			}

			CreateInfo(
				CreateFunc _create, 
				CreateSyntaxFunc _createSyntax,
				U32 _id) 
				: 
				create(_create), createSyntax(_createSyntax), id(_id) {}

			CreateFunc			create;
			CreateSyntaxFunc	createSyntax;
			U32					id;
		};

		typedef Btl::OpenHashMap<Btl::String, CreateInfo, Btl::HashKeyOpBtlString > CommandHashMap;

		C_CommandFactory(C_Status (*f)(const char* pName));
		~C_CommandFactory();

		template<class COMMAND>
		C_Status Register();

		bool Create(
			I_Command*& pCommand,
			C_Syntax*& pSyntax,
			const Btl::String& command);

		const CommandHashMap& GetCommands() const;

	private:
		typedef I_Command*	(*CreateFunc)(U32 type);
		typedef C_Syntax*	(*CreateSyntaxFunc)(void);

		CommandHashMap			m_commandMapping;
		U32						m_nextId;
		C_Status (*m_pPostRegFunc)(const char* pName);
	};

	#include "CommandFactory.inl"
}

#endif
