#include "stdafx.h"

#include "ui/Game/effects.h"
#include "ui/Game/UIXt.h"
#include "ui/Game/Gadgets/BaseUIVariableSelect.h"
#include "ui/UiInterface.h"
#include "ui/Game/UIMaths.h"

#ifdef CFG_ALLOW_OPTIMIZE_OFF_UI
#pragma optimize("", off)
#endif

namespace UI
{

void UIEffect::DoEffect(F32 timer,F32 *to_var1,F32 *to_var2,F32 *to_var3)
{
	*to_var1=m_Var[0]-((m_Var[0]-m_Var[3])*timer);
	*to_var2=m_Var[1]-((m_Var[1]-m_Var[4])*timer);
	*to_var3=m_Var[2]-((m_Var[2]-m_Var[5])*timer);
}

void UIEffect::DoEffect(F32 timer,F32 *to_var1,F32 *to_var2)
{
	*to_var1=m_Var[0]-((m_Var[0]-m_Var[3])*timer);
	*to_var2=m_Var[1]-((m_Var[1]-m_Var[4])*timer);
}

void UIEffect::DoEffect(F32 timer,F32 *to_var1)
{
	*to_var1=m_Var[0]-((m_Var[0]-m_Var[3])*timer);
}

void UIEffect::DoEffect(F32 timer,S32 *to_var)
{
	C_UIColour val;

	val.A( m_Col[ 0 ].A() - U8( ( m_Col[ 0 ].A() - m_Col[ 1 ].A() ) * timer ) );
	val.R( m_Col[ 0 ].R() - U8( ( m_Col[ 0 ].R() - m_Col[ 1 ].R() ) * timer ) );
	val.G( m_Col[ 0 ].G() - U8( ( m_Col[ 0 ].G() - m_Col[ 1 ].G() ) * timer ) );
	val.B( m_Col[ 0 ].B() - U8( ( m_Col[ 0 ].B() - m_Col[ 1 ].B() ) * timer ) );

	*to_var=(S32)val;
}

bool UIEffect::CanWeDoEffect(SCREEN_TIME stime)
{
	bool doit=true;

	if(m_When == EFFECT_WHEN_START && stime != SCREEN_TIME_START) 
		doit=false;

	if(m_When == EFFECT_WHEN_END && stime != SCREEN_TIME_END) 
		doit=false;
	
	if(m_When == EFFECT_WHEN_MAINLOOP && stime != SCREEN_TIME_NORMAL) 
		doit=false;

	if(m_When == EFFECT_WHEN_SELECTED || m_When == EFFECT_WHEN_LOCKED || m_When == EFFECT_WHEN_SB_TOP || m_When == EFFECT_WHEN_SB_YOU)
		doit=false;

	return doit;
}


void UIEffect::Update(F32 current_time,C_GadgetBaseData *pData,SCREEN_TIME stime,bool overide, const S32 sCurve )
{
	//do the effects
	if(CanWeDoEffect(stime) || overide)
	{
		F32 temp_timer;

		//bool bDoEffect = false;

		//if ( ( ( current_time >= m_StartTime ) && ( current_time <= ( m_StartTime + m_Duration ) ) ) )
		//{
		//	bDoEffect = true;
		//	m_PrevTime = current_time;
		//}
		//else if ( m_PrevTime < ( m_StartTime + m_Duration ) )
		//{
		//	bDoEffect = true;
		//	current_time = m_StartTime + m_Duration;
		//	m_PrevTime = current_time;
		//}

		//if ( bDoEffect )
		if ( current_time >= m_StartTime )
		{
			if(m_Sound > -1)
			{
				UiInterface()->PlaySound( m_Sound );
				m_Sound=-1;
			}

			// #aappleby - Patching around a divide-by-zero here
			if(m_Duration == 0.0f)
			{
				temp_timer = (current_time<m_StartTime) ? 0.0f : 1.0f;
			}
			else
			{
				temp_timer = (current_time-m_StartTime) / m_Duration; // need to convert to between 0.0f -> 1.0f
			}

			if ( temp_timer < 0.0f || temp_timer > 1.0f )
				return;	// to stop effects overwriting

			temp_timer = Maths::Clamped(temp_timer,0.0f,1.0f);

			if(temp_timer >= 1.0f)
			{
				if(m_EndSound > -1)
				{
					UiInterface()->PlaySound( m_EndSound );
					m_EndSound=-1;
				}
			}
/*
			// pull the 01 time from a curve if required
			if ( sCurve != -1 )
				temp_timer = g_UIHermiteCurves.EvaluateCurve( sCurve, temp_timer );
*/
			{
				F32 tmp = 0.0f;

				switch(m_Type)
				{
				case EFFECT_TYPE_MOVE:
					DoEffect(temp_timer,&pData->m_Position.x,&pData->m_Position.y,&pData->m_Position.z);
					break;
				case EFFECT_TYPE_MOVEX:
					DoEffect(temp_timer,&pData->m_Position.x);
					break;
				case EFFECT_TYPE_MOVEY:
					DoEffect(temp_timer,&pData->m_Position.y);
					break;
				case EFFECT_TYPE_MOVEZ:
					DoEffect(temp_timer,&pData->m_Position.z);
					break;
				case EFFECT_TYPE_SIZE:
					DoEffect(temp_timer,&pData->m_Size.x,&pData->m_Size.y,&tmp);
					break;
				case EFFECT_TYPE_SIZEX:
					DoEffect(temp_timer,&pData->m_Size.x);
					break;
				case EFFECT_TYPE_SIZEY:
					DoEffect(temp_timer,&pData->m_Size.y);
					break;
				case EFFECT_TYPE_SCALE:
					DoEffect(temp_timer,&pData->m_Scale.x,&pData->m_Scale.y,&pData->m_Scale.z);
					break;
				case EFFECT_TYPE_SCALEX:
					DoEffect(temp_timer,&pData->m_Scale.x);
					break;
				case EFFECT_TYPE_SCALEY:
					DoEffect(temp_timer,&pData->m_Scale.y);
					break;
				case EFFECT_TYPE_COLOUR:
				{
					S32 my_colour;
					DoEffect(temp_timer,&my_colour);	// Calc new colour

					pData->m_SpriteColours = my_colour;		// Assign to all elements of draw colour using 'SpriteColour ='
					break;
				}
				case EFFECT_TYPE_ALPHA:
				{
					F32 alpha;
					DoEffect(temp_timer,&alpha);
					U8 a8 = (U8)Maths::Clamped(alpha,0.0f,255.0f);

					for(U32 i = 0; i < C_UISpriteColours::m_SpriteColoursSize; i++)
						pData->m_SpriteColours[i].A( a8 );

					break;
				}
				case EFFECT_TYPE_COLOUR0:
					DoEffect(temp_timer,(S32 *)&pData->m_SpriteColours[0]);
					break;
				case EFFECT_TYPE_COLOUR1:
					DoEffect(temp_timer,(S32 *)&pData->m_SpriteColours[1]);
					break;
				case EFFECT_TYPE_COLOUR2:
					DoEffect(temp_timer,(S32 *)&pData->m_SpriteColours[2]);
					break;
				case EFFECT_TYPE_COLOUR3:
					DoEffect(temp_timer,(S32 *)&pData->m_SpriteColours[3]);
					break;
				case EFFECT_TYPE_TEXTUREMOVE0:
					DoEffect(temp_timer,&pData->m_TextureParams[0].m_Position.u,&pData->m_TextureParams[0].m_Position.v,&tmp);
					break;
				case EFFECT_TYPE_TEXTUREMOVE1:
					DoEffect(temp_timer,&pData->m_TextureParams[1].m_Position.u,&pData->m_TextureParams[1].m_Position.v,&tmp);
					break;
				case EFFECT_TYPE_TEXTUREMOVE2:
					DoEffect(temp_timer,&pData->m_TextureParams[2].m_Position.u,&pData->m_TextureParams[2].m_Position.v,&tmp);
					break;
				case EFFECT_TYPE_TEXTURESIZE0:
					DoEffect(temp_timer,&pData->m_TextureParams[0].m_Size.u,&pData->m_TextureParams[0].m_Size.v,&tmp);
					break;
				case EFFECT_TYPE_ROTATE:
					DoEffect(temp_timer,&pData->m_Rotation.x,&pData->m_Rotation.y,&pData->m_Rotation.z);
					break;
				case EFFECT_TYPE_ANIMATE:
				{
					F32 inc=1.0f/m_Var[0];
					
					S32 frame=S32(temp_timer*m_Var[0]);
					
					S32 stage=(S32)m_Var[2];

					pData->m_TextureParams[stage].m_Position.u=inc*frame;
					pData->m_TextureParams[stage].m_Position.v=0;

					pData->m_TextureParams[stage].m_Size.u=inc;
					pData->m_TextureParams[stage].m_Size.v=1.0f;
					break;
				}
				default:
					LOGWARN( g_Log,  "Effect %d Not currently supported\n", m_Type );
					break;
				}
			}
		}
	}
}

}; // namespace UI

#ifdef CFG_ALLOW_OPTIMIZE_OFF_UI
#pragma optimize("", on) 
#endif
