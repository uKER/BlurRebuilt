#pragma once

class Job : public BaseObject
{
	DECLARE_PERSISTENCE(Job);
};

class Result : public BaseObject
{
	DECLARE_PERSISTENCE(Result);
};

class Client : public ClientInterface
{
	DECLARE_PERSISTENCE(Client);

	void		Initialize();
	BaseObject *Process(BaseObject *pJob);
	void		CleanUp();
};