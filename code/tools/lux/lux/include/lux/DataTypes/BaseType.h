#ifndef __LUX_DATATYPES_BASETYPE_H__
#define __LUX_DATATYPES_BASETYPE_H__

namespace LUX
{

class SurfaceIntegrator;

namespace DataTypes
{

// void context datatype
struct Void
{
	void GetData(U8*&p) { };
	void SetIntegrator(LUX::SurfaceIntegrator *pI) { };
	int	GetSize() { return 0; }
};

class BaseType
{
public:
	typedef Void ContextDataType;
};

// specialize this template to return 'true' for any datatypes that need tangent space calculating
template<typename T>
inline bool NeedTangentSpace()
{
	return false;
}

}

}

#endif
