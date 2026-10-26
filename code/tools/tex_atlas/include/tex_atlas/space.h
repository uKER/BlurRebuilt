#ifndef CS_GEN_SPACE_H
#define CS_GEN_SPACE_H

namespace TexAtlas
{
	class Space
	{
	public:
		enum Side
		{
			TOP,
			BOTTOM,
			LEFT,
			RIGHT,
			NONE
		};

		Space();
/*
		Space(
			const Vec2S32& topLeft, 
			const Vec2S32& bottomRight);

		Space(
			U32 topLeftX, 
			U32 topLeftY,
			U32 bottomRightX,
			U32 bottomRightY);
*/
		S32 Area() const;

		bool CanContain(U32 dimensionX, U32 dimensionY) const;

		bool CanContain(const Vec2S32& dimensions) const;

		bool Intersects(const Vec2S32& p) const;

		bool Intersects(const Space& s) const;

		U32 X() const;

		U32 Y() const;

		const Vec2S32& TopLeftCorner() const;

		Vec2S32 TopRightCorner() const;

		Vec2S32 BottomLeftCorner() const;

		const Vec2S32& BottomRightCorner() const;

		Vec2S32 Dimensions() const;

		U32 Width() const;

		U32 Height() const;

		F32 Aspect() const;

		void Set(
			U32 topLeftX, 
			U32 topLeftY,
			U32 width, 
			U32 height);

		bool operator < (const Space& rhs) const;

	private:
		Vec2S32 m_topLeft;
		Vec2S32 m_bottomRight;
	};
}

#endif
