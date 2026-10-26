
#ifndef CAMERA2_CAMERAFACTORY_H
#define CAMERA2_CAMERAFACTORY_H

namespace Cam2
{

class C_Camera;
typedef C_Camera* (*CreateFn)();

// This is generated for each derived class:
template<typename T> C_Camera * CreateSpecificCamera()
{
	return new T();
}

class C_CameraFactory
{
public:
	C_CameraFactory();

	template<typename T> bool Register(S32 type)
	{
		return RegisterPrivate(type, CreateSpecificCamera<T> );
	}

	C_Camera* Create(S32 type);

private:

	// Class information about a camera
	class Cinfo
	{
	public:
		CreateFn m_createFn; // Create function
	};

	bool RegisterPrivate(S32 type, CreateFn createFn);

	typedef Ut::UtMap<S32, Cinfo> ClassMap;
	ClassMap m_classMap;
	Ut::Vector<Debug::Option> m_options;
};


extern C_CameraFactory g_CameraFactory;

} // End of namespace Cam2

#endif
