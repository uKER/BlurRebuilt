#include "stdafx.h"

#include <scene_renderer/Common.h>
#include <scene_renderer/SinCos.h>
#include <scene_renderer/D3dWindow.h>

#include <scene_renderer/Camera.h>

using namespace SRD;

Camera::Camera()
{
	Reset();
	m_pD3dWindow = 0;
}

Camera::~Camera()
{
    Clear();
}

Camera::Camera(D3dWindow* pD3dWindow)
{
	Reset();

	m_pD3dWindow = pD3dWindow;
}

void Camera::SetPosition(const Position3& position)
{
    m_origin[0] = position[0];
	m_origin[1] = position[1];
	m_origin[2] = position[2];
}

Position3 Camera::GetPosition() const
{
    return Position3(m_origin[0], m_origin[1], m_origin[2]);
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

void Camera::GetViewMatrix(D3DXMATRIX& matrix) const
{
	matrix = m_viewMatrix;
}

void Camera::GetProjectionMatrix(D3DXMATRIX& matrix) const
{
	matrix =  m_projectionMatrix;
}

void Camera::GetWorldMatrix(D3DXMATRIX& matrix) const
{
	matrix = m_worldMatrix;
}

void Camera::MoveForward(F32 dist)
{
    m_origin += dist*m_facing;
}   

void Camera::MoveRight(F32 dist)
{
    m_origin += dist*m_right;
}

void Camera::MoveUp(F32 dist)
{
    m_origin += dist*m_up;
}

void Camera::MoveInDirection(F32 dist, const Vector3F32& direction)
{
	Vector3F32 translation = dist * direction;
	m_origin[0] = translation[0];
	m_origin[0] = translation[1];
	m_origin[0] = translation[2];
}

void Camera::RotatePitch(F32 angle)
{
    m_rotAboutRight += angle;
}

void Camera::RotateYaw(F32 angle)
{
    m_rotAboutUp += angle;
}

void Camera::RotateRoll(F32 angle)
{
    m_rotAboutFacing += angle;
}

void Camera::Frame(const LPVS::Bounds& bounds)
{
}

void Camera::OnResize()
{
}

F32 Camera::GetFov() const
{
	return m_fov;
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
	m_zNear = 0.5f; 
	m_zFar = 41000.0f;
	m_centreZ = 150.0f;

	m_fov = (F32)LPVS::DegToRad(55.0f);
}

void Camera::Clear()
{
	Reset();
}
