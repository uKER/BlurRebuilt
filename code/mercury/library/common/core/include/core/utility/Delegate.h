#ifndef CORE_DELEGATE_H
#define CORE_DELEGATE_H

#ifdef _MSC_VER
#pragma once
#endif

/* Adaptation of the Delegate classes found at http://www.codeproject.com/KB/cpp/CppDelegateImplementation.aspx
 * Alternative implementations :
 *  - http://www.codeproject.com/KB/cpp/ImpossiblyFastCppDelegate.aspx
 *  - http://www.codeproject.com/KB/cpp/FastDelegate.aspx
 */

//example usage:
// Ut::Delegate<int()> d(&MyClass::MyParameterlessFunctionReturningInt);

namespace Ut
{
	namespace Internal
	{
		class DummyClass;

		template <typename R>
		struct DelegateStrategy0
		{
			DummyClass *m_pObject;

			union GenericPointer
			{
				typedef R (*FuncPtr)();
				typedef R (DummyClass::*MemFuncPtr)();
				U8 unused1[sizeof(FuncPtr)];
				U8 unused2[sizeof(MemFuncPtr)];
			} m_pFunction;

			virtual R operator()()
			{
				assert(false && "Error: no overriden delegate strategy");
				return R();
			}
		};

		template <typename R>
		struct DelegateStrategyStatic0 : public DelegateStrategy0<R>
		{
			DelegateStrategyStatic0(R (*func)())
			{
				m_pObject = 0;
				reinterpret_cast<GenericPointer::FuncPtr&>(m_pFunction) = func;
			}

			R operator()()
			{
				return (*(*(reinterpret_cast<GenericPointer::FuncPtr*>(&m_pFunction))))();
			}
		};

		template <typename R>
		struct DelegateStrategyMember0 : public DelegateStrategy0<R>
		{
			template <typename T>
			DelegateStrategyMember0(T* pObject, R (T::*func)())
			{
				//fwd declare unknown class, ensure member function pointer sizes match
				class UnknownClass;
				typedef R (UnknownClass::*UnknownPtr)();

				BIZ_STATIC_ASSERT(sizeof(UnknownPtr)==sizeof(GenericPointer::MemFuncPtr));
				m_pObject = reinterpret_cast<DummyClass*>(pObject);
				
				//now declare unknown class of type T. Ensure function pointer size hasn't changed
				class UnknownClass : public T { };
				BIZ_STATIC_ASSERT(sizeof(UnknownPtr)==sizeof(GenericPointer::MemFuncPtr));

				reinterpret_cast<UnknownPtr&>(m_pFunction) = func;
			}

			R operator()()
			{
				return (m_pObject->*(*(reinterpret_cast<GenericPointer::MemFuncPtr*>(&m_pFunction))))();
			}
		};



		template <typename R, typename P1>
		struct DelegateStrategy1
		{
			DummyClass *m_pObject;

			union GenericPointer
			{
				typedef R (*FuncPtr)(P1);
				typedef R (DummyClass::*MemFuncPtr)(P1);
				U8 unused1[sizeof(FuncPtr)];
				U8 unused2[sizeof(MemFuncPtr)];
			} m_pFunction;

			virtual R operator()(P1 p1)
			{
				assert(false && "Error: no overriden delegate strategy");
				return R();
			}
		};

		template <typename R, typename P1>
		struct DelegateStrategyStatic1 : public DelegateStrategy1<R, P1>
		{
			DelegateStrategyStatic1(R (*func)(P1))
			{
				m_pObject = 0;
				reinterpret_cast<GenericPointer::FuncPtr&>(m_pFunction) = func;
			}

			R operator()(P1 p1)
			{
				return (*(*(reinterpret_cast<GenericPointer::FuncPtr*>(&m_pFunction))))(p1);
			}
		};

		template <typename R, typename P1>
		struct DelegateStrategyMember1 : public DelegateStrategy1<R, P1>
		{
			template <typename T>
			DelegateStrategyMember1(T* pObject, R (T::*func)(P1))
			{
				class UnknownClass;
				typedef R (UnknownClass::*UnknownPtr)(P1);

				BIZ_STATIC_ASSERT(sizeof(UnknownPtr)==sizeof(GenericPointer::MemFuncPtr));
				m_pObject = reinterpret_cast<DummyClass*>(pObject);
				
				//now declare unknown class of type T. Ensure function pointer size hasn't changed
				class UnknownClass : public T { };
				BIZ_STATIC_ASSERT(sizeof(UnknownPtr)==sizeof(GenericPointer::MemFuncPtr));

				reinterpret_cast<UnknownPtr&>(m_pFunction) = func;
			}

			R operator()(P1 p1)
			{
				return (m_pObject->*(*(reinterpret_cast<GenericPointer::MemFuncPtr*>(&m_pFunction))))(p1);
			}
		};



		template <typename R, typename P1, typename P2>
		struct DelegateStrategy2
		{
			DummyClass *m_pObject;

			union GenericPointer
			{
				typedef R (*FuncPtr)(P1, P2);
				typedef R (DummyClass::*MemFuncPtr)(P1, P2);
				U8 unused1[sizeof(FuncPtr)];
				U8 unused2[sizeof(MemFuncPtr)];
			} m_pFunction;

			virtual R operator()(P1 p1, P2 p2)
			{
				assert(false && "Error: no overriden delegate strategy");
				return R();
			}
		};

		template <typename R, typename P1, typename P2>
		struct DelegateStrategyStatic2 : public DelegateStrategy2<R, P1, P2>
		{
			DelegateStrategyStatic2(R (*func)(P1, P2))
			{
				m_pObject = 0;
				reinterpret_cast<GenericPointer::FuncPtr&>(m_pFunction) = func;
			}

			R operator()(P1 p1, P2 p2)
			{
				return (*(*(reinterpret_cast<GenericPointer::FuncPtr*>(&m_pFunction))))(p1, p2);
			}
		};

		template <typename R, typename P1, typename P2>
		struct DelegateStrategyMember2 : public DelegateStrategy2<R, P1, P2>
		{
			template <typename T>
			DelegateStrategyMember2(T* pObject, R (T::*func)(P1, P2))
			{
				class UnknownClass;
				typedef R (UnknownClass::*UnknownPtr)(P1, P2);

				BIZ_STATIC_ASSERT(sizeof(UnknownPtr)==sizeof(GenericPointer::MemFuncPtr));
				m_pObject = reinterpret_cast<DummyClass*>(pObject);
				
				//now declare unknown class of type T. Ensure function pointer size hasn't changed
				class UnknownClass : public T { };
				BIZ_STATIC_ASSERT(sizeof(UnknownPtr)==sizeof(GenericPointer::MemFuncPtr));

				reinterpret_cast<UnknownPtr&>(m_pFunction) = func;
			}

			R operator()(P1 p1, P2 p2)
			{
				return (m_pObject->*(*(reinterpret_cast<GenericPointer::MemFuncPtr*>(&m_pFunction))))(p1, p2);
			}
		};



		template <typename R, typename P1, typename P2, typename P3>
		struct DelegateStrategy3
		{
			DummyClass *m_pObject;

			union GenericPointer
			{
				typedef R (*FuncPtr)(P1, P2, P3);
				typedef R (DummyClass::*MemFuncPtr)(P1, P2, P3);
				U8 unused1[sizeof(FuncPtr)];
				U8 unused2[sizeof(MemFuncPtr)];
			} m_pFunction;

			virtual R operator()(P1 p1, P2 p2, P3 p3)
			{
				assert(false && "Error: no overriden delegate strategy");
				return R();
			}
		};

		template <typename R, typename P1, typename P2, typename P3>
		struct DelegateStrategyStatic3 : public DelegateStrategy3<R, P1, P2, P3>
		{
			DelegateStrategyStatic3(R (*func)(P1, P2, P3))
			{
				m_pObject = 0;
				reinterpret_cast<GenericPointer::FuncPtr&>(m_pFunction) = func;
			}

			R operator()(P1 p1, P2 p2, P3 p3)
			{
				return (*(*(reinterpret_cast<GenericPointer::FuncPtr*>(&m_pFunction))))(p1, p2, p3);
			}
		};

		template <typename R, typename P1, typename P2, typename P3>
		struct DelegateStrategyMember3 : public DelegateStrategy3<R, P1, P2, P3>
		{
			template <typename T>
			DelegateStrategyMember3(T* pObject, R (T::*func)(P1, P2, P3))
			{
				class UnknownClass;
				typedef R (UnknownClass::*UnknownPtr)(P1, P2, P3);

				BIZ_STATIC_ASSERT(sizeof(UnknownPtr)==sizeof(GenericPointer::MemFuncPtr));
				m_pObject = reinterpret_cast<DummyClass*>(pObject);
				
				//now declare unknown class of type T. Ensure function pointer size hasn't changed
				class UnknownClass : public T { };
				BIZ_STATIC_ASSERT(sizeof(UnknownPtr)==sizeof(GenericPointer::MemFuncPtr));

				reinterpret_cast<UnknownPtr&>(m_pFunction) = func;
			}

			R operator()(P1 p1, P2 p2, P3 p3)
			{
				return (m_pObject->*(*(reinterpret_cast<GenericPointer::MemFuncPtr*>(&m_pFunction))))(p1, p2, p3);
			}
		};

	}


	template <typename Signature>
	class Delegate;

	template <typename R>
	class Delegate<R()>
	{
	public:
		Delegate(const Delegate& rhs)
		{
			m_DelegateStrategy = rhs.m_DelegateStrategy;
		}

		Delegate(R (*func)())
		{
			new (&m_DelegateStrategy) Internal::DelegateStrategyStatic0<R>(func);
		}

		template<typename T>
		Delegate(T* pObject, R (T::*func)())
		{
			new (&m_DelegateStrategy) Internal::DelegateStrategyMember0<R>(pObject, func);
		}

		Delegate& operator=(const Delegate &rhs)
		{
			m_DelegateStrategy = rhs.m_DelegateStrategy;
			return *this;
		}

		R operator()()
		{
			return (*reinterpret_cast<Internal::DelegateStrategy0<R>*>(&m_DelegateStrategy))();
		}

		bool operator==(const Delegate& rhs)
		{
			return memcmp(this->m_DelegateStrategy.buffer, rhs.m_DelegateStrategy.buffer, sizeof(Internal::DelegateStrategy0<R>))==0;
		}
	private:
		struct Strategy
		{
			//use placement new for creating strategy objects to avoid heap allocations
			U8 buffer[sizeof(Internal::DelegateStrategy0<R>)];
		}
		m_DelegateStrategy;
	};

	template <typename R, typename P1>
	class Delegate<R(P1)>
	{
	public:
		Delegate(R (*func)(P1))
		{
			new (&m_DelegateStrategy) Internal::DelegateStrategyStatic1<R,P1>(func);
		}

		template<typename T>
		Delegate(T* pObject, R (T::*func)(P1))
		{
			new (&m_DelegateStrategy) Internal::DelegateStrategyMember1<R,P1>(pObject, func);
		}

		Delegate operator=(const Delegate &rhs)
		{
			m_DelegateStrategy = rhs.m_DelegateStrategy;
			return *this;
		}

		R operator()(P1 p1)
		{
			return (*reinterpret_cast<Internal::DelegateStrategy1<R,P1>*>(&m_DelegateStrategy))(p1);
		}

		bool operator==(const Delegate& rhs)
		{
			return memcmp(this->m_DelegateStrategy.buffer, rhs.m_DelegateStrategy.buffer, sizeof(Internal::DelegateStrategy1<R,P1>))==0;
		}
	private:
		struct Strategy
		{
			//use placement new for creating strategy objects to avoid heap allocations
			U8 buffer[sizeof(Internal::DelegateStrategy1<R,P1>)];
		}
		m_DelegateStrategy;
	};

	template <typename R, typename P1, typename P2>
	class Delegate<R(P1, P2)>
	{
	public:
		Delegate(R (*func)(P1, P2))
		{
			new (&m_DelegateStrategy) Internal::DelegateStrategyStatic2<R,P1,P2>(func);
		}

		template<typename T>
		Delegate(T* pObject, R (T::*func)(P1, P2))
		{
			new (&m_DelegateStrategy) Internal::DelegateStrategyMember2<R,P1,P2>(pObject, func);
		}

		Delegate operator=(const Delegate &rhs)
		{
			m_DelegateStrategy = rhs.m_DelegateStrategy;
			return *this;
		}

		R operator()(P1 p1, P2 p2)
		{
			return (*reinterpret_cast<Internal::DelegateStrategy2<R,P1,P2>*>(&m_DelegateStrategy))(p1,p2);
		}

		bool operator==(const Delegate& rhs)
		{
			return memcmp(this->m_DelegateStrategy.buffer, rhs.m_DelegateStrategy.buffer, sizeof(Internal::DelegateStrategy2<R,P1,P2>))==0;
		}
	private:
		struct Strategy
		{
			//use placement new for creating strategy objects to avoid heap allocations
			U8 buffer[sizeof(Internal::DelegateStrategy2<R,P1,P2>)];
		}
		m_DelegateStrategy;
	};

	template <typename R, typename P1, typename P2, typename P3>
	class Delegate<R(P1, P2, P3)>
	{
	public:
		Delegate(R (*func)(P1, P2, P3))
		{
			new (&m_DelegateStrategy) Internal::DelegateStrategyStatic3<R,P1,P2,P3>(func);
		}

		template<typename T>
		Delegate(T* pObject, R (T::*func)(P1, P2, P3))
		{
			new (&m_DelegateStrategy) Internal::DelegateStrategyMember3<R,P1,P2>(pObject, func);
		}

		Delegate operator=(const Delegate &rhs)
		{
			m_DelegateStrategy = rhs.m_DelegateStrategy;
			return *this;
		}

		R operator()(P1 p1, P2 p2, P3 p3)
		{
			return (*reinterpret_cast<Internal::DelegateStrategy3<R,P1,P2,P3>*>(&m_DelegateStrategy))(p1,p2,p3);
		}

		bool operator==(const Delegate& rhs)
		{
			return memcmp(this->m_DelegateStrategy.buffer, rhs.m_DelegateStrategy.buffer, sizeof(Internal::DelegateStrategy3<R,P1,P2,P3>))==0;
		}
	private:
		struct Strategy
		{
			//use placement new for creating strategy objects to avoid heap allocations
			U8 buffer[sizeof(Internal::DelegateStrategy3<R,P1,P2,P3>)];
		}
		m_DelegateStrategy;
	};
}

#endif