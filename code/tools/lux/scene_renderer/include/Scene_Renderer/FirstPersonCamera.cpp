#include "stdafx.h"

#include <scene_renderer/Common.h>
#include <scene_renderer/SinCos.h>
#include <scene_renderer/D3dWindow.h>

#include <scene_renderer/Camera.h>
#include <scene_renderer/FirstPersonCamera.h>

using namespace SR;

FirstPersonCamera::FirstPersonCamera() : Camera()
{
	Reset();
}

FirstPersonCamera::~FirstPersonCamera()
{
    Clear();
}

FirstPersonCamera::FirstPersonCamera(D3dWindow* pD3dWindow) : Camera(pD3dWindow)
{
	Reset();

	m_pD3dWindow = pD3dWindow;
}

SrCameraTypeEnum FirstPersonCamera::GetType() const
{
	return SR_CAMERA_TYPE_FIRST_PERSON;
}

void FirstPersonCamera::Frame(const LPVS::Bounds& bounds)
{
	SetRotation(0,0,0);

	F32 dist = sqrtf(bounds.GetBoundingBoxRadiusSquared()) + 0.1f;

	U32 viewportDX = m_pD3dWindow->GetWidth();
	U32 viewportDY = m_pD3dWindow->GetHeight();

	F32	yFov;
	F32 xFov;
	F32 fov = (F32)LPVS::DegToRad(56.303f);

	m_centreZ = tan(fov) * ((F32)(viewportDX)/2.0f);
	yFov = (F32)atan( (viewportDY / m_centreZ)*0.5f);
	xFov = (F32)atan( (viewportDX / m_centreZ)*0.5f);
	fov = min(yFov,xFov);
	dist /= (F32)sin(fov);

	{
		F32	biasFactor = 1.0f;
		D3DXMatrixPerspectiveFovLH(
				(D3DXMATRIX*) &m_projectionMatrix,
				2.0f * (F32) atanf( (viewportDY)*0.5f / m_centreZ ),
				(F32)(viewportDX) / (F32)(viewportDY),
				biasFactor*m_zNear,
				biasFactor*m_zFar
				);
		m_pD3dWindow->GetD3dDevicePtr()->SetTransform(D3DTS_PROJECTION, &m_projectionMatrix);
	}

	Position3 newPos;
	newPos[0] = bounds.GetCentre()[0];
	newPos[1] = bounds.GetCentre()[1];
	newPos[2] = bounds.GetCentre()[2];
	
	SetPosition(newPos);
	MoveForward(-dist);
}

bool FirstPersonCamera::OnUpdateDevice()
{
	/*if (m_pD3dWindow == 0)
	{
		return false;
	}

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

    // used to compute the bottom row of the view matrix
    F32 view41,view42,view43;
    view41 = -D3DXVec3Dot(&right,&m_position);
    view42 = -D3DXVec3Dot(&up,&m_position);
    view43 = -D3DXVec3Dot(&facing,&m_position);

    D3DXMATRIX cameraMatrix = 
		D3DXMATRIX( right.x, up.x, facing.x, 0.0f,
					right.y, up.y, facing.y, 0.0f,
					right.z, up.z, facing.z, 0.0f,
					view41, view42, view43, 1.0f);

	  	// save our newly calculated vectors
	m_right= right;
	m_up = up;
	m_facing = facing;

    HRESULT hr;

    // update the device - if we fail then return the error code
	hr = m_pD3dWindow->GetD3dDevicePtr()->SetTransform(D3DTS_VIEW,&cameraMatrix);
    if (FAILED(hr)){
        return false;
    }

    // reset the various variables to reflect the new update
    m_rotAboutUp = m_rotAboutRight = m_rotAboutFacing = 0.0f;
*/
	return true;
}

void FirstPersonCamera::OnResize()
{
}

void FirstPersonCamera::Reset()
{
	Camera::Reset();
}

void FirstPersonCamera::Clear()
{
	Camera::Clear();
	Reset();
}
