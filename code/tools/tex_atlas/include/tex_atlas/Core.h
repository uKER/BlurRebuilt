#ifndef CS_GEN_CORE_H
#define CS_GEN_CORE_H

namespace TexAtlas
{
	class Database;

	class Core
	{
	public:
		Core();
		~Core();

		Tu::Status Create();

		void Destroy();

		bool IsCreated() const;

		Tu::Status BuildGridCropSheet(
			AftkScene* pAftkScene,
			const std::string& dbFileName,
			const std::string& csFileName,
			U32 csSize,
			U32 borderSize);

		Tu::Status BuildFixedSizeCropSheet(
			AftkScene* pAftkScene,
			const std::string& dbFileName,
			const std::string& csFileName,
			U32 csSize,
			U32 subImageSize,
			U32 borderSize);

		///
		///Crop sheets will be named as csFileName01, csFileName02, csFileName03 etc.
		///
		///@param baseFileName The base file name
		Tu::Status BuildScaleCropSheet(
			AftkScene* pAftkScene,
			const std::string& dbFileName,
			const std::string& csFileName,
			U32 csSize,
			F32 scale,
			U32 borderSize,
			bool allowRotation);

		Tu::Status BuildAutoScaleCropSheet(
			AftkScene* pAftkScene,
			const std::string& dbFileName,
			const std::string& csFileName,
			U32 csSize,
			U32 borderSize,
			bool allowRotation);

		Progress* ProgressCallback();

		void SetProgressCallback(Progress* pProgress);

		void SetPrintMessageCallback(void (*pPrintCallbackFunc)(const std::string& text));

		void SetPrintWarningCallback(void (*pPrintCallbackFunc)(const std::string& text));

		void SetPrintErrorCallback(void (*pPrintCallbackFunc)(const std::string& text));

		void DoError(const std::string& text);

		void DoWarning(const std::string& text);

		void DoMessage(const std::string& text);

	private:
		bool				m_isCreated;
		AftkScene*			m_pAftkScene;

		void (*m_pPrintMessageCallbackFunc)(const std::string& text);
		void (*m_pPrintWarningCallbackFunc)(const std::string& text);
		void (*m_pPrintErrorCallbackFunc)(const std::string& text);
		Progress*			m_pProgressCallback;
	};
};

#endif
