#ifndef	ANIMATEDSHAPECONTROLLER_H
#define	ANIMATEDSHAPECONTROLLER_H

namespace Collision
{
	class C_CompositeShape;
}

namespace Gravity
{
	class C_Scene;
	class C_Body;

	class ALIGN16 C_AnimatedShapeController 
	{
	public:
		C_AnimatedShapeController(Gravity::C_Body* pBody = NULL);
		
		~C_AnimatedShapeController();

		void Activate();
		void Deactivate();
		void UpdateTransform(Collision::C_CompositeShape* pCompShape);

		bool operator < (const C_AnimatedShapeController& controller);
		bool operator <= (const C_AnimatedShapeController& controller);
		bool operator > (const C_AnimatedShapeController& controller);
		bool operator >= (const C_AnimatedShapeController& controller);
		bool operator == (const C_AnimatedShapeController& controller);
		bool operator != (const C_AnimatedShapeController& controller);

		Collision::C_CompositeShape* GetCompositeShape();
		void SetScene(C_Scene* pScene);
		C_Body* GetBody();

		inline void SetTransformMatrix(const Maths::HMat43 PCREF transform, const U32 index)
		{
#ifndef CFG_SPEED
			BIZ_ASSERT(index < m_nTransforms);
#endif
			m_Transforms[index] = transform;
		}

		friend class C_Scene;

	private:
		C_AnimatedShapeController* m_pPrev;
		C_AnimatedShapeController* m_pNext;
		C_Scene* m_pScene;
		C_Body* m_pBody;
		Maths::HMat43* m_Transforms;
#ifndef CFG_SPEED
		U32 m_nTransforms;
#endif
		bool m_Active;
	
	};
}

#endif