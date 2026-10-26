#ifndef TOOLS_CRYPT_RANDOM_H
#define TOOLS_CRYPT_RANDOM_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Tools
{
	class C_CryptRandom
	{
	public:
		C_CryptRandom();
		~C_CryptRandom();

	public:
		bool Initialise();
		void Finalise();

	public:
		bool Generate(void* data, int length);

	public:
		U64 operator()();

	private:
		HCRYPTPROV _provider;
	};
}

#endif