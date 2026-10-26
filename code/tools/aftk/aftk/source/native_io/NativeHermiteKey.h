#ifndef NATIVE_HERMITE_KEY_H
#define NATIVE_HERMITE_KEY_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace Aftk
{
	class C_NativeHermiteKey : public C_NativeBaseKey
	{
	public:
		static void Serialise(
			C_NativeDiskWriter* pDiskWriter, 
			const C_HermiteKey& key)
		{
			C_NativeBaseKey::Serialise(
				pDiskWriter,
				key);

			*pDiskWriter << key.m_inTangentAngle;
			*pDiskWriter << key.m_outTangentAngle;
			*pDiskWriter << key.m_inTangentType;
			*pDiskWriter << key.m_outTangentType;
		}

		static void Deserialise(
			C_NativeDiskReader* pDiskReader, 
			C_NativeFtk* pNativeFtk, 
			C_HermiteKey& key)
		{
			C_NativeBaseKey::Deserialise(
				pDiskReader,
				key);

			*pDiskReader >> key.m_inTangentAngle;
			*pDiskReader >> key.m_outTangentAngle;
			if (pDiskReader->GetVersion() >= 211)
			{
				*pDiskReader >> (U32&)key.m_inTangentType;
				*pDiskReader >> (U32&)key.m_outTangentType;
			}
			else
			{
				key.m_inTangentType = E_CurveTangentType_Unknown;
				key.m_outTangentType = E_CurveTangentType_Unknown;
			}
		}
	};
}
#endif
