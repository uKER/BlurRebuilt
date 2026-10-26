
#ifndef CAMERA2_FEEDLIST_H
#define CAMERA2_FEEDLIST_H

namespace Cam2
{

class I_Feed;
class C_CamObj;
class C_FeedList
{
private:
	Ut::Vector<I_Feed *> m_list;

public:
	void Add(I_Feed* cam);
	void Remove(I_Feed* cam);
	void Clear();
	void SetObject(C_CamObj& obj);
	void Update(F32 period);
	int Size();
	I_Feed* operator[] (int i);
	void DeleteAll();
};

} // End of namespace

#endif
