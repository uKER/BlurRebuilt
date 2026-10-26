#ifndef __GAME_RESOURCE_MODEL_TYPE_MANAGER_H__
#define __GAME_RESOURCE_MODEL_TYPE_MANAGER_H__

namespace GameResource
{

class C_ModelResource;

/// modeltype
class I_ModelType
{
public:
	virtual ~I_ModelType() {}

	virtual void OnLoad(C_ModelResource *pModel) = 0;
};

/// register different model types which allows a model type spefic callback function to be run
/// these callback function will be used to expose particular parameters for that model type
class ModelTypeManager
{
public:
	/// destructor
	~ModelTypeManager();

	/// register a new model type
	/// ownership of the pType pointer passes to the ModelTypeManager class
	void RegisterModelType(const char *pszName, I_ModelType *pType);

	/// run the callback for the specified modeltype
	void OnLoad(const char *pszType, C_ModelResource *pModel);

private:
	/// return the index for the modeltype with the given name
	int GetModelTypeIdx(const char *pszType);

	struct Item
	{
		Util::C_String sName;
		I_ModelType *pType;
	};

	Btl::Vector<Item> m_Types;
};

extern ModelTypeManager g_ModelTypeManager;

} // namespace WorldRender

#endif // WORLD_RENDER_MODEL_RENDER_H
