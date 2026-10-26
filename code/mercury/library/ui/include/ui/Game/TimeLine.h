#ifndef UITIMELINE_H
#define UITIMELINE_H

#ifdef _MSC_VER
#pragma once
#endif

#include <Xt/XtGame.h>

#include "ui/Game/UiXt.h"

namespace UI
{
class C_GadgetBase;
class C_UIXtHermiteProxy;
class C_UIXtHermite1D;

/// Timeline : contains one time and multiple effects, gadgets to be animated must be annotated in Xt
class C_Timeline
{
public:
	/// Default constructor
	C_Timeline( ) :
		m_Name( XtSz8(NULL) ),
		m_Type( E_Type_Wrap ),
		m_RestType( E_RestType_None ),
		m_Time( 0.0f ),
		m_bActive( false ),
		m_bDenySleep( false )
	{
	}

	/// Bind given Xt annotated class
	void		BindImpl( XtType* def, const U8* structPtr );

	/// Reset variables to their origins
	void		Reset( const bool overide = false );
	/// Update the timer and effects
	void		Update( const F32 time );

	/// Sleeping time - no processing at all
	const bool	IsSleeping();

	/// Timeline active
	const bool	Active();
	void		Active( const bool bActive );

	/// Set timeline time note this loops through bounce and wrap types
	void		SetTime( const F32 newtime );

private:
	class C_Direction
	{
	public:
		enum E_Mode
		{
			E_Mode_None,
			E_Mode_Inc,
			E_Mode_Dec,
			E_Mode_PauseInc,
			E_Mode_PauseDec,
		};
	public:
		C_Direction() : m_Mode( E_Mode_None ) { }
		C_Direction( const E_Mode _mode ) : m_Mode( _mode ) { }

		const F32		Amount() const;
		const bool		IsPaused() const;
		const E_Mode	Mode() const;

		void			Pause();
		void			Resume();
		void			Reverse();
		void			Mode( const E_Mode _mode );

	private:
		E_Mode			m_Mode;
	};

private:
	void				UpdateImpl( F32 _time, C_Direction& _speed, bool _run_commands );
	void				DoLuaCommand( const XtSz8& cmd );

public:
	/// Timeline timer types
	enum E_Type
	{
		E_Type_Wrap = 0,			///< Wrap round to begining
		E_Type_Bounce,				///< Reverse direction in ping-pong style
		E_Type_Stop,				///< Run until the end and stop
	};
	/// Timeline timer rest types
	enum E_RestType
	{
		E_RestType_None = 0,		///< Stays where it is when not active
		E_RestType_Start,			///< Runs until it reaches the start again when not active
		E_RestType_End,				///< Runs until it reaches the end when not active
	};
	/// XtType for timeline time range
	class C_TimeRange
	{
	public:
		XtF32 start;
		XtF32 end;

		XtF32	Length( )
		{ 
			return end-start; 
		}
	};
	/// Generic effect container, can be assigned to any annotated atomic variable in Xt
	class C_Effect
	{
	public:
		/// Range interface base class
		class I_Range
		{
		public:
			enum E_Type
			{
				E_Type_None = -1,
				E_Type_Range,
				E_Type_Lerp,
			};
		public:
			I_Range( E_Type type = E_Type_None ) : m_Type( type ) { }

			const E_Type	Type( )	{ return m_Type; }
		private:
			E_Type	m_Type;
		};
		/// Templated interpolation range class
		template <typename _Type>
		class C_Range : public I_Range
		{
		public:
			C_Range( ) : I_Range( I_Range::E_Type_Lerp ) { }

		public:
			_Type m_From;
			_Type m_To;
		};
		/// Templated array switch range class
		template <typename _Type>
		class C_Array : public I_Range
		{
		public:
			C_Array( ) : I_Range( I_Range::E_Type_Range ) { }

		public:
			XtArray< _Type >	m_Items;
		};
		/// Effect modifier interface base class
		class I_Modifier
		{
		public:
			virtual ~I_Modifier( ) { }
			virtual void			Reset( const bool overide = false ) = 0;
			virtual void			Apply( const F32 time ) = 0;
		};

		/// Handler for callbacks when an effect has changed a variable, usese XtEventCallback( user )
		class C_HasChanged
		{
		public:
			C_HasChanged( ) : callback(0), base( NULL ), type( 0 )
			{ 

			}

		public:
			XtEventCallback					callback;
			S32								type;
			void *							base;
		};

		/// Modifier tempate base
		template < typename _Type >
		class C_Modifier : public I_Modifier
		{
		public:
			C_Modifier( _Type* t, C_HasChanged& hc = NULL ) :
				m_pComponent( t ), 
				m_HasChanged( hc )
			{
				// Store origin
				if( m_pComponent )
					m_Origin = *m_pComponent;
			}

			void Reset( const bool overide = false )
			{
				if( !m_pComponent )
					return;

				*m_pComponent = m_Origin;
			}

		protected:
			_Type				m_Origin;
			_Type *				m_pComponent;
			C_HasChanged		m_HasChanged;
		};

		/// Interpolation modifier
		template < typename _RangeType, typename _ModType >
		class C_ModifierLerp : public C_Modifier< _ModType >
		{
		public:
			C_ModifierLerp( C_Range< _RangeType >& in, _ModType* t, C_HasChanged& hc ) : 
				m_Range( in ),
				C_Modifier< _ModType >( t, hc )
			{
			}

			void Apply( const F32 time );
		private:
			C_Range< _RangeType >	m_Range;
		};

		/// Switch between array elements modifier
		template < typename _RangeType, typename _ModType >
		class C_ModifierArray : public C_Modifier< _ModType >
		{
		public:
			C_ModifierArray( C_Array< _RangeType >& in, _ModType* t, C_HasChanged& hc ) : 
				m_Array( in ), 
				C_Modifier< _ModType >( t, hc )
			{
			}

			void			Apply( const F32 time );

		private:
			C_Array< _RangeType >&	m_Array;
		};

	public:
		/// Default constructor
		C_Effect();

		~C_Effect();

		/// Bind effect to variable by searching annotation for matching string 
		void Bind( XtType* def, const U8* structPtr );

		/// Reset all the modifiers back to their origin
		void Reset( )
		{
			for( S32 i = 0; i < m_Modifiers.Size(); i++ )
				m_Modifiers[ i ]->Reset();
		}

		/// Apply all modifiers
		void Apply(const F32 delta);
		/// Evaluate curve data
		F32	 Evaluate(const F32 delta);

	public:
		XtSz8								m_ComponentName;
		XtArray< C_UIXtHermiteKeyFrame >	m_KeyFrames;
		I_Range *							m_pAmounts;
		C_HasChanged						m_HasChanged;

	private:
		Btl::Vector< I_Modifier * >			m_Modifiers;
		C_UIXtHermite1D*					m_pCurveEval;
		C_UIXtHermiteProxy*					m_pCurveData;
	};
	/// Event container - for the time being they will call a line of lua when the event moment occurs
	class C_Event
	{
	public:
		XtSz8			m_Command;
		XtF32			m_Time;
	};

public:
	XtSz8				m_Name;
	XtSz8				m_OnRestCmd;
	XtSz8				m_OnStartCmd;
	bool				m_bActive;
	bool				m_bDenySleep;
	C_TimeRange			m_Range;
	E_Type				m_Type;
	E_RestType			m_RestType;
	XtArray< C_Effect >	m_Effects;
	XtArray< C_Event >	m_Events;

private:
	F32					m_Time;
	C_Direction			m_Direction;
};

}; // namespace UI
#endif // UITIMELINE_H
