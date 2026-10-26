#ifndef NGLBASEDRIVER_H
#define NGLBASEDRIVER_H

class NglBaseDriver
{
public:
	virtual const char* GetPlatform() const = 0;
	virtual const char* GetName() const = 0;
	virtual bool IsBigEndian() const = 0;
};

#endif // NGLBASEDRIVER_H


