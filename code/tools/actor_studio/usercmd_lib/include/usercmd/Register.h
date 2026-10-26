#ifndef EDITOR_USERCMD_REGISTER_H
#define EDITOR_USERCMD_REGISTER_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace EditorUserCmd
{
	class C_Register : public Ut::Singleton< C_Register, Ut::CreateUsingNew >
	{
	public:
		C_Register();
		~C_Register();

		EditorLib::C_Status Initialise(const Btl::String& projectName);
		void Shutdown();

	private:
		C_Register(const C_Register& rhs);
		const C_Register& operator=(const C_Register& rhs);
	};
}

#endif
