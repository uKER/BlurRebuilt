/////////////////////////////////////////////////////////////
// Collision Spheres
/////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "racingphysics/collisions/collisionspheres.h"

#include "racingphysics/collisions/collisionmesh.h"
#include "racingphysics/collisions/collisioncache.h"
#include "racingphysics/collisions/collisioncache.inl"
#include "racingphysics/collisions/spherecollision.h"
#include "racingphysics/collisions/generatespheres.h"

#include "racingphysics/debug/draw.h"
#include "display/render/basicrender.h"

//#pragma optimize ("", off)

//using namespace Maths;

extern Sys::I_FileSystem& GetPAKAwareFileSystem_Memory();

////////////////////////////////////////////////////////

namespace RacingPhysics
{

void COObject::Initialise(const Axis* a)
{
	m_axis = a;
	m_centre = Point3R(0.f, 0.f, 0.f);
	Clear(&m_size);
	m_radius = 0.f;

	TransformBoundingBox(*m_axis);
}

void COObject::SetAxis(const Axis* a)
{
	m_axis = a;
	ClearTransformState();
}

bool COObject::CollideBoundingBoxes(const COObject& other, float tolerance) const
{
	TransformBoundingBox(GetAxis());
	other.TransformBoundingBox(other.GetAxis());

	return m_boundingBox.TestOverlap(other.m_boundingBox.GetMin(), other.m_boundingBox.GetMax(), tolerance);
}

void COObject::TransformBoundingBox_Sub(const Axis& a) const
{
	TransformCentre(a);

	Vector3R xyz = Maths::Constants<Vector3R>::XYZ();

	m_aabb = Abs(m_size.X() * a.AxisX()) 
			+ Abs(m_size.Y() * a.AxisY()) 
			+ Abs(m_size.Z() * a.AxisZ()) 
			+ xyz * 0.01f;

	Point3R min = m_transformedCentre - m_aabb;
	Point3R max = m_transformedCentre + m_aabb;
	m_boundingBox.SetMinMax(min, max);

	m_transformState = CO_TRANSFORM_STATE_AABB;
}

void COObject::TransformCentre(const Axis& a) const
{
	if (m_transformState >= CO_TRANSFORM_STATE_CENTRE)
		return;

	m_transformedCentre = a.Global(m_centre);
	m_transformState = CO_TRANSFORM_STATE_CENTRE;
}

void COObject::DisplayAABB() const
{
	TransformAll(*m_axis);

	Display::Device()->SetCullMode(Display::E_CullMode_None);
	Display::Device()->SetDepthEnable(Display::E_Bool_False);
	Display::Device()->SetMaxBeginVerticesUsage(100000);

	Display::BasicRender()->BindColoured3D();

	{
		DrawPrimitive::Instance().Begin(Display::E_PrimitiveType_LineStrip);
		DrawPrimitive::Instance().SetVertexDataColor(ColourARGB::Green);
		DrawPrimitive::Instance().SetVertexDataXYZ(m_transformedCentre.X() + m_aabb.X(), m_transformedCentre.Y() + m_aabb.Y(), m_transformedCentre.Z() + m_aabb.Z());
		DrawPrimitive::Instance().SetVertexDataXYZ(m_transformedCentre.X() + m_aabb.X(), m_transformedCentre.Y() + m_aabb.Y(), m_transformedCentre.Z() - m_aabb.Z());
		DrawPrimitive::Instance().SetVertexDataXYZ(m_transformedCentre.X() + m_aabb.X(), m_transformedCentre.Y() - m_aabb.Y(), m_transformedCentre.Z() - m_aabb.Z());
		DrawPrimitive::Instance().SetVertexDataXYZ(m_transformedCentre.X() + m_aabb.X(), m_transformedCentre.Y() - m_aabb.Y(), m_transformedCentre.Z() + m_aabb.Z());
		DrawPrimitive::Instance().SetVertexDataXYZ(m_transformedCentre.X() + m_aabb.X(), m_transformedCentre.Y() + m_aabb.Y(), m_transformedCentre.Z() + m_aabb.Z());
		DrawPrimitive::Instance().End();
		
		DrawPrimitive::Instance().Begin(Display::E_PrimitiveType_LineStrip);
		DrawPrimitive::Instance().SetVertexDataColor(ColourARGB::Green);
		DrawPrimitive::Instance().SetVertexDataXYZ(m_transformedCentre.X() - m_aabb.X(), m_transformedCentre.Y() + m_aabb.Y(), m_transformedCentre.Z() + m_aabb.Z());
		DrawPrimitive::Instance().SetVertexDataXYZ(m_transformedCentre.X() - m_aabb.X(), m_transformedCentre.Y() + m_aabb.Y(), m_transformedCentre.Z() - m_aabb.Z());
		DrawPrimitive::Instance().SetVertexDataXYZ(m_transformedCentre.X() - m_aabb.X(), m_transformedCentre.Y() - m_aabb.Y(), m_transformedCentre.Z() - m_aabb.Z());
		DrawPrimitive::Instance().SetVertexDataXYZ(m_transformedCentre.X() - m_aabb.X(), m_transformedCentre.Y() - m_aabb.Y(), m_transformedCentre.Z() + m_aabb.Z());
		DrawPrimitive::Instance().SetVertexDataXYZ(m_transformedCentre.X() - m_aabb.X(), m_transformedCentre.Y() + m_aabb.Y(), m_transformedCentre.Z() + m_aabb.Z());
		DrawPrimitive::Instance().End();
	}
}

////////////////////////////////////////////////////////

void COObjectSpheres::Shutdown()
{
	m_originalData.Destroy();
	m_transformedData.Destroy();
	m_sphereContactIndices.Destroy();
	m_unmodifiedData.Destroy();
}

void COObjectSpheres::InitialiseBike(const Axis* a, const Point3R& c, const Vector3R& size,
									 const Point3R& p1, const Point3R& p2, float wheelRadius)
{
	RP_ASSERT(m_originalData.Empty());

	m_axis = a;
	m_centre = c;

	float sphereRadius = size.X();

	{
		m_originalData.PushBack();

		// body
		for (int i = 0; i < 5; i++)
		{
			m_originalData.Back().subNodes.PushBack();
			m_originalData.Back().subNodes.Back().SetRadius(sphereRadius);
		}
		
		m_originalData.Back().subNodes[0].SetCentre(c + Vector3R(0.f, 0.5f, -size.Z() + sphereRadius));
		m_originalData.Back().subNodes[1].SetCentre(c + Vector3R(0.f, 0.4f, (-size.Z() + sphereRadius) * 0.5f));
		m_originalData.Back().subNodes[2].SetCentre(c + Vector3R(0.f, 0.3f, 0.f));
		m_originalData.Back().subNodes[3].SetCentre(c + Vector3R(0.f, 0.4f, (size.Z() - sphereRadius) * 0.5f));
		m_originalData.Back().subNodes[4].SetCentre(c + Vector3R(0.f, 0.5f, size.Z() - sphereRadius));

		m_notGroundStartIndex = m_originalData.Size();
		m_originalData.PushBack();
		
		// rider
		m_originalData.Back().subNodes.PushBack();
		m_originalData.Back().subNodes.PushBack();
		m_originalData.Back().subNodes[0].SetCentre(c + Vector3R(0.f, 0.5, 0.f)); 
		m_originalData.Back().subNodes[0].SetRadius(sphereRadius * 1.5f);
		m_originalData.Back().subNodes[1].SetCentre(c + Vector3R(0.f, size.Y() - sphereRadius, -0.2f));
		m_originalData.Back().subNodes[1].SetRadius(sphereRadius);

		// wheels
		m_originalData.Back().subNodes.PushBack();
		m_originalData.Back().subNodes.PushBack();
		m_originalData.Back().subNodes[2].SetCentre(p1); 
		m_originalData.Back().subNodes[2].SetRadius(wheelRadius);
		m_originalData.Back().subNodes[3].SetCentre(p2); 
		m_originalData.Back().subNodes[3].SetRadius(wheelRadius);
	}

	Finish();

	m_originalData[0].sphere.SetCentre(m_centre);
	m_originalData[0].sphere.SetRadius(m_radius);
	m_originalData[1].sphere.SetCentre(m_centre);
	m_originalData[1].sphere.SetRadius(m_radius);

	m_transformedData = m_originalData;
	m_unmodifiedData = m_originalData;
}

Debug::Bool g_debugMakeCarMoreCarShaped("RacingPhysics.Collision.MakeCollisionObjectMoreCarShaped", true, Debug::Persist());

void COObjectSpheres::InitialiseCar(const Axis* a, const Point3R& c, const Vector3R& size, const Point3R* wheelPositions, float wheelRadius)
{
	m_axis = a;
	m_centre = c;

	float r = size.Z() / 8.f;

	for (int node = 0; node < 8; node++)
	{
		m_originalData.PushBack();

		Point3R min, max;
		max = Maths::Constants<Point3R>::NegativeMax();
		min = Maths::Constants<Point3R>::Max();

		for (int i = -1; i <= 1; i += 2)
		for (int j = -1; j <= 1; j += 2)
		for (int k = -1; k <= 1; k += 2)
		{
			if ( g_debugMakeCarMoreCarShaped && ( (node >= 0 && node <= 3) || (node >= 6) ) )
				if (j == 1)
					continue;

			Vector3R p(	i * r, 
						j * r, 
						k * r);

			p.X(p.X() + -size.X() * 0.5f + float(node % 2) * size.X());
			p.Z(p.Z() + -size.Z() * 0.75f + float(node >> 1) * size.Z() * 0.5f);

			m_originalData.Back().subNodes.PushBack();
			m_originalData.Back().subNodes.Back().SetRadius(r * 1.25f);
			m_originalData.Back().subNodes.Back().SetCentre(c + p);

			max = Max(max, c + p + Vector3R(r, r, r));
			min = Min(min, c + p - Vector3R(r, r, r));
		}

		FinishNode(node);
	}

	{
		//m_notGroundStartIndex = m_originalData.Size();
		m_originalData.PushBack();
		
		// wheels
		m_originalData.Back().subNodes.Resize(4);

		for (int i = 0; i < 4; i++)
		{
			m_originalData.Back().subNodes[i].SetCentre(wheelPositions[i]); 
			m_originalData.Back().subNodes[i].SetRadius(wheelRadius);
		}

		FinishNode(m_originalData.Size() - 1);
	}
	
	Finish();

	m_notGroundStartIndex = m_originalData.Size();
	m_transformedData = m_originalData;
}

void COObjectSpheres::FinishNode(int node)
{
	Point3R min, max;
	max = Maths::Constants<Point3R>::NegativeMax();
	min = Maths::Constants<Point3R>::Max();

	for (int j = 0; j < m_originalData[node].subNodes.Size(); j++)
	{
		const COSphere& sphere = m_originalData[node].subNodes[j];
		float r = sphere.Radius();
		max = Max(max, sphere.Centre() + Vector3R(r, r, r));
		min = Min(min, sphere.Centre() - Vector3R(r, r, r));
	}

	Point3R centre = Lerp(min, max, 0.5f);
	FloatVR radius = 0.f;

	for (int j = 0; j < m_originalData[node].subNodes.Size(); j++)
	{
		const COSphere& sphere = m_originalData[node].subNodes[j];
		radius = Max(radius, Magnitude(MakeVector(sphere.Centre() - centre)) + sphere.Radius());
	}

	m_originalData[node].sphere.SetCentre(centre);
	m_originalData[node].sphere.SetRadius(radius);
}

bool COObjectSpheres::InitialiseFromFile(const Axis* a, const Point3R& c, const char* filename)
{
	/*m_axis = a;
	m_centre = c;
	m_notGroundStartIndex = 0;

	ExportCollisionData carCollisionData;
	Sys::I_File* f = 0;

	if (Sys::GetDefaultFileSystem()->FileExists(filename))
	{
		f = Sys::GetDefaultFileSystem()->Open(filename, Sys::E_FileAccess_Read, true);
	}

	if (!f)
	{
		f = Sys::GetDefaultFileSystem()->Open("DontShip\\carcollision.dat", Sys::E_FileAccess_Read);
		LOGERROR(g_RacingPhysicsLog, "Could not open collision data file %s", filename);
		return false;
	}

	if (f)
	{
		Sys::BinaryDeserialiser<Ut::BigEndianToNative> archive(*f);
		archive >> carCollisionData;
		delete f;
	}

	const GenerateSpheres::Node* pSphereData = Pointer(carCollisionData.m_pSphereData);
	
	if (pSphereData != 0)
	{
		for (int i = 0; i < pSphereData->subNodes.Size(); i++)
		{
			const GenerateSpheres::Sphere& sphere = pSphereData->subNodes[i].sphere;
			m_originalData.PushBack();
			m_originalData.Back().sphere = COSphere(sphere.position, sphere.radius);

			for (int j = 0; j < pSphereData->subNodes[i].subNodes.Size(); j++)
			{
				const GenerateSpheres::Sphere& sphere = pSphereData->subNodes[i].subNodes[j].sphere;
				m_originalData.Back().subNodes.PushBack(COSphere(sphere.position, sphere.radius));
			}
		}

		m_unmodifiedData = m_originalData;
	}
	else
		RP_ASSERT(!"Could not create car collision data");

	Finish();

	return true;*/

	return false;
}

void COObjectSpheres::Finish()
{
	Point3R min, max;
	Clear(&max);
	Clear(&min);
	m_radius = 0.f;

	m_transformedData = m_originalData;
	m_sphereContactIndices.Resize(m_originalData.Size());

	for (int i = 0; i < m_originalData.Size(); i++)
	{
		m_sphereContactIndices[i].Resize(m_originalData[i].subNodes.Size());

		for (int j = 0; j < m_originalData[i].subNodes.Size(); j++)
		{
			const COSphere& sphere = m_originalData[i].subNodes[j];
			float r = sphere.Radius();
			max = Max(max, sphere.Centre() + Vector3R(r, r, r));
			min = Min(min, sphere.Centre() - Vector3R(r, r, r));
		}
	}

	m_centre = MakePoint((MakeVector(max) + MakeVector(min)) * 0.5f);
	m_size = (max - min) * 0.5f;

	for (int i = 0; i < m_originalData.Size(); i++)
	{
		m_sphereContactIndices[i].Resize(m_originalData[i].subNodes.Size());

		for (int j = 0; j < m_originalData[i].subNodes.Size(); j++)
		{
			const COSphere& sphere = m_originalData[i].subNodes[j];
			FloatVR r = sphere.Radius();
			m_radius = Max(FloatVR(m_radius), Magnitude(MakeVector(sphere.Centre() - m_centre)) + r);
		}
	}

	TransformAll(*m_axis);
}

void DisplaySphere(const COSphere& sphere, ColourARGB colour)
{
	Axis axis;
	axis.SetIdentityMatrix();
	axis.SetPosition(sphere.Centre());
	
	const MathsStorage::Matrix44& tempMatrix = axis.GetMatrix();
	Display::BasicRender()->BindColoured3D(&tempMatrix);

	DrawPrimitive::Instance().Begin(Display::E_PrimitiveType_LineStrip);
	DrawPrimitive::Instance().SetVertexDataColor(colour);

	float rs = sphere.Radius();

	for (int i = -3; i <= 3; i++)
	{
		float y = i * rs / 3;
		float rc = sqrt(rs * rs - y * y);

		for (int j = 0; j <= 10; j++)
		{
			float b = j * 2 * Maths::Pi / 10;
			DrawPrimitive::Instance().SetVertexDataXYZ(rc * sin(b), y, rc * cos(b));
		}
	}

	DrawPrimitive::Instance().End();
}

// display (for debug)
void COObjectSpheres::Display(const Axis& a, ColourARGB c) const
{
	TransformAll(*m_axis);

	Display::Device()->SetCullMode(Display::E_CullMode_None);
	Display::Device()->SetDepthEnable(Display::E_Bool_False);
	Display::Device()->SetMaxBeginVerticesUsage(100000);

	for (int j = 0; j < m_transformedData.Size(); j++)
	{
		DisplaySphere(m_transformedData[j].sphere, ColourARGB::Blue);

		for (int k = 0; k < m_transformedData[j].subNodes.Size(); k++)
		{
			ColourARGB colour;

			if (j >= m_notGroundStartIndex)
				colour = ColourARGB::Red;
			else
				colour = ColourARGB::Yellow;

			DisplaySphere(m_transformedData[j].subNodes[k], colour);
		}
	}
}

//////////////////////////////////////////////////////////
// transform

void COObjectSpheres::TransformStage1(const Axis& a) const
{
}

void COObjectSpheres::TransformStage2(const Axis& a) const
{
	TransformAll(a);
}

void COObjectSpheres::TransformAll(const Axis& a) const
{
	if (m_transformState == CO_TRANSFORM_STATE_FULL)
		return;

	TransformBoundingBox(a);

	m_transformState = CO_TRANSFORM_STATE_FULL;

	for (int i = 0; i < m_originalData.Size(); i++)
	{
		Point3R p = a.Global(m_originalData[i].sphere.Centre());
		m_transformedData[i].sphere.SetCentre(p);

		for (int j = 0; j < m_originalData[i].subNodes.Size(); j++)
			m_transformedData[i].subNodes[j].SetCentre(a.Global(m_originalData[i].subNodes[j].Centre()));
	}
}

////////////////////////////////////////////////////////////////
// COObject

CollisionCache::Contact COObjectSpheres::GetIntersection(const COObject& other, bool transform) const
{
	if (transform)
	{
		TransformAll(GetAxis());
		other.TransformAll(other.GetAxis());
	}

	CollisionCache::Contact contact;
	contact.collisionType = CollisionCache::COLLISION_TYPE_NONE;
	contact.intersectionDistance = 0.f;
	contact.surfaceFlags = 0;
	contact.boundingBoxesHit = 0;

	const COObjectSpheres& object = static_cast<const COObjectSpheres&>(other);

	for (int i = 0; i < m_transformedData.Size(); i++)
	{
		for (int j = 0; j < object.m_transformedData.Size(); j++)
		{
			const COSphere& sphere1 = m_transformedData[i].sphere;
			const COSphere& sphere2 = object.m_transformedData[j].sphere;

			Vector3R v = sphere2.Centre() - sphere1.Centre();
			FloatVR d2 = Magnitude2(v);
			FloatVR r = sphere1.Radius() + sphere2.Radius() - contact.intersectionDistance;
			
			if (d2 < r * r)
			{
				for (int k = 0; k < m_transformedData[i].subNodes.Size(); k++)
				{
					for (int l = 0; l < object.m_transformedData[j].subNodes.Size(); l++)
					{
						const COSphere& sphere1 = m_transformedData[i].subNodes[k];
						const COSphere& sphere2 = object.m_transformedData[j].subNodes[l];

						Vector3R v = sphere2.Centre() - sphere1.Centre();
						FloatVR v2 = v | v;
						FloatVR rSum = sphere1.Radius() + sphere2.Radius();
						FloatVR r = rSum - contact.intersectionDistance;

						if (v2 < r * r)
						{
							FloatVR s = Sqrt(v2);
							Normal3R n = MakeNormal(v / s);
					
							contact.intersectionDistance = rSum - s;
							contact.intersectionPoint = sphere1.Centre() + n * sphere1.Radius();
							contact.intersectionNormal = n;
							contact.collisionType = CollisionCache::COLLISION_TYPE_OTHER;
						}
					}
				}
			}
		}
	}

	return contact;
}

CollisionCache::Contact COObjectSpheres::GetSeparation(const COObject& other, float maxDistance, bool transform) const
{
	if (transform)
	{
		TransformAll(GetAxis());
		other.TransformAll(other.GetAxis());
	}

	const COObjectSpheres& object = static_cast<const COObjectSpheres&>(other);

	CollisionCache::Contact contact;
	contact.collisionType = CollisionCache::COLLISION_TYPE_NONE;
	contact.intersectionDistance = FloatVR(maxDistance);
	contact.surfaceFlags = 0;
	contact.boundingBoxesHit = 0;

	for (int i = 0; i < m_transformedData.Size(); i++)
	{
		for (int j = 0; j < object.m_transformedData.Size(); j++)
		{
			const COSphere& sphere1 = m_transformedData[i].sphere;
			const COSphere& sphere2 = object.m_transformedData[j].sphere;

			Vector3R v = sphere2.Centre() - sphere1.Centre();
			FloatVR d2 = Magnitude2(v);
			FloatVR r = contact.intersectionDistance + sphere1.Radius() + sphere2.Radius();
			
			if (d2 < r * r)
			{
				for (int k = 0; k < m_transformedData[i].subNodes.Size(); k++)
				{
					for (int l = 0; l < object.m_transformedData[j].subNodes.Size(); l++)
					{
						const COSphere& sphere1 = m_transformedData[i].subNodes[k];
						const COSphere& sphere2 = object.m_transformedData[j].subNodes[l];

						Vector3R v = sphere2.Centre() - sphere1.Centre();
						FloatVR d2 = Magnitude2(v);
						FloatVR r = contact.intersectionDistance + sphere1.Radius() + sphere2.Radius();

						if (d2 < r * r)
						{
							contact.intersectionDistance = Sqrt(d2) - sphere1.Radius() - sphere2.Radius();
							contact.intersectionPoint = sphere1.Centre();
							contact.intersectionNormal = Normalised(v);
							contact.collisionType = CollisionCache::COLLISION_TYPE_OTHER;
						}
					}
				}
			}
		}
	}

	return contact;
}

bool COObjectSpheres::CollideWithObject(const COObject& other, bool transform) const
{
	if (transform)
	{
		if (!CollideBoundingBoxes(other)) return false;

		TransformAll(GetAxis());
		other.TransformAll(other.GetAxis());
	}

	const COObjectSpheres& object = static_cast<const COObjectSpheres&>(other);

	for (int i = 0; i < m_transformedData.Size(); i++)
	{
		for (int j = 0; j < object.m_transformedData.Size(); j++)
		{
			const COSphere& sphere1 = m_transformedData[i].sphere;
			const COSphere& sphere2 = object.m_transformedData[j].sphere;

			FloatVR d2 = Magnitude2(sphere1.Centre() - sphere2.Centre());
			FloatVR r = sphere1.Radius() + sphere2.Radius();

			if (d2 < r * r)
			{
				for (int k = 0; k < m_transformedData[i].subNodes.Size(); k++)
				{
					for (int l = 0; l < object.m_transformedData[j].subNodes.Size(); l++)
					{
						const COSphere& sphere1 = m_transformedData[i].subNodes[k];
						const COSphere& sphere2 = object.m_transformedData[j].subNodes[l];

						FloatVR d2 = Magnitude2(sphere1.Centre() - sphere2.Centre());
						FloatVR r = sphere1.Radius() + sphere2.Radius();

						if (d2 < r * r)
							return true;
					}
				}
			}
		}
	}

	return false;
}

void COObjectSpheres::GetContacts(const COObject& other, ContactList& contacts, bool transform) const
{
	contacts.Clear();

	if (transform)
	{
		if (!CollideBoundingBoxes(other)) return;

		TransformAll(GetAxis());
		other.TransformAll(other.GetAxis());
	}

	const COObjectSpheres& object = static_cast<const COObjectSpheres&>(other);

	for (int i = 0; i < m_transformedData.Size(); i++)
	{
		for (int j = 0; j < object.m_transformedData.Size(); j++)
		{
			const COSphere& sphere1 = m_transformedData[i].sphere;
			const COSphere& sphere2 = object.m_transformedData[j].sphere;

			Vector3R v = sphere2.Centre() - sphere1.Centre();
			FloatVR d2 = Magnitude2(v);
			FloatVR r = sphere1.Radius() + sphere2.Radius();
			
			if (d2 < r * r)
			{
				for (int k = 0; k < m_transformedData[i].subNodes.Size(); k++)
				{
					for (int l = 0; l < object.m_transformedData[j].subNodes.Size(); l++)
					{
						const COSphere& sphere1 = m_transformedData[i].subNodes[k];
						const COSphere& sphere2 = object.m_transformedData[j].subNodes[l];

						Vector3R v = sphere2.Centre() - sphere1.Centre();
						FloatVR v2 = v | v;
						FloatVR rSum = sphere1.Radius() + sphere2.Radius();
						
						if (v2 < rSum * rSum)
						{
							FloatVR s = Sqrt(v2);
							Normal3R n = MakeNormal(v / s);
	
							CollisionCache::Contact newContact;
							newContact.intersectionDistance = rSum - s;
							newContact.collisionType = CollisionCache::COLLISION_TYPE_NONE;
							newContact.intersectionPoint = sphere1.Centre() + n * sphere1.Radius();
							newContact.intersectionNormal = n;
							newContact.surfaceFlags = 0;

							contacts.PushBack(newContact);
						}
					}
				}
			}
		}
	}
}

Btl::Vector<COObjectSpheres::Node> &COObjectSpheres::GetOriginalData()
{
	return m_originalData;
}

Btl::Vector<COObjectSpheres::Node> &COObjectSpheres::GetUnmodifiedData()
{
	return m_unmodifiedData;
}

void COObjectSpheres::ResetSpheres()
{
	m_originalData = m_unmodifiedData;
}

////////////////////////////////////////////////////////////////////
// cache

template <typename PrimitiveType>
CollisionCache::BooleanType COObjectSpheres::GetSeparation(const PrimitiveType& p, CollisionCache::Contact& contact) const
{
	CollisionCache::BooleanType hit = Maths::Constants<CollisionCache::BooleanType>::False();

	for (int i = 0; i < m_transformedData.Size(); i++)
	{
		if (Btl::SameType<PrimitiveType, CollisionCache::VerticalSurface>::result 
			|| i < m_notGroundStartIndex)
		{
			SphereCollision collision(m_transformedData[i].sphere.Centre(), m_transformedData[i].sphere.Radius() + m_maxDistance, CollisionCache::NullBoundingBox());

			CollisionCache::Contact newContact;
			newContact.intersectionDistance = Maths::Constants<CollisionCache::ScalarType>::Zero();
			newContact.collisionType = CollisionCache::COLLISION_TYPE_NONE;

			collision.CollideFull(p, newContact);

			if (newContact.collisionType != CollisionCache::COLLISION_TYPE_NONE)
			{
				for (int j = 0; j < m_originalData[i].subNodes.Size(); j++)
				{
					const COSphere& sphere = m_transformedData[i].subNodes[j];

					SphereCollision collision(sphere.Centre(), sphere.Radius() + m_maxDistance, CollisionCache::NullBoundingBox());

					CollisionCache::Contact newContact;
					newContact.intersectionDistance = Maths::Constants<CollisionCache::ScalarType>::Zero();
					newContact.collisionType = CollisionCache::COLLISION_TYPE_NONE;

					hit |= collision.CollideFeature(p, newContact);

					if (newContact.collisionType != CollisionCache::COLLISION_TYPE_NONE)
					{
						// ignore collisions with the back of surfaces
						if (newContact.intersectionDistance < collision.GetRadius())
						{
							//RP_ASSERT(FloatVR(m_maxDistance + 0.001f) > newContact.intersectionDistance);
							m_maxDistance = m_maxDistance - newContact.intersectionDistance;
							contact = newContact;
							contact.intersectionDistance = CollisionCache::ScalarType(m_maxDistance);
						}
					}
				}
			}
		}
	}

	return hit;
}

template <typename PrimitiveType>
CollisionCache::BooleanType COObjectSpheres::GetCollision(const PrimitiveType& p, CollisionCache::Contact& contact) const
{
	CollisionCache::BooleanType hit = Maths::Constants<CollisionCache::BooleanType>::False();

	for (int i = 0; i < m_transformedData.Size(); i++)
	{
		if (Btl::SameType<PrimitiveType, CollisionCache::VerticalSurface>::result 
			|| i < m_notGroundStartIndex)
		{
			SphereCollision collision(m_transformedData[i].sphere.Centre(), m_transformedData[i].sphere.Radius(), CollisionCache::NullBoundingBox());

			CollisionCache::Contact newContact;
			newContact.intersectionDistance = Maths::Constants<CollisionCache::ScalarType>::Zero();
			newContact.collisionType = CollisionCache::COLLISION_TYPE_NONE;

			collision.CollideFull(p, newContact);

			if (newContact.collisionType != CollisionCache::COLLISION_TYPE_NONE)
			{
				for (int j = 0; j < m_originalData[i].subNodes.Size(); j++)
				{
					const COSphere& sphere = m_transformedData[i].subNodes[j];
					SphereCollision collision(sphere.Centre(), sphere.Radius(), CollisionCache::NullBoundingBox());

					hit |= collision.CollideFeature(p, contact);
				}
			}
		}
	}

	return hit;
}

template <typename PrimitiveType>
CollisionCache::BooleanType COObjectSpheres::GetContacts(const PrimitiveType& p, CollisionCache::Contact& contact) const
{
	CollisionCache::BooleanType hit = Maths::Constants<CollisionCache::BooleanType>::False();

	for (int i = 0; i < m_transformedData.Size(); i++)
	{
		if (Btl::SameType<PrimitiveType, CollisionCache::VerticalSurface>::result 
			|| i < m_notGroundStartIndex)
		{
			SphereCollision collision(m_transformedData[i].sphere.Centre(), m_transformedData[i].sphere.Radius(), CollisionCache::NullBoundingBox());

			CollisionCache::Contact newContact;
			newContact.intersectionDistance = Maths::Constants<CollisionCache::ScalarType>::Zero();
			newContact.collisionType = CollisionCache::COLLISION_TYPE_NONE;

			collision.CollideFull(p, newContact);

			if (newContact.collisionType != CollisionCache::COLLISION_TYPE_NONE)
			{
				for (int j = 0; j < m_originalData[i].subNodes.Size(); j++)
				{
					const COSphere& sphere = m_transformedData[i].subNodes[j];
					SphereCollision collision(sphere.Centre(), sphere.Radius(), CollisionCache::NullBoundingBox());

					if (m_sphereContactIndices[i][j] > -1)
					{
						hit |= collision.CollideFeature(p, (*m_pContactList)[m_sphereContactIndices[i][j]]);
					}
					else
					{
						CollisionCache::Contact newContact;
						newContact.intersectionDistance = Maths::Constants<CollisionCache::ScalarType>::Zero();
						newContact.collisionType = CollisionCache::COLLISION_TYPE_NONE;

						hit |= collision.CollideFeature(p, newContact);

						if (newContact.collisionType != CollisionCache::COLLISION_TYPE_NONE)
						{
							m_sphereContactIndices[i][j] = m_pContactList->Size();
							m_pContactList->PushBack(newContact);
						}
					}
				}
			}
		}
	}

	return hit;
}

template <typename PrimitiveType>
CollisionCache::BooleanType COObjectSpheres::CollideFeature(const PrimitiveType& p, CollisionCache::Contact& contact) const
{
	if (m_separation)
		return GetSeparation(p, contact);
	else if (m_pContactList)
		return GetContacts(p, contact);
	else
		return GetCollision(p, contact);
}

CollisionCache::Contact COObjectSpheres::GetCollision(const CollisionCache& cache, U32 flags) const
{
	m_pContactList = 0;
	m_separation = false;

	TransformAll(*m_axis);

	return cache.CollideVolume<COObjectSpheres, BoundingBoxCollision, false>(*this, m_boundingBox, flags);
}

void COObjectSpheres::GetContacts(const CollisionCache& cache, U32 flags, ContactList& contacts) const
{
	contacts.Clear();

	for (int i = 0; i < m_sphereContactIndices.Size(); i++)
		for (int j = 0; j < m_sphereContactIndices[i].Size(); j++)
			m_sphereContactIndices[i][j] = -1;

	m_pContactList = &contacts;
	m_separation = false;

	TransformAll(*m_axis);

	cache.CollideVolume<COObjectSpheres, BoundingBoxCollision, false>(*this, m_boundingBox, flags);
}

CollisionCache::Contact COObjectSpheres::GetSeparation(const CollisionCache& cache, U32 flags, float maxDistance) const
{
	m_pContactList = 0;
	m_maxDistance = maxDistance;
	m_separation = true;

	TransformAll(*m_axis);

	Vector3R size(maxDistance, maxDistance, maxDistance);

	Point3R min = m_transformedCentre - m_aabb - size;
	Point3R max = m_transformedCentre + m_aabb + size;

	// clip to the size of the cache to avoid assert
	min = Max(min, cache.GetMin());
	max = Min(max, cache.GetMax());

	BoundingBoxCollision box(min, max);

	return cache.CollideVolume<COObjectSpheres, BoundingBoxCollision, false>(*this, box, flags);
}

CollisionCache::Contact COObjectSpheres::GetIntersection(const CollisionCache& cache, U32 flags) const
{
	m_pContactList = 0;
	m_separation = false;

	TransformAll(*m_axis);

	Point3R min = m_transformedCentre - m_aabb;
	Point3R max = m_transformedCentre + m_aabb;

	// clip to the size of the cache to avoid assert
	min = Max(min, cache.GetMin());
	max = Min(max, cache.GetMax());

	BoundingBoxCollision box(min, max);

	return cache.CollideVolume<COObjectSpheres, BoundingBoxCollision, false>(*this, box, flags);
}

////////////////////////////////////////////////////////////////
// Point

bool COObjectSpheres::CollideWithPoint(const Point3R& position, float radius, bool verticalOnly) const
{
	BIZ_ASSERT_FAIL("CollideWithPoint not implemented");
	return false;
}

float COObjectSpheres::GetPointIntersection(const Point3R& position, float radius, Point3R* intersectionPoint, Normal3R* normal, bool verticalOnly, const Axis *pCustomAxis) const
{
	// This is used for Photo Mode to make sure collisions happen with the VehicleModel axis and not the Vehicle axis:
	if (pCustomAxis)
	{
		ClearTransformState(); // To make sure it applies the transform
		TransformAll(*pCustomAxis);
		ClearTransformState(); // To make sure it applies the transform next time
	}
	else
	{
		TransformAll(*m_axis);
	}

	FloatVR intersectionDistance = Maths::Constants<FloatVR>::Zero();
	Vector3R size(radius, radius, radius); 

	Point3R bmin = position - size;
	Point3R bmax = position + size;
	
	if (verticalOnly) bmin.Y(bmin.Y() - 1.f); // allow 1m extra in case of 2d collision

	if (m_boundingBox.TestOverlap(bmin, bmax))
	{
		FloatVR r = FloatVR(radius);

		for (int i = 0; i < m_transformedData.Size(); i++)
		{
			const COSphere& sphere = m_transformedData[i].sphere;
			
			Point3R p = position;
			if (verticalOnly) p.Y(sphere.Centre().Y());
			Vector3R v = p - sphere.Centre();
			FloatVR d2 = Magnitude2(v);
			FloatVR r = sphere.Radius() + radius - intersectionDistance;
			
			if (d2 < r * r)
			{
				for (int j = 0; j < m_transformedData[i].subNodes.Size(); j++)
				{
					const COSphere& sphere = m_transformedData[i].subNodes[j];
					
					Point3R p = position;
					if (verticalOnly) p.Y(sphere.Centre().Y());
					Vector3R v = p - sphere.Centre();
					FloatVR v2 = v | v;
					FloatVR rSum = sphere.Radius() + radius;
					FloatVR r = rSum - intersectionDistance;

					if (v2 < r * r)
					{
						FloatVR s = Sqrt(v2);
						intersectionDistance = rSum - s;
						Normal3R n = MakeNormal(v / s);
						*intersectionPoint = sphere.Centre() + n * sphere.Radius();
						*normal = n;
					}
				}
			}
		}
	}

	return intersectionDistance;
}

}


