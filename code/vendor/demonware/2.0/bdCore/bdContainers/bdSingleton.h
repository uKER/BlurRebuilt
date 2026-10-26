// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE: This class implements the singleton design pattern.

#ifndef BD_SINGLETON_H
#define BD_SINGLETON_H

#include <bdCore/bdMemory/bdMemory.h>

/// This class implements the singleton design pattern.
/// Singletons are classes that have only one instance, and a global point of
/// access.
/// To use \c bdSingleton, declare an empty class to inherit from
/// \c bdSingleton<T>, where \c T is the name of the class you want to access
/// as a singleton.
/// You must also use the \c BD_REGISTER_SINGLETON macro to register \c T. Your
/// singleton can now be accessed via the empty class that you declared.
/// Since all memory allocations must go through bdMemory, \c bdSingleton 's
/// cannot be constructed "pre-main". Instead lazy construction is used, and
/// \c bdSingleton instances are constructed on the first call to their
/// respective getInstance() methods. This means that a \c bdSingleton cannot
/// be accessed until bdCore::init() has been called.
/// To ensure that only the singleton instance of \c T is created you can
/// prevent users from creating their own instances of \c T. To do this make
/// default constructor of \c T and the default constructor of the blank class
/// protected. In doing so you must also make \c bdSingleton<T> a friend of
/// \c T so it has access to this constructor.
/// See bdError for a complete example.
template <typename T>
class bdSingleton
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		/// The global access point to the \c bdSingleton.
		inline static T* BD_CALL getInstance();

		/// Replace the single instance with an instance of a derived class.
		/// This allows the functionality of a singleton to be specialized by
		/// higher levels of the architecture.
		/// \param	newInstance[in]	A pointer to the instance of a \c T or a
		///							class derived from \c T to use as the new
		///							single instance.
		inline void replaceInstance(const T *const newInstance);

		/// Destroy the single instance.
		/// This method is called from bdSingletonRegistry::cleanUp(). It
		/// simply calls delete on \c m_instance.
		inline static void BD_CALL destroyInstance();

	protected:

		/// Default constructor.
		/// It is protected so that this class cannot be created
		/// directly.
		inline bdSingleton();

	protected:

		/// A pointer to the single class instance.
		static T *m_instance;

};

/// This macro should be used to initialize the static class pointer to the
/// single instance of a singleton. See bdError.cpp for an example.
#define BD_REGISTER_SINGLETON(CLASS) \
	template<> CLASS * bdSingleton< CLASS >::m_instance = BD_NULL;

#include <bdPlatform/bdPlatformString/bdPlatformString.h>
#include <bdCore/bdContainers/bdFastArray.h>

/// A registry of all singleton instances. We use lazy singleton creation
/// so on the first call to mySingleton::getInstance() an instance is created
/// and a function capable of destroying it is registered in the
/// bdSingletonRegistry. This is done so that we can release the memory
/// associated with all bdSingleton 's when bdCore::quit() is called.
/// bdCore::quit() calls bdSingletonRegistryImpl::cleanUp(), which
/// destroys all registered bdSingleton 's in the opposite order to their
/// creation.
/// \note bdSingletonRegistry is itself a singleton.
class bdSingletonRegistryImpl
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		/// Defines a function pointer to a function that will destroy
		/// a bdSingleton instance.
		typedef void (BD_CALL *bdSingletonDestroyFunction)();

		/// Add a bdSingleton 's destroy function into the registry.
		/// \param	destroyFunction[in]	A pointer to a function to destroy
		///								an instance of a singleton.
		bdBool add(const bdSingletonDestroyFunction destroyFunction)
		{
			const bdBool added = !m_cleaningUp;
			if(added)
			{
				m_destroyFunctions.pushBack(destroyFunction);
			}
#ifdef BD_DEBUG
			else
			{
				// bdLog seems to use singletons, so leave this printf here?
				bdFprintf(BD_STDERR, "Error: %s (%u)\n"
					"bdSingletonRegistryImpl::add(), cannot register bdSingletons "
					"while in bdSingletonRegistryImpl::cleanUp.\n", __FILE__, __LINE__);
			}
#endif // BD_DEBUG
			return added;
		}

		/// Calls all the registered \c bdSingleton destroy functions in the
		/// reverse order to which they where added.
		void cleanUp()
		{
			const bdSingletonDestroyFunction* begin = m_destroyFunctions.begin();
			bdSingletonDestroyFunction* end = m_destroyFunctions.end();
			m_cleaningUp = true;

			while (end != begin)
			{
				end--;
				(*end)();
			}

			// Can't do anything here as the bdSingletonRegistryImpl will be deleted
			// by the last call to (*end)() above, ie "this" becomes invalid.
			// m_cleaningUp = false;
		}

	protected:

		/// Grants friendship to bdSingleton<bdSingletonRegistryImpl> so that
		/// it can access the protected default constructor.
		friend class bdSingleton<bdSingletonRegistryImpl>;

		/// Default constructor.
		bdSingletonRegistryImpl()
		: m_cleaningUp(false)
		{
		}

		/// Destructor.
		virtual ~bdSingletonRegistryImpl()
		{
		}

	protected:

		/// Storage of the bdSingleton destroy function pointers.
		bdFastArray<bdSingletonDestroyFunction> m_destroyFunctions;

		/// Indicates if we are inside a call to cleanUp().
		bdBool m_cleaningUp;

};

/// This class provides a point of access to the singleton instance.
/// Implementation details can be found in bdSingletonRegistryImpl.
class bdSingletonRegistry : public bdSingleton<bdSingletonRegistryImpl>
{
	BD_DECLARE_NEW_AND_DELETE_OPERATORS

	protected:
		/// Default constructor
		bdSingletonRegistry();
};
#include <bdCore/bdContainers/bdSingleton.inl>

#endif // BD_SINGLETON_H
