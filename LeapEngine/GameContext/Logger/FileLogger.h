#pragma once
#include "Observer.h"
#include "Debug.h"
#include <string>
#include <fstream>
#include "ILogger.h"

namespace leap
{
	class FileLogger final : TObserver<Debug::LogInfo>, public ILogger
	{
	public:
		FileLogger();
		virtual ~FileLogger();
		FileLogger(const FileLogger& other) = delete;
		FileLogger(FileLogger&& other) = delete;
		FileLogger& operator=(const FileLogger& other) = delete;
		FileLogger& operator=(FileLogger&& other) = delete;

		virtual void SetEnabled(bool enable) override;
		void SetPath(const std::string& path);
	private:
		void StartFile();
		void CloseFile();

		virtual void Notify(const Debug::LogInfo& data) override;
		bool m_Enabled{};
		std::string m_Path{ "log.txt" };
		std::fstream m_File{};
	};
}
