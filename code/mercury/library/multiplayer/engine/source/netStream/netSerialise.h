#ifndef NETSERIALISE_H
#define NETSERIALISE_H

#ifdef _MSC_VER
#pragma once
#endif

class netStream;

class netSerialisable
{
public:
	virtual ~netSerialisable();
	virtual bool serialise(netStream* stream)=0;
	virtual bool deserialise(netStream* stream)=0;
	virtual int serialiseSize();
};

#endif //NETSERIALISE_H
