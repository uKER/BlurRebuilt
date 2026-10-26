#ifndef AFTK_LIMIT_MIXIN_H
#define AFTK_LIMIT_MIXIN_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace Aftk
{
	class C_NativeLimitMixin;

	class C_LimitMixin
	{
	public:
		C_LimitMixin();
		C_LimitMixin(const C_LimitMixin& rhs);
		~C_LimitMixin();

		const C_LimitMixin& operator=(const C_LimitMixin& rhs);

		void SetIsLimited(E_LimitType type, bool isLimited);

		void SetLimit(E_LimitType type, F32 limit);

		bool IsLimited(E_LimitType type) const;

		F32 GetLimit(E_LimitType type) const;

	protected:
		void OneTimeInit();

		F32 m_limits[E_LimitType_Count];
		bool m_isLimited[E_LimitType_Count];

		FRIEND_CLASS C_NativeLimitMixin;
	};
}

#endif
