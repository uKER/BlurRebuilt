#include "SharedActorsPch.h"
#include <shared_actors/utility/ModelResourceUsage.h>

#include <core/containers/genesis_containers/Sort.h>
#include <game_debug/ModelResourceUsageUtils.h>
#include <game_resource/ModelResource.h>
#include <world_render/ModelInstance.h>
#include <world_render/RenderModelData.h>
#include <system/debug/I_DebugRender.h>


namespace ModelResourceUsage_Impl
{

struct C_TextureSizeSortItem
{
	U32											m_uSizeInBytes;
	const WorldRender::RenderModelTextureInfo*	m_pTextureInfo;

	inline bool operator > (const C_TextureSizeSortItem& rhs) const {return m_uSizeInBytes < rhs.m_uSizeInBytes;}
};

typedef Btl::Vector<C_TextureSizeSortItem> C_TextureSizeSortItems;

inline U32 BytesToKb(const U32 bytes) {return (bytes + 1023) / 1024;}

} // namespace ModelResourceUsage_Impl




C_ModelResourceUsage::C_ModelResourceUsage()
	: m_WorldRenderUsage()
{
}

C_ModelResourceUsage::~C_ModelResourceUsage()
{
}

void C_ModelResourceUsage::AddModelResource(const WorldRender::C_RenderModelData* pRenderModelData, const S32 modelIndex)
{
	if (pRenderModelData != NULL)
	{
		pRenderModelData->AccumulateResourceUsage(m_WorldRenderUsage, modelIndex);
	}
}

void C_ModelResourceUsage::AddModelInstance(const WorldRender::C_ModelInstance* pModelInstance)
{
	if (pModelInstance != NULL)
	{
		pModelInstance->AccumulateResourceUsage(m_WorldRenderUsage);
	}
}

U32 C_ModelResourceUsage::CalcTotalGraphicsBytes() const
{
	U32 total = 0;

	for (WorldRender::RenderModelTextureInfoList::ConstIterator it = m_WorldRenderUsage.textureInfoList.Begin(); it != m_WorldRenderUsage.textureInfoList.End(); ++it)
	{
		total += it->GetValue().uSizeInBytes;
	}

	return total;
}

void C_ModelResourceUsage::DebugRender(DbUtil::I_DebugRender& debugRender) const
{
	using namespace ModelResourceUsage;
	using namespace ModelResourceUsage_Impl;

	const Maths::HVec2 origin(100.0f, 100.0f);
	const Maths::RGBA colour(1.0f, 1.0f, 1.0f, 1.0f);

	const F32 lineHeight = 24.0f;
	const Maths::HVec2 lineSpacing(0.0f, lineHeight);

	C_TextureSizeSortItems textureSizeSortItems;
	textureSizeSortItems.Reserve(m_WorldRenderUsage.textureInfoList.Size());

	// show texture totals
	{
		U32 total = 0;
		C_TextureTotals subTotals;
		memset(&subTotals, 0, sizeof(subTotals));

		for (WorldRender::RenderModelTextureInfoList::ConstIterator it = m_WorldRenderUsage.textureInfoList.Begin(); it != m_WorldRenderUsage.textureInfoList.End(); ++it)
		{
			total += it->GetValue().uSizeInBytes;
			AccumulateTotals(subTotals, it->GetValue().textureName, it->GetValue().uSizeInBytes, it->GetValue().eFormat);

			{
				C_TextureSizeSortItem textureSizeSortItem;
				textureSizeSortItem.m_uSizeInBytes = it->GetValue().uSizeInBytes;
				textureSizeSortItem.m_pTextureInfo = &it->GetValue();
				textureSizeSortItems.PushBack(textureSizeSortItem);
			}
		}

		Maths::HVec2 pos = origin;
		const Maths::HVec2 columnOffset(100.0f, 0.0f);
		Util::C_String text;

		text.Printf("%d Kb", BytesToKb(total));
		debugRender.Draw2dText(pos, text, colour, lineHeight);
		debugRender.Draw2dText(pos + columnOffset, "Textures", colour, lineHeight);
		pos += lineSpacing;

		for (U32 i = 0; i < E_TextureType_Count; ++i)
		{
			text.Printf("%d Kb", BytesToKb(subTotals[i]));
			debugRender.Draw2dText(pos, text, colour, lineHeight);
			debugRender.Draw2dText(pos + columnOffset, ModelResourceUsage::GetTextureTypeName(static_cast<E_TextureType>(i)), colour, lineHeight);
			pos += lineSpacing;
		}
	}

	// show texture names
	{
		Util::ShellSortArray<C_TextureSizeSortItems, C_TextureSizeSortItem>(textureSizeSortItems, textureSizeSortItems.Size());

		Maths::HVec2 pos = origin + Maths::HVec2(300.0f, 0.0f);

		Util::C_String text;
		for (C_TextureSizeSortItems::ConstIterator it = textureSizeSortItems.Begin(); it != textureSizeSortItems.End(); ++it)
		{
			const WorldRender::RenderModelTextureInfo* pTextureInfo = it->m_pTextureInfo;
			const char* sharedString = (pTextureInfo->flags & WorldRender::RenderModelTextureInfo::E_Flag_Shared) ? "* " : "";
			text.Printf("%d Kb ( %d x %d x %d ) %s%s", BytesToKb(pTextureInfo->uSizeInBytes), pTextureInfo->uWidth, pTextureInfo->uHeight, pTextureInfo->uLevels, sharedString, pTextureInfo->textureName.Chars());
			debugRender.Draw2dText(pos, text, colour, lineHeight);
			pos += lineSpacing;
		}
	}
}

void C_ModelResourceUsage::DebugDump(Debug::Logger& debugLogger) const
{
	using namespace ModelResourceUsage_Impl;

	C_TextureSizeSortItems textureSizeSortItems;
	textureSizeSortItems.Reserve(m_WorldRenderUsage.textureInfoList.Size());

	for (WorldRender::RenderModelTextureInfoList::ConstIterator it = m_WorldRenderUsage.textureInfoList.Begin(); it != m_WorldRenderUsage.textureInfoList.End(); ++it)
	{
		C_TextureSizeSortItem textureSizeSortItem;
		textureSizeSortItem.m_uSizeInBytes = it->GetValue().uSizeInBytes;
		textureSizeSortItem.m_pTextureInfo = &it->GetValue();
		textureSizeSortItems.PushBack(textureSizeSortItem);
	}

	Util::ShellSortArray<C_TextureSizeSortItems, C_TextureSizeSortItem>(textureSizeSortItems, textureSizeSortItems.Size());

	for (C_TextureSizeSortItems::ConstIterator it = textureSizeSortItems.Begin(); it != textureSizeSortItems.End(); ++it)
	{
		const WorldRender::RenderModelTextureInfo* pTextureInfo = it->m_pTextureInfo;
		const char* sharedString = (pTextureInfo->flags & WorldRender::RenderModelTextureInfo::E_Flag_Shared) ? "* " : "";
		LOGINFO(debugLogger, "%d ( %d x %d x %d ) %s%s", pTextureInfo->uSizeInBytes, pTextureInfo->uWidth, pTextureInfo->uHeight, pTextureInfo->uLevels, sharedString, pTextureInfo->textureName.Chars());
	}
}

void C_ModelResourceUsage::DebugDump(Sys::I_File& file) const
{
	Util::C_String text;
	for (WorldRender::RenderModelTextureInfoList::ConstIterator it = m_WorldRenderUsage.textureInfoList.Begin(); it != m_WorldRenderUsage.textureInfoList.End(); ++it)
	{
		const char* sharedString = (it->GetValue().flags & WorldRender::RenderModelTextureInfo::E_Flag_Shared) ? "* " : "";
		text.Printf("%d,%d,%d,%d,%s,%s\n", it->GetValue().uSizeInBytes, it->GetValue().uWidth, it->GetValue().uHeight, it->GetValue().uLevels, sharedString, it->GetValue().textureName.Chars());
		file.Write(text.Chars(), text.Length());
	}
}
