#include <nexus_render/stdafx.h>
#include <nexus_render/D3DWindow.h>
#include <nexus_render/Camera.h>
#include <nexus_render/D3DUtility.h>

using namespace NexusRender;


Camera::Camera()
{
	Reset();
	m_pWindow = 0;
}

Camera::~Camera()
{
    Clear();
}

Camera::Camera(C_D3DWindow* pWindow)
{
	Reset();

	m_pWindow = pWindow;
}

void Camera::SetOrigin(const Maths::HVec3& origin)
{
    m_origin[0] = origin[0];
	m_origin[1] = origin[1];
	m_origin[2] = origin[2];
}

Maths::HVec3 Camera::GetOrigin() const
{
    return Maths::HVec3(m_origin[0], m_origin[1], m_origin[2]);
}

Maths::HVec3 Camera::GetPosition() const
{
    return Maths::HVec3(m_positionWorld[0], m_positionWorld[1], m_positionWorld[2]);
}

void Camera::SetRotation(F32 pitch, F32 yaw, F32 roll)
{
	m_rotAboutFacing = roll;
	m_rotAboutUp = yaw;
	m_rotAboutRight = pitch;

    m_facing = D3DXVECTOR3(0.0f,0.0f,1.0f);    // facing down the positive Z axis
    m_right = D3DXVECTOR3(1.0f,0.0f,0.0f);     // with the positive X axis to the right
    m_up = D3DXVECTOR3(0.0f,1.0f,0.0);         // and the positive Y axis up

    D3DXMATRIX matTotal; // used to speed up the process by performing less matrix
                        // multiplications.
	D3DXMatrixIdentity(&matTotal);

    // the matrices which represent the rotation about each of our 3 axes
    D3DXMATRIX matRotAboutUp; 
	D3DXMATRIX matRotAboutRight;
	D3DXMATRIX matRotAboutFacing;

	D3DXMatrixIdentity(&matRotAboutUp);
	D3DXMatrixIdentity(&matRotAboutRight);
	D3DXMatrixIdentity(&matRotAboutFacing);

	// rotate about right vector (pitch)
	D3DXMatrixRotationAxis(&matRotAboutRight,&m_right,m_rotAboutRight);

	// rotate about up vector (yaw)
    D3DXMatrixRotationAxis(&matRotAboutUp,&D3DXVECTOR3(0,1,0),m_rotAboutUp);
	
	// rotate about direction vector (roll)
	D3DXMatrixRotationAxis(&matRotAboutFacing,&m_facing,m_rotAboutFacing);

	// Multiply matrices to produce a rotation matrix for our local coordinate system
	D3DXMatrixMultiply(&matTotal,&matRotAboutRight,&matRotAboutUp);
	D3DXMatrixMultiply(&matTotal,&matRotAboutFacing,&matTotal);
	
	D3DXVECTOR3 facing;
    D3DXVECTOR3 up;
    D3DXVECTOR3 right;

	// transform 2 of the vectors by this matrix (matTotal) and get the third 
	// using the cross product

    D3DXVec3TransformCoord(&right,&m_right,&matTotal);   
	D3DXVec3TransformCoord(&up,&m_up,&matTotal);   
	D3DXVec3Cross(&facing,&right,&up);

	// Ensure orthogonal axes
    D3DXVec3Cross(&up,&facing,&right);

    // remember to normalise vectors before putting them into the view matrix
    D3DXVec3Normalize(&right,&right);
    D3DXVec3Normalize(&up,&up);
    D3DXVec3Normalize(&facing,&facing);

	m_right= right;
	m_up = up;
	m_facing = facing;
}

void Camera::Frame(const Aftk::C_Bounds& bounds)
{
}

void Camera::OnResize()
{
}

void Camera::Reset()
{
    // initialise members
    m_origin = D3DXVECTOR3(0.0f,0.0f,0.0f);  // start at the origin
    m_facing = D3DXVECTOR3(0.0f,0.0f,1.0f);    // facing down the positive Z axis
    m_right = D3DXVECTOR3(1.0f,0.0f,0.0f);     // with the positive X axis to the right
    m_up = D3DXVECTOR3(0.0f,1.0f,0.0);         // and the positive Y axis up
    m_rotAboutUp = m_rotAboutRight = m_rotAboutFacing = 0.0f;

	//Set the near and far clipping planes
	m_zNear = 0.01f; 
//	m_zNear = 0.5f; 
//	m_zFar = 40000.0f;
	m_zFar = 10000.0f;

//	m_zNear = 1.0f; 
//	m_zFar = 1000000.0f;

	m_centreZ = 150.0f;

	m_fov = (F32)Maths::DegToRad(55.0f);
}

void Camera::Clear()
{
	Reset();
}
