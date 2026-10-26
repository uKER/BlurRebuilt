#ifndef LUX_SCENE_UNWRAPPER_H
#define LUX_SCENE_UNWRAPPER_H

namespace LUX
{
	class SceneUnwrapper
	{
	public:
		SceneUnwrapper();
		~SceneUnwrapper();

		bool Unwrap(Aftk::C_Scene* pAftkScene);

	private:
		SceneUnwrapper(const SceneUnwrapper& rhs);
		const SceneUnwrapper& operator=(const SceneUnwrapper& rhs);
	};
}

#endif
