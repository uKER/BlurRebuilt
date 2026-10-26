
namespace Aftk
{
	class C_Scene;
	class C_SceneNode;
}

namespace Cam2
{
class C_CameraScene;

void AftkToCameraScene(Aftk::C_Scene* scene, Aftk::C_SceneNode* topNode, C_CameraScene& outScene);

} // end of namespace
