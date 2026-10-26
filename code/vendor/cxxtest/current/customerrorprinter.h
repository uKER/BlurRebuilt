#ifndef __cxxtest__CustomErrorPrinter_h__
#define __cxxtest__CustomErrorPrinter_h__

//
// The ErrorPrinter is a simple TestListener that
// just prints "OK" if everything goes well, otherwise
// reports the error in the format of compiler messages.
// The ErrorPrinter uses std::cout
//

#include <cxxtest/Flags.h>
#include <core/core.h>
#ifdef PLATFORM_PC
#include <windows.h>
#endif

#ifndef _CXXTEST_HAVE_STD
#   define _CXXTEST_HAVE_STD
#endif // _CXXTEST_HAVE_STD

#include <cxxtest/ErrorFormatter.h>
#include <cxxtest/StdValueTraits.h>
#include <system/memory/memorytracker.h>

#ifdef _CXXTEST_OLD_STD
#   include <iostream.h>
#else // !_CXXTEST_OLD_STD
#   include <iostream>
#endif // _CXXTEST_OLD_STD

namespace CxxTest 
{
    class CustomErrorPrinter : public ErrorFormatter
    {
    public:
		static char m_MemoryTrackerBuffer[1024*1024];

        CustomErrorPrinter( CXXTEST_STD(ostream) &o = CXXTEST_STD(cout), const char *preLine = "(", const char *postLine = ")" ) :
            ErrorFormatter( new Adapter(o), preLine, postLine ) {}
        virtual ~CustomErrorPrinter() { delete outputStream(); }

		void enterTest( const TestDescription &td )
		{
			Memory::TrackNodes::Instance().IncrementTag();

			ErrorFormatter::enterTest(td);
		}

		void leaveTest( const TestDescription &td )
		{
			ErrorFormatter::leaveTest(td);

			Memory::Tracker::Instance().DumpAllocatedBlocks(Memory::TrackNodes::Instance().Tag());
		}

    private:
        class Adapter : public OutputStream
        {
            CXXTEST_STD(ostream) &_o;
        public:
            Adapter( CXXTEST_STD(ostream) &o ) : _o(o) {}
            void flush() { _o.flush(); }
            OutputStream &operator<<( const char *s )
			{
#ifdef PLATFORM_PC
				OutputDebugString(s);
#endif
				_o << s;
				return *this;
			}

            OutputStream &operator<<( Manipulator m ) { return OutputStream::operator<<( m ); }
            OutputStream &operator<<( unsigned i )
            {
                char s[1 + 3 * sizeof(unsigned)];
                numberToString( i, s );
                *this << s;
                return *this;
            }
        };
    };
}

#endif // __cxxtest__CustomErrorPrinter_h__
