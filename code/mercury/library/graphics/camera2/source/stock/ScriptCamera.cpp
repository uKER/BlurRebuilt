
#include "stdafx.h"
#include "camera2/CameraType.h"
#include "camera2/stock/ScriptCamera.h"
#pragma warning (push, 3)
#include <System/Serialise/I_FileSystem.h>
#include <Framework/application/CoreSystems.h>
#pragma warning (pop)

namespace Cam2
{

// A script camera, loads a file from 'camscript.txt' and plays it back

PreKey::PreKey()
{
	flags = 0;
	x = y = z = ow = ox = oy = oz = fov = period = 0.f;
	idepth = aperture = 0.f;
	gameSpeed = 1.0f;
}

CamState::CamState()
{
	fov = Maths::DegToRad(60.f);
	idepth = aperture = 0.f;
}


C_ScriptCamera::C_ScriptCamera()
{
	SetType(E_ScriptCamera);
	SetName("Script");
	m_script = 0;
	m_scriptPos = m_scriptCount = 0;
	memset(m_scriptLine, 0, sizeof(m_scriptLine));
	m_counter = 0.f;
	m_GameSpeed = 1.0f;
	
	LoadScript();
}

C_ScriptCamera::~C_ScriptCamera()
{
	delete [] m_script; // Free previous script
	m_script = 0;
}

void C_ScriptCamera::OnUpdate()
{
	UpdateScript();
}

void C_ScriptCamera::LoadScript()
{
	S32 len = 0;
	char name[128] = "camscript.txt";
	
	if (m_script)
	{
		delete[] m_script; // Free previous script
		m_script = 0;
	}

	m_scriptPos = 0;
	m_scriptCount = 0;

	memset(&m_key, 0, sizeof(m_key)); // Blank keyframes
	memset(&m_dot, 0, sizeof(m_dot));
	
	Sys::I_FileSystem& fs = *Sys::GetDefaultFileSystem();
	Sys::I_File* file = fs.Open(name, Sys::E_FileAccess_Read);
	
	if (file == 0) return;
	
	// Get length of script:
	len = (S32)file->Length();
	
	m_script = new char[len+1];
	if (m_script == 0)
	{
		delete file;
		return;
	}

	memset(m_script, 0, len+1);
	
	// Read in script to memory:
	file->Read(m_script, len);
	delete file;
	file = 0;

	// Start feeding lines into the keyframe stack:
	m_counter = 0.f;
	ScriptNextLine();
	ScriptNextLine();
	m_key[2].period = 0.f; // Go instantly to first frame
}

S32 C_ScriptCamera::ScriptNextLine()
{
	if (m_script == 0)
		return 1;

	memset(m_scriptLine, 0, sizeof(m_scriptLine));

	// Get the next line of the script:
	S32 len = (S32)strcspn(m_script + m_scriptPos, "\r\n");
	if (len <= 0)
		return 1; // End of script

	if ((U32)len > sizeof(m_scriptLine) - 1)
		len = sizeof(m_scriptLine) - 1;

	// Copy line
	memcpy(m_scriptLine, m_script + m_scriptPos, len);
	m_scriptPos += len;

	// Skip to next line:
	m_scriptPos += (S32)strspn(m_script+m_scriptPos,"\r\n");
	m_scriptCount++;
	
	// Shuffle down previous keyframes:
	S32 i = 0;
	for (i = 0; i < 3; i++)
		m_key[i] = m_key[i + 1];

	// Parse the line into keyframe 3
	ParseKeyFrame(m_key + 3, m_scriptLine);
	CheckScriptFlip();

	// Copy into m_dot, and if it's a endpoint extend m_dot[0] or m_dot[3] (for a nicer Catmull-Rom)
	memcpy(m_dot, m_key, sizeof(m_dot));
	if (m_dot[1].period <= 0.f) m_dot[0] = m_dot[1];
	if (m_dot[3].period <= 0.f) m_dot[3] = m_dot[2];

	return 0;
}

Maths::C_Quaternion EulerToQuat(F32 pitch, F32 yaw, F32 roll)
{
	F32 c1 = cos(yaw  *-0.5f); F32 s1 = sin(yaw  *-0.5f);
	F32 c2 = cos(pitch* 0.5f); F32 s2 = sin(pitch* 0.5f);
	F32 c3 = cos(roll * 0.5f); F32 s3 = sin(roll * 0.5f);

	Maths::C_Quaternion qy(0.f, s1 , 0.f, c1);
 	Maths::C_Quaternion qx(s2 , 0.f, 0.f, c2);
	Maths::C_Quaternion qz(0.f, 0.f, s3 , c3);

	return qy * qx * qz;
}

void C_ScriptCamera::ParseKeyFrame(PreKey *key, char *text)
{
	char *pt = text;
	if (pt) key->flags = strtol(pt,&pt,0);
	if (pt) key->period=(F32)strtod(pt,&pt);
	if (pt) key->x=(F32)strtod(pt,&pt);
	if (pt) key->y=(F32)strtod(pt,&pt);
	if (pt) key->z=(F32)strtod(pt,&pt);
	
	F32 pitch = 0.f, yaw = 0.f, roll = 0.f;
	if (pt) pitch = (F32)strtod(pt,&pt);
	if (pt) yaw   = (F32)strtod(pt,&pt);
	if (pt) roll  = (F32)strtod(pt,&pt);
	
	pitch = Maths::DegToRad(pitch);
	yaw   = Maths::DegToRad(yaw);
	roll  = Maths::DegToRad(roll);
	
	Maths::C_Quaternion q = EulerToQuat(pitch, yaw, roll);
	key->ow = (F32)q.Real().W();
	key->ox = (F32)q.Imag().X();
	key->oy = (F32)q.Imag().Y();
	key->oz = (F32)q.Imag().Z();

	if (pt) key->fov=(F32)strtod(pt,&pt);
	key->fov = Maths::DegToRad(key->fov);

	if (pt) key->idepth = (F32)strtod(pt, &pt);
	if (pt) key->aperture = (F32)strtod(pt, &pt);
	if (pt) key->gameSpeed = (F32)strtod(pt, &pt);
}

void C_ScriptCamera::CheckScriptFlip()
{
	F32 cosa = 0.f;
	
	cosa  = m_key[2].ow * m_key[3].ow;
	cosa += m_key[2].ox * m_key[3].ox;
	cosa += m_key[2].oy * m_key[3].oy;
	cosa += m_key[2].oz * m_key[3].oz;

	if (cosa < 0.f)
	{
		// Greater than 180 degrees away, so flip target quaternion:
		m_key[3].ow *= -1.f;
		m_key[3].ox *= -1.f;
		m_key[3].oy *= -1.f;
		m_key[3].oz *= -1.f;
	}
}

void C_ScriptCamera::UpdateScript()
{
	if (m_script == 0)
		return;
	
	F32 frac = 0.f, period = 0.f;
	S32 ret = 0;
	period=m_key[2].period;
	
	// Check if we have reached the end of the keyframe:
	if (m_counter>=period)
	{
		m_counter-=period;
		
		// Move to next line in the presentation script
		ret=ScriptNextLine();
		if (ret!=0)
		{
			// End of script
			m_scriptPos=0;
			m_scriptCount=0;
			ScriptNextLine();
		}

		period=m_key[2].period;
	}

	if (period>0.00001f) frac=m_counter/period;
	
	PresentCatmull(m_dot,frac);

	m_counter += Period();
}

// Catmull-Rom smooth interpolation between floats:
static F32 Catmull(F32 v0,F32 v1,F32 v2,F32 v3,F32 s)
{
	F32 result=0.f;
	
	result=v1*2.f;
	result+=(-v0+v2)*s;
	result+=((v0*2.f)-(v1*5.f)+(v2*4.f)-v3) * (s*s);
	result+=(-v0+(v1*3.f)-(v2*3.f)+v3) * (s*s*s);
	result*=0.5f;

	return result;
}


// Catmull-Rom smooth interpolation between vectors:
static Maths::HVec3 Catmull3(const Maths::HVec3& v0, const Maths::HVec3& v1, const Maths::HVec3& v2, const Maths::HVec3& v3, F32 s)
{
	Maths::HVec3 result(0.f,0.f,0.f);
	
	result = v1*2.f;
	result += (-v0+v2)*s;
	result += ((v0*2.f)-(v1*5.f)+(v2*4.f)-v3) * (s*s);
	result += (-v0+(v1*3.f)-(v2*3.f)+v3) * (s*s*s);
	result = result*0.5f;

	return result;
}

// Catmull-Rom smooth interpolation between quaternions:
static Maths::C_Quaternion Catmull4(const Maths::C_Quaternion& q0, const Maths::C_Quaternion& q1, const Maths::C_Quaternion& q2, const Maths::C_Quaternion& q3, F32 s)
{
	Maths::HVec4 v0=q0,v1=q1,v2=q2,v3=q3,result;
	
	result = v1*2.f;
	result = result+(-v0+v2)*s;
	result = result+((v0*2.f)-(v1*5.f)+(v2*4.f)-v3) * (s*s);
	result = result+(-v0+(v1*3.f)-(v2*3.f)+v3) * (s*s*s);
	result = result*0.5f;
	
	return (Maths::C_Quaternion)result;
}

void C_ScriptCamera::PresentCatmull(PreKey *key,float frac)
{
	CamState c[4];
	S32 i = 0;
	
	memset(&c,0,sizeof(c));
	
	for (i=0;i<4;i++) GetFrame(c+i,key+i);
	
	SetPosition( Catmull3(c[0].position, c[1].position, c[2].position, c[3].position, frac) );

	Maths::C_Quaternion orient = Catmull4(c[0].orient, c[1].orient, c[2].orient, c[3].orient, frac); 
	orient.Normalise();
	SetOrient( orient );
	
	SetFov( Catmull(c[0].fov, c[1].fov, c[2].fov, c[3].fov, frac) );
	if (Fov() < 0.0000001f) SetFov(Maths::DegToRad(60.f));

	SetIdepth  ( Catmull(c[0].idepth,   c[1].idepth,   c[2].idepth,   c[3].idepth, frac) );
	SetAperture( Catmull(c[0].aperture, c[1].aperture, c[2].aperture, c[3].aperture, frac) );

	// Change frac to an ease in/out curve:
	float ease = 0.5f - cos(frac * Pi) * 0.5f;
	m_GameSpeed = c[1].gameSpeed + (c[2].gameSpeed - c[1].gameSpeed) * ease;
}

// Get a presentation frame as a world position/world orientation
void C_ScriptCamera::GetFrame(CamState *pcam, PreKey *paf)
{
	CamState c;

	c.position = Maths::HVec3(paf->x,paf->y,paf->z);
	c.orient = Maths::C_Quaternion(paf->ox, paf->oy, paf->oz, paf->ow);
	c.fov = paf->fov;

	if (paf->flags & 1)
	{
		// Place it relative to the object
		Maths::HVec3 relative = (Maths::HVec3)(Object().orient * Maths::C_Quaternion(c.position, Maths::HVecW(0.f)) * !Object().orient); // relative vector in world space
		c.position = Object().position + relative;
		c.orient = Object().orient * c.orient;
	}

	c.idepth = paf->idepth;
	c.aperture = paf->aperture;
	c.gameSpeed = paf->gameSpeed;
	
	*pcam=c;
}

float C_ScriptCamera::GameSpeed() const
{
	return m_GameSpeed;
}

} // End of namespace Cam2
