
namespace Aftk
{
	class C_Scene;
	class C_SceneNode;
}

namespace Cam2
{
class C_AnimData;

void AftkToAnimData(Aftk::C_Scene* scene, const Aftk::C_SceneNode* source, const Aftk::C_SceneNode* instance, C_AnimData* panim);

} // end of namespace
