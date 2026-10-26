#pragma once

struct Wrap32_XMemCreateCompressionContextArguments
{
	UINT							codecType;
	WRAP32_SHARED_MEMORY_OFFSET		pCodecParams;
	UINT							flags;
	WRAP32_FAR_POINTER				pContext;
};

struct Wrap32_XMemCompressArguments
{
	WRAP32_FAR_POINTER				pContext;
	WRAP32_SHARED_MEMORY_OFFSET		pDestination;
	WRAP32_SHARED_MEMORY_OFFSET		pDestSize;
	WRAP32_SHARED_MEMORY_OFFSET		pSource;
	UINT							srcSize;
};

struct Wrap32_XMemDestroyCompressionContextArguments
{
	WRAP32_FAR_POINTER				pContext;
};

