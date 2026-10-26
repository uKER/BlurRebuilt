#include "stdafx.h"

#include <converter_shared/ProcessGraph.h>
#include <data/DataTypes.h>
#include <data/AdvertisingData.h>

#include <Advertising/AdvertResource.h>

using namespace Advertising;

C_DataAdvertising::C_DataAdvertising()
{
}

C_DataAdvertising::~C_DataAdvertising()
{
}

const Util::C_String C_DataAdvertising::GetName() const
{
	return Util::C_String("C_DataAdvertising");
}

void C_DataAdvertising::AddAdvertQuad(const Advertising::AdvertResource::AdvertQuad &quad)
{
	m_AdvertResource.quadArray.PushBack(quad);
}

const Advertising::AdvertResource& C_DataAdvertising::GetAdvertResource() const
{
	return m_AdvertResource;
}

void C_DataAdvertising::SortAdvertQuads()
{
	//sort ad objects by route & placement index
	//std::sort(m_AdvertResource.quadArray.Begin(), m_AdvertResource.quadArray.End());
}

bool C_DataAdvertising::Export(Sys::I_File *pFile) const
{
	Sys::BinarySerialiserAutoEndian ar(*pFile);
	ar << m_AdvertResource;

	return true;
}
