
#include "stdafx.h"

#include <System/Misc/Random.h>

#include <ui/Game/Gadgets/ButtonMessages.h>
#include <ui/Graphics/Simple2D.h>
#include <Ui/LightPaintSystem.h>
#include <Ui/UiManager.h>
#include <Ui/SoundEffects.h>

#include <Core/Maths/HVec/New/IncludeGeneral.h>
#include <Core/Maths/HVec/New/IncludeGeneral.inl>

#include <Dynamic/Particle/Event.h>
#include <Dynamic/Particle/EventInstance.h>
#include <Dynamic/Particle/EventInstanceHandle.h>
#include <Dynamic/Particle/Generic/XtInterface.h>

extern Util::C_Random gUiRand;

Debug::Float gUiLightPaintRate("Ui.LightPaint.Rate", 4.0f, Debug::Range(0.0f, 50.0f));
Debug::Float gUiLightPaintDamp("Ui.LightPaint.Damp", 0.86f, Debug::Range(0.0f, 1.0f));

Debug::Float gUiLightPaintRandLow("Ui.LightPaint.RandLow", 0.0f, Debug::Range(0.0f, 50.0f));
Debug::Float gUiLightPaintRandHigh("Ui.LightPaint.RandHigh", 0.0f, Debug::Range(0.0f, 50.0f));

Debug::Bool gUiLightPaintPause("Ui.LightPaint.Pause", false);

Maths::Mat43 MakeMat43_LookAt(Maths::Point3 cameraPosition, Maths::Point3 targetPosition)
{
	Maths::Vector3	vectorToTarget	= targetPosition - cameraPosition;
	Maths::Mat33	lookatMatrix	= MakeMat33_AxisZ(vectorToTarget);

	return Maths::MakeMat43(lookatMatrix, cameraPosition);
}

struct SparkleVertex
{
	float	x, y, z, w;
	XtU32	colour;
	float	u0, v0;
};

struct SparkleTrail
{
	Maths::StorageVector3 left;
	Maths::StorageVector3 right;
};

struct SparkleObject
{
	Maths::Mat43	transform;
	Maths::Vector3	position;
	Maths::Vector3	velocity;
	Maths::Vector3	targetNormal;

	bool			active;
	int				killed;
	int				layer;
	int				panel;

	XtU32						colour;
	UI::C_TextureHandleIndex	texture;

	XtHandle<DynamicEffect::Particle::XtInterface::Event> particleEvent;

	float			rotation;
	float			rotationSpeed;
	float			width;

	int				trailCount;
	int				trailIndex;
	SparkleTrail*	trails;

	int				controlCount;
	int				controlIndex;
	SparkleControl*	controls;

	int				vbIndex;

public:
	SparkleObject();
	~SparkleObject();
};

SparkleObject::SparkleObject() :
	active(false),
	killed(0),
	layer(0),
	panel(0),
	colour(0xFFFFFFFF),
	texture(0),
	rotation(0.0f),
	rotationSpeed(0.0f),
	width(0.0f),
	trailCount(0),
	trailIndex(0),
	trails(0),
	controlCount(0),
	controlIndex(0),
	controls(0),
	vbIndex(0)
{
}

SparkleObject::~SparkleObject()
{
	delete [] controls;
}

void SO_TargetNextControl(SparkleObject* so)
{
	if (!so)
		return;

	if ((so->controlIndex + 1) < so->controlCount)
	{
		so->controlIndex++;

		SparkleControl* control = &so->controls[so->controlIndex];

		so->targetNormal = MakeVector3(Normalise(Load(&control->position) - so->position));
	}
	else
	{
		so->killed = so->trailCount;

		if (DynamicEffect::Particle::XtInterface::Event* event = so->particleEvent)
		{
			if (event->pEvent)
			{
				DynamicEffect::C_Node node;

				node.SetPosition((Maths::HVec3)so->position);

				event->pEvent->Create(node);
				
				//play sound when generating particle effect - used in previously on blur only
				UI::UiInterface()->PlaySound( UI::E_UiSoundEffect_PreviouslySparksActionIn );
			}
		}
	}
}

void SO_Create(SparkleObject* so, int trailCount, SparkleTrail* trails, int vbIndex)
{
	if (!so)
		return;

	so->trailCount	= trailCount;
	so->trails		= trails;
	so->vbIndex		= vbIndex;
}

void SO_Initialise(SparkleObject* so, XtU32 colour, UI::C_TextureHandleIndex texture, const XtHandle<DynamicEffect::Particle::XtInterface::Event>& particleEvent, int layer, int panel, float width, int controlCount, SparkleControl* controls)
{
	if (!so || (controlCount <= 0) || !controls)
		return;

	so->active	= true;
	so->killed	= 0;
	so->layer	= layer;
	so->panel	= panel;

	so->colour			= colour;
	so->texture			= texture;
	so->particleEvent	= particleEvent;

	so->width			= width;
	so->position		= Load(&controls[0].position);
	so->velocity		= Maths::MakeVector3(0.0f, 0.0f, 0.0f);
	so->rotation		= gUiRand.FRand(-3.141f, 3.141f);
	so->rotationSpeed	= gUiRand.FRand(7.5f, 10.0f);

	if (gUiRand.FRand() < 0.5f)
		so->rotationSpeed *= -1.0f;

	if ((so->controls = new SparkleControl[controlCount]) == 0)
		return;

	so->controlCount	= controlCount;
	so->controlIndex	= 0;

	so->trailIndex = 0;

	for(int i = 0; i < so->trailCount; i++)
	{
		SparkleTrail* trail = &so->trails[i];

		Store(so->position, &trail->left);
		Store(so->position, &trail->right);
	}

	for(int i = 0; i < controlCount; i++)
		so->controls[i] = controls[i];

	SO_TargetNextControl(so);
}

void SO_Update(SparkleObject* so, float tick)
{
	if (!so->trails || (so->trailCount <= 0) || !so->controls || (so->controlCount <= 0))
		return;

	// Update position and velocity

	SparkleControl* control = &so->controls[so->controlIndex];

	if (so->killed <= 0)
	{
		Maths::Vector3	delta	= Load(&control->position) - so->position;
		float			length	= GetF32(Length(delta));

		if (length > 1.0f)
			delta = delta * Maths::MakeVF32(1.0f / length);
		else
			delta = so->targetNormal;

		so->velocity = (so->velocity + (delta * Maths::MakeVF32(gUiLightPaintRate))) * Maths::MakeVF32(gUiLightPaintDamp);
		so->velocity = so->velocity + (Maths::Vector3)gUiRand.SphereRand() * Maths::MakeVF32(gUiRand.FRand(gUiLightPaintRandLow, gUiLightPaintRandHigh));

		so->position	= so->position + so->velocity * Maths::MakeVF32(tick);
		so->rotation	= so->rotation + so->rotationSpeed * tick;
		so->transform	= Maths::MakeMat33_RotZ(Maths::MakeVF32(so->rotation)) * Maths::MakeMat43(Maths::MakeMat33_AxisAngle(so->velocity, Maths::MakeVF32(0.0f)), so->position);
	}

	// Create trail

	SparkleTrail* old = &so->trails[so->trailIndex];

	so->trailIndex = (so->trailIndex + 1) % so->trailCount;

	Maths::Vector3 aw = RowW(so->transform);
	Maths::Vector3 ax = RowX(so->transform);

	SparkleTrail* trail = &so->trails[so->trailIndex];

	Store(aw, &trail->left);
	Store(aw + ax * Maths::MakeVF32(so->width), &trail->right);

	// Update control index

	if (so->killed <= 0)
	{
		if (GetF32(Dot(so->targetNormal, Load(&control->position) - so->position)) < 0.0f)
			SO_TargetNextControl(so);
	}
	else
	{
		if (--so->killed <= 0)
		{
			so->active = false;

			delete [] so->controls;

			so->controls		= 0;
			so->controlCount	= 0;
			so->controlIndex	= 0;

			so->trailIndex	= 0;
		}
	}
}

void SO_PreRender(SparkleObject* so, SparkleVertex* buffer, Maths::Mat44 transform)
{
	if (!so->trails || (so->trailCount <= 0))
		return;

	volatile SparkleVertex* vtx = buffer;

	int prevTrailIndex = (so->trailIndex + 1) % so->trailCount;

	XtU32 colour = so->colour;

	for(int i = 1; i < so->trailCount; i++)
	{
		int nextTrailIndex = (prevTrailIndex + 1) % so->trailCount;

		SparkleTrail* st0 = &so->trails[prevTrailIndex];
		SparkleTrail* st1 = &so->trails[nextTrailIndex];

		prevTrailIndex = nextTrailIndex;

		Maths::Vector4 p0 = Maths::MakeVector4(Load(&st0->left), Maths::MakeVF32(1.0f));
		Maths::Vector4 p1 = Maths::MakeVector4(Load(&st0->right), Maths::MakeVF32(1.0f));
		Maths::Vector4 p2 = Maths::MakeVector4(Load(&st1->right), Maths::MakeVF32(1.0f));
		Maths::Vector4 p3 = Maths::MakeVector4(Load(&st1->left), Maths::MakeVF32(1.0f));

		float v0 = (float)i / (float)so->trailCount;
		float v1 = ((float)i + 1.0f) / (float)so->trailCount;

		Maths::Vector4 t0 = p0 * transform;
		Maths::Vector4 t1 = p1 * transform;
		Maths::Vector4 t2 = p2 * transform;
		Maths::Vector4 t3 = p3 * transform;

		p0 = t0;
		p1 = t1;
		p2 = t2;
		p3 = t3;

		t0 = Maths::MakeVector4(X(t0) / W(t0), Y(t0) / W(t0), Z(t0), W(t0));
		t1 = Maths::MakeVector4(X(t1) / W(t1), Y(t1) / W(t1), Z(t1), W(t1));
		t2 = Maths::MakeVector4(X(t2) / W(t2), Y(t2) / W(t2), Z(t2), W(t2));
		t3 = Maths::MakeVector4(X(t3) / W(t3), Y(t3) / W(t3), Z(t3), W(t3));

		// t0 -> t2
		// t1 -> t3

		Maths::Line l0 = MakeLine(Maths::MakePoint3(X(t0), Y(t0), Maths::MakeVF32(0.0f)), Maths::MakePoint3(X(t1), Y(t1), Maths::MakeVF32(0.0f)));
		Maths::Line l1 = MakeLine(Maths::MakePoint3(X(t2), Y(t2), Maths::MakeVF32(0.0f)), Maths::MakePoint3(X(t3), Y(t3), Maths::MakeVF32(0.0f)));

		Maths::VF32 f0, f1;

		if (
				(W(t0) > Maths::MakeVF32(0.1f)) &&
				(W(t1) > Maths::MakeVF32(0.1f)) &&
				(W(t2) > Maths::MakeVF32(0.1f)) &&
				(W(t3) > Maths::MakeVF32(0.1f)) &&
				(bool)Intersect(l0, l1, &f0, &f1, Maths::MakeVF32(1000000.1f))
			)
		{
			Maths::Vector4 c = Lerp(t0, t1, f0); // TODO: needs to happen in screenspace then back project... (or just pass in pretransformed...)

			c = Maths::MakeVector4(X(c) * W(c), Y(c) * W(c), Z(c), W(c));

			float u2 = GetF32(f0); // TODO: This is wrong, need to take intersection point into account
			float v2 = (v0 + v1) * 0.5f;

			// Bowtie - Triangle 0

			vtx->x		= GetF32(X(p0));
			vtx->y		= GetF32(Y(p0));
			vtx->z		= GetF32(Z(p0));
			vtx->w		= GetF32(W(p0));
			vtx->colour	= colour;
			vtx->u0		= v0;
			vtx->v0		= 0.0f;
			vtx++;

			vtx->x		= GetF32(X(c));
			vtx->y		= GetF32(Y(c));
			vtx->z		= GetF32(Z(c));
			vtx->w		= GetF32(W(c));
			vtx->colour	= colour;
			vtx->u0		= v2;
			vtx->v0		= u2;
			vtx++;

			vtx->x		= GetF32(X(p3));
			vtx->y		= GetF32(Y(p3));
			vtx->z		= GetF32(Z(p3));
			vtx->w		= GetF32(W(p3));
			vtx->colour	= colour;
			vtx->u0		= v1;
			vtx->v0		= 0.0f;
			vtx++;

			// Bowtie - Triangle 1

			vtx->x		= GetF32(X(p1));
			vtx->y		= GetF32(Y(p1));
			vtx->z		= GetF32(Z(p1));
			vtx->w		= GetF32(W(p1));
			vtx->colour	= colour;
			vtx->u0		= v0;
			vtx->v0		= 1.0f;
			vtx++;

			vtx->x		= GetF32(X(p2));
			vtx->y		= GetF32(Y(p2));
			vtx->z		= GetF32(Z(p2));
			vtx->w		= GetF32(W(p2));
			vtx->colour	= colour;
			vtx->u0		= v1;
			vtx->v0		= 1.0f;
			vtx++;

			vtx->x		= GetF32(X(c));
			vtx->y		= GetF32(Y(c));
			vtx->z		= GetF32(Z(c));
			vtx->w		= GetF32(W(c));
			vtx->colour	= colour;
			vtx->u0		= v2;
			vtx->v0		= u2;
			vtx++;
		}
		else
		{
			// Quad - Triangle 0

			vtx->x		= GetF32(X(p0));
			vtx->y		= GetF32(Y(p0));
			vtx->z		= GetF32(Z(p0));
			vtx->w		= GetF32(W(p0));
			vtx->colour	= colour;
			vtx->u0		= v0;
			vtx->v0		= 0.0f;
			vtx++;

			vtx->x		= GetF32(X(p1));
			vtx->y		= GetF32(Y(p1));
			vtx->z		= GetF32(Z(p1));
			vtx->w		= GetF32(W(p1));
			vtx->colour	= colour;
			vtx->u0		= v0;
			vtx->v0		= 1.0f;
			vtx++;

			vtx->x		= GetF32(X(p2));
			vtx->y		= GetF32(Y(p2));
			vtx->z		= GetF32(Z(p2));
			vtx->w		= GetF32(W(p2));
			vtx->colour	= colour;
			vtx->u0		= v1;
			vtx->v0		= 1.0f;
			vtx++;

			// Quad - Triangle 1

			vtx->x		= GetF32(X(p0));
			vtx->y		= GetF32(Y(p0));
			vtx->z		= GetF32(Z(p0));
			vtx->w		= GetF32(W(p0));
			vtx->colour	= colour;
			vtx->u0		= v0;
			vtx->v0		= 0.0f;
			vtx++;

			vtx->x		= GetF32(X(p2));
			vtx->y		= GetF32(Y(p2));
			vtx->z		= GetF32(Z(p2));
			vtx->w		= GetF32(W(p2));
			vtx->colour	= colour;
			vtx->u0		= v1;
			vtx->v0		= 1.0f;
			vtx++;

			vtx->x		= GetF32(X(p3));
			vtx->y		= GetF32(Y(p3));
			vtx->z		= GetF32(Z(p3));
			vtx->w		= GetF32(W(p3));
			vtx->colour	= colour;
			vtx->u0		= v1;
			vtx->v0		= 0.0f;
			vtx++;
		}
	}
}

const int kMaxSparkleBuffers = 2;

int						gSparkleCount;
SparkleObject*			gSparkleObjects;
SparkleTrail*			gSparkleTrails;
int						gSparkleBufferIndex;
Display::C_VertexBuffer	gSparkleBuffers[kMaxSparkleBuffers];

void LightPaint_Init(int objectCount, int trailCount)
{
	if (gSparkleCount > 0)
		return;

	int totalTrails = objectCount * trailCount;

	if (
			((gSparkleObjects = new SparkleObject[objectCount]) == 0) ||
			((gSparkleTrails = new SparkleTrail[totalTrails]) == 0)
		)
	{
		LightPaint_Destroy();
		return;
	}

	gSparkleCount = objectCount;

	for(int i = 0; i < objectCount; i++)
		SO_Create(&gSparkleObjects[i], trailCount, gSparkleTrails + (trailCount * i), i * (trailCount - 1) * 6);

	int maxVertices = objectCount * (trailCount - 1) * 6 * sizeof(SparkleVertex);

	for(int i = 0; i < kMaxSparkleBuffers; i++)
	{
		if (!Display::Device()->CreateVertexBuffer(maxVertices, 0, &gSparkleBuffers[i]))
		{
			LightPaint_Destroy();
			return;
		}
	}
}

void LightPaint_Destroy()
{
	for(int i = 0; i < kMaxSparkleBuffers; i++)
	{
		if (gSparkleBuffers[i].IsCreated())
			Display::Device()->DestroyVertexBuffer(&gSparkleBuffers[i]);
	}

	delete [] gSparkleTrails;
	delete [] gSparkleObjects;

	gSparkleCount		= 0;
	gSparkleTrails		= 0;
	gSparkleObjects		= 0;
	gSparkleBufferIndex	= 0;
}

void LightPaint_Emit(XtU32 colour, UI::C_TextureHandleIndex texture, const XtHandle<DynamicEffect::Particle::XtInterface::Event>& particleEffect, int layer, int panel, float width, int controlCount, SparkleControl* controls)
{
	for(int i = 0; i < gSparkleCount; i++)
	{
		SparkleObject* so = &gSparkleObjects[i];

		if (so->active)
			continue;

		SO_Initialise(so, colour, texture, particleEffect, layer, panel, width, controlCount, controls);

		break;
	}
}

void LightPaint_ClearAll()
{
	for(int i = 0; i < gSparkleCount; i++)
	{
		SparkleObject* so = &gSparkleObjects[i];

		if (!so->active)
			continue;

		so->active = false;

		delete [] so->controls;

		so->controls		= 0;
		so->controlCount	= 0;
		so->controlIndex	= 0;

		so->trailIndex	= 0;
	}
}

void LightPaint_Update(float tick)
{
	if (!gUiLightPaintPause)
	{
		for(int i = 0; i < gSparkleCount; i++)
		{
			SparkleObject* so = &gSparkleObjects[i];

			if (!so->active)
				continue;

			for(int i = 0; i < 2; i++)
				SO_Update(so, 1.0f / 60.0f);
		}
	}
}

void LightPaint_RenderElement(const UI::C_Sprite* pSprite)
{
	SparkleObject* so = (SparkleObject*)pSprite->m_UserData;

	if (so->trailCount <= 0)
		return;

	UI::gSimple2D.GetRenderer().SetEffect((UI::E_Effect)pSprite->m_RenderState.m_EffectIndex, 0, (UI::E_TextureAddressMode)pSprite->m_RenderState.m_EffectAddressMode);

	Display::C_Matrix polygonMatrix;
	Store(Maths::HMat44(), &polygonMatrix);

	Display::Device()->SetVertexShaderConstantF(CV_POLYGON_TRANSFORM, (F32*)&polygonMatrix, 4);

	Display::Device()->SetCullMode(Display::E_CullMode_None);
	Display::Device()->SetDepthEnable(Display::E_Bool_False);

	Display::Device()->SetBlendEnable(Display::E_Bool_True);
	Display::Device()->SetBlend(Display::E_Blend_One, Display::E_Blend_One, Display::E_BlendOp_Add, Display::E_Blend_One, Display::E_Blend_One, Display::E_BlendOp_Add);

	Display::Device()->SetVertexDeclaration(UI::gSimple2D.GetRenderer().GetVertexDeclaration());
	Display::Device()->SetVertexBuffer(0, &gSparkleBuffers[gSparkleBufferIndex], 0, sizeof(SparkleVertex));

	Display::Device()->DrawPrimitive(Display::E_PrimitiveType_TriangleList, so->vbIndex, (so->trailCount - 1) * 6);

	Display::Device()->SetCullMode(Display::E_CullMode_Default);
}

void LightPaint_Render()
{
	if (gSparkleCount <= 0)
		return;

	UI::C_RenderState rs;

	rs.m_BlendMode		= UI::E_UiBlendMode_Unknown;
	rs.m_EffectIndex	= UI::E_Effect_PolygonPaint;

	UI::C_UISpriteColours colours;

	gSparkleBufferIndex = (gSparkleBufferIndex + 1) % kMaxSparkleBuffers;

#ifdef PLATFORM_PC
	// [Blur2PC] The sprites added here go into the UI sprite lists, which only the UI update clears
	// (C_Simple2D::StartFrame from C_UiManager::Process). Retail renders once per update, so each trail had
	// one sprite. The decoupled PC loop (ApplicationPC.cpp IdleLoop) renders several frames per update, and
	// each added another sprite for the same trail: the additive trail drew 2 or 3 times on the frames
	// between updates and once on update frames, and flickered. Add the sprites on the first render after
	// the lists were rebuilt; later renders only refill the vertex buffer, which the sprite draws from
	// (LightPaint_RenderElement binds gSparkleBuffers[gSparkleBufferIndex]). Trails are started and
	// ended only in the update, so the set of active trails matches the sprites already added.
	static U32 s_spriteListGeneration = 0xFFFFFFFF;
	const bool addSprites = s_spriteListGeneration != UI::gSimple2D.Blur2PCListGeneration();
	s_spriteListGeneration = UI::gSimple2D.Blur2PCListGeneration();
#else
	const bool addSprites = true;
#endif

	if (SparkleVertex* buffer = (SparkleVertex*)gSparkleBuffers[gSparkleBufferIndex].Lock())
	{
		for(int i = 0; i < gSparkleCount; i++)
		{
			SparkleObject* so = &gSparkleObjects[i];

			if (!so->active)
				continue;

			Maths::Mat44 transform = UI::gSimple2D.GetTransformMatrixByPanel(so->panel);

			SO_PreRender(so, buffer + so->vbIndex, transform);

			if (!addSprites)	// [Blur2PC] see above
				continue;

			UI::gSimple2D.AddSprite(so->layer, Maths::HVec3(), 1.0f, 1.0f, colours, so->panel, (UI::E_SpriteType)UI::E_GameSpriteType_LightPaint, rs, (INTPTR)so);	// [Blur2PC][x64]
			UI::gSimple2D.CurrentSprite()->m_Callback = LightPaint_RenderElement;

			UI::gSimple2D.AddTexUVs(0, so->texture);
		}

		gSparkleBuffers[gSparkleBufferIndex].Unlock();
	}
}