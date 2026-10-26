#ifndef LOGICALFS_PAKFS_PAK_PACKET_COMPRESS_H
#define LOGICALFS_PAKFS_PAK_PACKET_COMPRESS_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Pak
{
	void SetPacketCompressionPolicy(S32	policy);
	S32 GetPacketCompressionPolicy();
}

#endif
