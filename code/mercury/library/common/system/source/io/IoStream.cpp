#include "SystemPch.h"
#include "IoImpl.h"

class IoStream
{
public:
	bool Create(int bufferSize, int bufferCount);
	void Destroy();

public:
	bool HasAvailableBuffers();

public:
	int Request(const IoFile& file, U64 offset, IoStreamCallback* callback);
	void Release(int bufferIndex);
};

class IoRiver
{
public:
	IoRiver(float bufferWeight);
};


IoStreamManager m;

m.Initialise(20 * 1024, 20);

IoStream s = m.CreateStream(file, offsetStart, offsetEnd, bufferWeight);

s.Prime();
s.Start();
s.Stop();

U8* buf = s.NextBuffer(); // Returns null if no buffer available...? auto releases last buffer, ReleaseBuffer() can be called explicitly if nessecary?
