
#include "StdAfx.h"
#include <system/serialise.h>

#pragma warning (push, 4)

#include "BoxWorld.h"
#include "BoxCar.h"

namespace Framework
{

C_BoxWorld g_BoxWorld;

C_BoxWorld::C_BoxWorld()
{
	S32 i = 0;
	for (i = 0; i < 8; i++)
	{
		m_carList.PushBack();
		C_BoxCar& car = m_carList.Back();
		
		car.m_position = Maths::HVec3((F32)i * 3.f, 0.f, 0.f);
	}
}

C_BoxWorld::~C_BoxWorld()
{
	m_carList.Empty();
}

void C_BoxWorld::Update(F32 period)
{
	S32 i = 0;
	for (i = 0; i < m_carList.Size(); i++)
	{
		C_BoxCar& car = m_carList[i];
		
		if (i == 0)
			car.ReadControls();
		
		if (i >= 1)
		{
			car.m_forw = 43.f - (F32)i * 0.1f;
			car.m_steering = 0.1f;
		}
		
		car.Update(period);
	}
}

C_BoxCar* C_BoxWorld::GetCar(S32 index)
{
	if (index < 0)
		return 0;
	if (index >= m_carList.Size())
		return 0;
	
	return &m_carList[index];
}

void C_BoxWorld::Render()
{
	S32 i = 0;
	for (i = 0; i < m_carList.Size(); i++)
		m_carList[i].Render();
}

void C_BoxWorld::Serialise(Sys::I_BinaryStream& stream)
{
	Sys::BinarySerialiser<Ut::NativeToBigEndian> serial(stream);
	
	S32 i = 0;
	for (i = 0; i < m_carList.Size(); i++)
	{
		serial << m_carList[i].m_position;
		Maths::HVec4 tmp = m_carList[i].m_orient;
		serial << tmp;
	}
}

void C_BoxWorld::Deserialise(Sys::I_BinaryStream& stream)
{
	Sys::BinaryDeserialiser<Ut::BigEndianToNative> serial(stream);

	S32 i = 0;
	for (i = 0; i < m_carList.Size(); i++)
	{
		serial >> m_carList[i].m_position;
		Maths::HVec4 tmp;
		serial >> tmp;
		m_carList[i].m_orient = (Maths::C_Quaternion)tmp;
	}
}

}
