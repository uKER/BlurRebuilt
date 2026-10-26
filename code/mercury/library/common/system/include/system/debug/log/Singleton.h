#ifndef __SYSTEM_LOG_SINGLETON_H__
#define __SYSTEM_LOG_SINGLETON_H__

namespace Debug
{

// singleton with lazy initialization and automatic destruction
template <typename T>
class Singleton
{
public:
				// initialize pointer to 0
				Singleton()
				{
				}

				// destroy singleton on program exit
				~Singleton()
				{
					if (s_pInstance)
						delete s_pInstance;
				}

				// retrieve singleton instance, create if needed
	static T	&Instance()
				{ 
					if (!s_pInstance)
						s_pInstance = new T;
					return *s_pInstance;
				}
private:
	static T	*s_pInstance;
};

}

#endif
