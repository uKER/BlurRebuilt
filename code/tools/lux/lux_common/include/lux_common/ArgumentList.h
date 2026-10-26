#pragma once
 
class ArgumentList 
{
private:
   int     m_argc;
   std::vector< LPCTSTR > m_argv;
 
public:
   ArgumentList( LPCTSTR /* pszCmdLine */ );
 
   int getNumArgs( void ) const;
   LPCTSTR getArg( int nArg ) const;
   LPCTSTR getArg( int nArg, bool bConsume = false );
   bool isOption( int nArg ) const; // ?? 
   bool hasOption( LPCTSTR pszOption );
   void consume( int nArg );
};
 
 
inline int ArgumentList::getNumArgs( void ) const {
 
   return m_argc;
}
 
 
inline LPCTSTR ArgumentList::getArg( int nArg ) const {
 
   assert( 0 <= nArg && nArg < m_argc );
   return m_argv[ nArg ];
}
 
 
inline LPCTSTR ArgumentList::getArg( int nArg, bool bConsume ) {
 
   assert( 0 <= nArg && nArg < m_argc );
   const LPCTSTR pszArgument = m_argv[ nArg ];
   if ( bConsume ) {
      consume( nArg );
   }
   return pszArgument;
}
 
 
/**
 * An argument is considered an option
 * if preceeded by a slash or a minus.
 * The point of call can choose to consider unrecognized
 * options to be file names. TextEdit does this.
 */
inline bool ArgumentList::isOption( int nArg ) const {
 
   assert( 0 <= nArg && nArg < m_argc );
 
   LPCTSTR const pszArg = getArg( nArg );
   return _T( '/' ) == *pszArg || _T( '-' ) == *pszArg;
}
 
// end of file
