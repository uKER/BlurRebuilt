#pragma once

class Job : public BaseObject
{
	DECLARE_PERSISTENCE(Job);

	void write(ost::Engine &ar)
	{
		for (int i=0; i<JOB_SIZE; i++)
			ar << rand();
	}
	void read(ost::Engine &ar)
	{
		int dummy;
		for (int i=0; i<JOB_SIZE; i++)
			ar >> dummy;
	}
};

class Result : public BaseObject
{
	DECLARE_PERSISTENCE(Result);

	void write(ost::Engine &ar)
	{
		for (int i=0; i<RESULT_SIZE; i++)
			ar << rand();
	}
	void read(ost::Engine &ar)
	{
		int dummy;
		for (int i=0; i<RESULT_SIZE; i++)
			ar >> dummy;
	}
};

class Client : public ClientInterface
{
	DECLARE_PERSISTENCE(Client);

	void		Initialize();
	BaseObject *Process(BaseObject *pJob);
	void		CleanUp();
};
