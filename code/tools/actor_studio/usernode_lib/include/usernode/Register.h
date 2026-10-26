#ifndef EDITOR_PROPS_REGISTER_H
#define EDITOR_PROPS_REGISTER_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace UserNode
{
	class C_Register : public Ut::Singleton< C_Register, Ut::CreateUsingNew >
	{
	public:
		C_Register();
		~C_Register();

		void Initialise();
		void Shutdown();

	private:
		C_Register(const C_Register& rhs);
		const C_Register& operator=(const C_Register& rhs);
	};
}

#endif
