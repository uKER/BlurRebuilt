#ifndef CRYPTRANDOM_H
#define CRYPTRANDOM_H

#ifdef _MSC_VER
#pragma once
#endif

class CryptRandom
{
public:
	CryptRandom();
	~CryptRandom();

public:
	bool Initialise();
	void Finalise();

public:
	bool Generate(void* data, int length);

public:
	XtU64 operator()();

private:
	HCRYPTPROV _provider;
};

#endif // CRYPTRANDOM_H