#include "stdafx.h"

#include <scene_renderer/Common.h>
#include <scene_renderer/SinCos.h>
#include <scene_renderer/D3dWindow.h>

#include <scene_renderer/Camera.h>
#include <scene_renderer/TargetCamera.h>

using namespace SRD;

TargetCamera::TargetCamera() : Camera()
{
	Reset();
}

TargetCamera::~TargetCamera()
{
    Clear();
}

TargetCamera::TargetCamera(D3dWindow* pD3dWindow) : Camera(pD3dWindow)
{
	Reset();

	m_rotAboutRight = 0.3f;
}

SrCameraTypeEnum TargetCamera::GetType() const
{
	return SR_CAMERA_TYPE_TARGET;
}

void TargetCamera::MoveInZ(F32 distance)
{
	m_zDistance += distance;
}

void TargetCamera::SetZDistance(F32 distance)
{
	m_zDistance = distance;
}

F32 TargetCamera::GetZDistance() const
{
	return m_zDistance;
}

void TargetCamera::Frame(const LPVS::Bounds& bounds)
{
	F32 dist = sqrtf(bounds.GetBoundingBoxRadiusSquared()) + 0.1f;

	U32 viewportDX = m_pD3dWindow->GetWidth();
	U32 viewportDY = m_pD3dWindow->GetHeight();

	F32	yFov;
	F32 xFov;
	F32 fov = (F32)LPVS::DegToRad(56.303f);

	m_centreZ = tan(fov) * ((F32)(viewportDX)/2.0f);

	m_fov = 2.0f * (F32) atanf( (viewportDY)*0.5f / m_centreZ );

	yFov = (F32)atan( (viewportDY / m_centreZ)*0.5f);
	xFov = (F32)atan( (viewportDX / m_centreZ)*0.5f);
	fov = min(yFov,xFov);
//	m_fov = fov;
	dist /= (F32)sin(fov);

	{
		F32	biasFactor = 1.0f;
		D3DXMatrixPerspectiveFovLH(
				(D3DXMATRIX*) &m_projectionMatrix,
				m_fov,
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
	m_zDistance = dist;
}

bool TargetCamera::OnUpdateDevice()
{
	if (m_pD3dWindow == 0)
	{
		return false;
	}

	{
		D3DXMATRIX matTotal;
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

    	// save our newly calculated vectors
		m_right= right;
		m_up = up;
		m_facing = facing;

		F32 view41 = -D3DXVec3Dot(&right,&m_origin);
		F32 view42 = -D3DXVec3Dot(&up,&m_origin);
		F32 view43 = -D3DXVec3Dot(&facing,&m_origin);

		m_viewMatrix = D3DXMATRIX ( 
			right.x, up.x, facing.x, 0.0f,
			right.y, up.y, facing.y, 0.0f,
			right.z, up.z, facing.z, 0.0f,
			view41, view42, view43, 1.0f);
	}

	D3DXMATRIX dollyTranslation = D3DXMATRIX(
		1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		0,0, m_zDistance, 1.0f);

	m_viewMatrix = m_viewMatrix * dollyTranslation;

	Matrix44 mView;
	mView[0][0] = m_viewMatrix._11; mView[0][1] = m_viewMatrix._12; mView[0][2] = m_viewMatrix._13; mView[0][3] = m_viewMatrix._14;
	mView[1][0] = m_viewMatrix._21; mView[1][1] = m_viewMatrix._22; mView[1][2] = m_viewMatrix._23; mView[1][3] = m_viewMatrix._24;
	mView[2][0] = m_viewMatrix._31; mView[2][1] = m_viewMatrix._32; mView[2][2] = m_viewMatrix._33; mView[2][3] = m_viewMatrix._34;
	mView[3][0] = m_viewMatrix._41; mView[3][1] = m_viewMatrix._42; mView[3][2] = m_viewMatrix._43; mView[3][3] = m_viewMatrix._44;
/*
	Matrix44 mWorld;
	InvertOrthogonal(&mWorld, mView);

	m_worldMatrix._11 = mWorld[0][0]; m_worldMatrix._12 = mWorld[0][1]; m_worldMatrix._13 = mWorld[0][2]; m_worldMatrix._14 = mWorld[0][3];
	m_worldMatrix._21 = mWorld[1][0]; m_worldMatrix._22 = mWorld[1][1]; m_worldMatrix._23 = mWorld[1][2]; m_worldMatrix._24 = mWorld[1][3];
	m_worldMatrix._31 = mWorld[2][0]; m_worldMatrix._32 = mWorld[2][1]; m_worldMatrix._33 = mWorld[2][2]; m_worldMatrix._34 = mWorld[2][3];
	m_worldMatrix._41 = mWorld[3][0]; m_worldMatrix._42 = mWorld[3][1]; m_worldMatrix._43 = mWorld[3][2]; m_worldMatrix._44 = mWorld[3][3];
*/
	D3DXMatrixIdentity(&m_worldMatrix);

    HRESULT hr;

    // update the device - if we fail then return the error code
	hr = m_pD3dWindow->GetD3dDevicePtr()->SetTransform(D3DTS_VIEW,&m_viewMatrix);
    if (FAILED(hr)){
        return false;
    }

	m_pD3dWindow->GetD3dDevicePtr()->SetTransform(D3DTS_WORLD,&m_worldMatrix);


    // reset the various variables to reflect the new update
    m_rotAboutUp = m_rotAboutRight = m_rotAboutFacing = 0.0f;

	return true;
}

void TargetCamera::OnResize()
{
	F32 aspect= (F32)(m_pD3dWindow->GetWidth()) / (F32)(m_pD3dWindow->GetHeight());

	D3DXMatrixPerspectiveFovLH(
			(D3DXMATRIX*) &m_projectionMatrix,
			m_fov,
			aspect,
			m_zNear,
			m_zFar);

	m_pD3dWindow->GetD3dDevicePtr()->SetTransform(D3DTS_PROJECTION, &m_projectionMatrix);
}

void TargetCamera::Reset()
{
	Camera::Reset();

	m_zDistance = 0;
}

void TargetCamera::Clear()
{
	Camera::Clear();
	Reset();
}
