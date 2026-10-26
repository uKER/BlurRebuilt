#include "stdafx.h"
#include <animation/util/DebugRender.h>
#include <animation/pose/Pose.h>
#include <animation/common/JointTransform.h>
#include <animation/common/JointTransform.inl>
#include <animation/common/StringId.h>
#include <animation/common/Stats.h>
#include <animation/skeleton/Skeleton.h>
#include <animation/task/TaskManager.h>
#include "../cycle/CycleMapping.h"
#include <system/debug/I_DebugRender.h>
#include <btl/utility/string.h>


namespace Animation
{

#ifndef CFG_GOLD

void DebugRenderPoseTransforms(
	DbUtil::I_DebugRender* pDebugRender,
	const C_Pose& pose,
	const C_Skeleton* skeleton,
	const Maths::Mat43& worldMatrix,
	bool renderTransforms,
	bool renderJointNames,
	bool renderBones,
	const Maths::RGBA& boneColour,
	F32 jointAxisScale
)
{
	U32 numJoints = pose.GetNumJointTransforms();
	U32 numSkeletonJoints = 0;
	
	if (skeleton != NULL)
	{
		numSkeletonJoints = skeleton->GetNumJoints();
	}
	
	for (U32 i = 0; i < numJoints; ++i)
	{
		Maths::Mat43 jointMatrix;
		jointMatrix = pose.GetModelJointTransform(i, skeleton).AsMat43() * worldMatrix;
		
		if (renderTransforms)
		{
			DebugRenderJointMatrix(pDebugRender, jointMatrix, jointAxisScale);
		}
		
		if (skeleton != NULL && renderBones && i < numSkeletonJoints)
		{
			S32 parentIndex = skeleton->GetJointParentIndex(i);
			if (parentIndex >= 0)
			{
				Maths::HVec3 jointPos = Maths::HVec3(Maths::RowW(jointMatrix));
				Maths::HVec3 parentJointPos = Maths::HVec3(pose.GetModelJointTransform(parentIndex, skeleton).GetPosition() * worldMatrix);
				pDebugRender->Draw3dLine(parentJointPos, jointPos, boneColour);
			}
		}
		
		if (renderJointNames)
		{
			pDebugRender->Draw3dText(Maths::HVec3(Maths::RowW(jointMatrix)), pose.GetJointTransformName(i).GetString());
		}
	}
}

void DebugRenderJointMatrix(DbUtil::I_DebugRender* pDebugRender, const Maths::Mat43& jointMatrix, F32 size)
{
	static const Maths::RGBA kRed(255, 0, 0);
	static const Maths::RGBA kGreen(0, 255, 0);
	static const Maths::RGBA kBlue(0, 0, 255);
	
	Maths::HF32 scale(size);
	
	Maths::HMat43 m(jointMatrix);
	
	Maths::HVec3 o = m.Translation();
	Maths::HVec3 dx = m.AX();
	Maths::HVec3 dy = m.AY();
	Maths::HVec3 dz = m.AZ();
		
	pDebugRender->Draw3dLine(o, o + dx * scale, kRed);
	pDebugRender->Draw3dLine(o, o + dy * scale, kGreen);
	pDebugRender->Draw3dLine(o, o + dz * scale, kBlue);
}

#ifdef ANIMATION_STATS_ENABLED

DECLARE_DEBUG_BOOL(g_DebugShowStats,	"Animation.Stats.Show", false);
DECLARE_DEBUG_FLOAT(g_DebugOffsetX,		"Animation.Stats.OffsetX", 60.f);
DECLARE_DEBUG_FLOAT(g_DebugOffsetY,		"Animation.Stats.OffsetY", 150.f);
DECLARE_DEBUG_FLOAT(g_DebugSize,		"Animation.Stats.Size", 20.f);
DECLARE_DEBUG_FLOAT(g_DebugColumnOffset,"Animation.Stats.ColumnOffset", 250.f);
DECLARE_DEBUG_FLOAT(g_DebugRowOffset,"Animation.Stats.RowOffset", 250.f);

static void DebugRenderCycleMappingStats(DbUtil::I_DebugRender* pDebugRender, const Maths::HVec2& startPos, F32 size )
{
	Maths::HVec2 pos = startPos;
	Maths::HVec2 lineOffset(0.f, size);
	Maths::HVec2 indent(size * 2.f, 0.f);
	Maths::RGBA col(0xff, 0xff, 0xff, 0xff);

	const C_Stats& mappingStats = Animation::gStats;

	Btl::String128 str;
	pDebugRender->Draw2dText(pos, "Cycle Mapping Stats", col, size);
	pos += lineOffset * 1.5f;
	pDebugRender->Draw2dText(pos, str.Printf("Active: %d", mappingStats.numActiveCycleMappings).Chars(), col, size);
	pos += lineOffset;
	pDebugRender->Draw2dText(pos, str.Printf("Cached: %d", mappingStats.numCachedCycleMappings).Chars(), col, size);
	pos += lineOffset;
	pDebugRender->Draw2dText(pos, str.Printf("Total: %d", mappingStats.numCachedCycleMappings + mappingStats.numActiveCycleMappings).Chars(), col, size);
	pos += lineOffset;
	pDebugRender->Draw2dText(pos, str.Printf("Invalid: %d", mappingStats.numInvalidCycleMappings).Chars(), col, size);
	pos += lineOffset;

	pDebugRender->Draw2dText(pos, str.Printf("Total Memory Used: %.1fK", (F32)mappingStats.memoryUsedByCycleMappings / 1024.f).Chars(), col, size);
	pos += lineOffset;

	pDebugRender->Draw2dText(pos, str.Printf("Cache Memory Used: %.1fK (%.1f%%)", (F32)mappingStats.cycleMappingCacheMemoryUsed / 1024.f, 100.f * mappingStats.cycleMappingCacheMemoryUsed / (F32)mappingStats.memoryUsedByCycleMappings).Chars(), col, size);
	pos += lineOffset;
	pDebugRender->Draw2dText(pos, str.Printf("Cache Memory Limit: %.1fK", (F32)mappingStats.cycleMappingCacheMemoryLimit / 1024.f).Chars(), col, size);
	pos += lineOffset;
	
	// Bar representing ratio of cached to active cycle mapping memory.
	F32 barWidth = 8.f * size;
	pDebugRender->Draw2dBar(pos, Maths::HVec2(barWidth, size), Maths::RGBA(0x80, 0x80, 0x80));
	pDebugRender->Draw2dBar(pos, Maths::HVec2(barWidth * (mappingStats.cycleMappingCacheMemoryUsed / (F32)mappingStats.memoryUsedByCycleMappings), size), Maths::RGBA(0x30, 0xdd, 0x00));
	pos += lineOffset;
}

static void DebugRenderPoseStats(DbUtil::I_DebugRender* pDebugRender, const Maths::HVec2& startPos, F32 size )
{
	Maths::HVec2 pos = startPos;
	Maths::HVec2 lineOffset(0.f, size);
	Maths::HVec2 indent(size * 2.f, 0.f);
	Maths::RGBA col(0xff, 0xff, 0xff, 0xff);

	const C_Stats& stats = gStats;

	Btl::String128 str;
	pDebugRender->Draw2dText(pos, "Pose Stats", col, size);
	pos += lineOffset * 1.5f;
	pDebugRender->Draw2dText(pos, str.Printf("Active: %d", stats.numPoses).Chars(), col, size);
	pos += lineOffset;
	pDebugRender->Draw2dText(pos, str.Printf("C_Pose Memory: %.1fK", (stats.numPoses * sizeof(C_Pose)) / 1024.f).Chars(), col, size);
	pos += lineOffset;
	pDebugRender->Draw2dText(pos, str.Printf("Data Memory: %.1fK", stats.memoryUsedByPoseData / 1024.f).Chars(), col, size);
	pos += lineOffset;
	pDebugRender->Draw2dText(pos, str.Printf("Total Memory: %.1fK", (stats.memoryUsedByPoseData + stats.numPoses * sizeof(C_Pose)) / 1024.f).Chars(), col, size);
	pos += lineOffset;

	// Bar representing ratio of C_Pose memory to pose data memory.
	F32 barWidth = 8.f * size;
	pDebugRender->Draw2dBar(pos, Maths::HVec2(barWidth, size), Maths::RGBA(0x80, 0x80, 0x80));
	F32 poseMem = static_cast<F32>(stats.numPoses * sizeof(C_Pose));
	pDebugRender->Draw2dBar(pos, Maths::HVec2(barWidth * (poseMem / (poseMem + stats.memoryUsedByPoseData)), size), Maths::RGBA(0x30, 0xdd, 0x30));

	pos += lineOffset;
}

static void DebugRenderTaskStats(DbUtil::I_DebugRender* pDebugRender, const Maths::HVec2& startPos, F32 size)
{
	Maths::HVec2 pos = startPos;
	Maths::HVec2 lineOffset(0.f, size);
	Maths::HVec2 indent(size * 2.f, 0.f);
	Maths::RGBA col(0xff, 0xff, 0xff, 0xff);

	const C_Stats& stats = gStats;

	Btl::String128 str;
	pDebugRender->Draw2dText(pos, "Evaluation Task Stats", col, size);
	pos += lineOffset * 1.5f;
	pDebugRender->Draw2dText(pos, str.Printf("Immediate Tasks: %d", stats.numImmediateTasks).Chars(), col, size);
	pos += lineOffset;
	pDebugRender->Draw2dText(pos, str.Printf("Threaded Tasks: %d", stats.numThreadedTasks).Chars(), col, size);
	pos += lineOffset;
	pDebugRender->Draw2dText(pos, str.Printf("Total Tasks: %d", stats.numImmediateTasks + stats.numThreadedTasks).Chars(), col, size);
	pos += lineOffset;
	pDebugRender->Draw2dText(pos, str.Printf("Flushes: %d", stats.numFlushes).Chars(), col, size);
	pos += lineOffset;
	pDebugRender->Draw2dText(pos, str.Printf("Max Threaded Tasks: %d", stats.maxThreadedTasks).Chars(), col, size);
	pos += lineOffset;
	pDebugRender->Draw2dText(pos, str.Printf("Total Batches: %d", stats.numBatches).Chars(), col, size);
	pos += lineOffset;
	pDebugRender->Draw2dText(pos, str.Printf("Memory Used: %d", stats.memoryUsedByTaskManager).Chars(), col, size);
	pos += lineOffset;

}

static void DebugRenderCycleStats(DbUtil::I_DebugRender* pDebugRender, const Maths::HVec2& startPos, F32 size)
{
	Maths::HVec2 pos = startPos;
	Maths::HVec2 lineOffset(0.f, size);
	Maths::HVec2 indent(size * 2.f, 0.f);
	Maths::RGBA col(0xff, 0xff, 0xff, 0xff);

	const C_Stats& stats = gStats;

	Btl::String128 str;
	pDebugRender->Draw2dText(pos, "Cycle Stats", col, size);
	pos += lineOffset * 1.5f;
	pDebugRender->Draw2dText(pos, str.Printf("Total Cycles: %d", stats.numCycles).Chars(), col, size);
	pos += lineOffset;
	pDebugRender->Draw2dText(pos, str.Printf("Memory Used: %.1fK", stats.memoryUsedByCycles / 1024.f).Chars(), col, size);
	pos += lineOffset;
}

#endif

void DebugRenderStats(DbUtil::I_DebugRender* pDebugRender)
{
#ifdef ANIMATION_STATS_ENABLED
	
	if (!g_DebugShowStats) return;
	
	Maths::HVec2 pos((F32) g_DebugOffsetX, (F32) g_DebugOffsetY);
	Maths::RGBA col(0xff, 0xff, 0xff, 0xff);

	Maths::HVec2 rowStart = pos;

	DebugRenderCycleMappingStats(pDebugRender, pos, g_DebugSize);
	pos += Maths::HVecX((F32) g_DebugColumnOffset);
	DebugRenderPoseStats(pDebugRender, pos, g_DebugSize);
	pos += Maths::HVecX((F32) g_DebugColumnOffset);
	DebugRenderTaskStats(pDebugRender, pos, g_DebugSize);

	rowStart += Maths::HVecY((F32) g_DebugRowOffset);
	pos = rowStart;
	DebugRenderCycleStats(pDebugRender, pos, g_DebugSize);
#endif

}




#endif

} // end Animation
