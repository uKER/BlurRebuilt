
#ifndef BOXWORLD_H
#define BOXWORLD_H

namespace Sys
{
	class I_BinaryStream;
};

namespace Framework
{

class C_BoxCar;
class C_BoxWorld
{
public:
	C_BoxWorld();
	~C_BoxWorld();
	void Update(F32 period);
	C_BoxCar* GetCar(S32 index = 0);
	void Render();
	void Serialise(Sys::I_BinaryStream& stream);
	void Deserialise(Sys::I_BinaryStream& stream);

private:
	Ut::Vector<C_BoxCar> m_carList;
};

extern C_BoxWorld g_BoxWorld;

}

#endif
