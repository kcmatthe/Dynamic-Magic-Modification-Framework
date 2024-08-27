#include "Logger.h"

namespace Log
{
	void SetupLog()
	{
		auto logsFolder = SKSE::log::log_directory();
		if (!logsFolder)
			SKSE::stl::report_and_fail("SKSE log_directory not provided, logs disabled.");
		auto pluginName = SKSE::PluginDeclaration::GetSingleton()->GetName();
		auto logFilePath = *logsFolder / std::format("{}.log", pluginName);
		auto fileLoggerPtr = std::make_shared<spdlog::sinks::basic_file_sink_mt>(logFilePath.string(), true);
		auto loggerPtr = std::make_shared<spdlog::logger>("log", std::move(fileLoggerPtr));
		spdlog::set_default_logger(std::move(loggerPtr));
		spdlog::set_level(spdlog::level::info);
		spdlog::flush_on(spdlog::level::trace);
	}

	void UpdateLogLevel(int level)
	{
		//0 = trace
		//1 = debug
		//2 = info
		//3 = warning
		//4 = error
		//5 = critical
		//6 = off
		switch (level) {
		case 0:
			spdlog::set_level(static_cast<spdlog::level::level_enum>(level));
			logger::info("Log level set to trace");
			break;
		case 1:
			spdlog::set_level(static_cast<spdlog::level::level_enum>(level));
			logger::info("Log level set to debug");
			break;
		case 2:
			spdlog::set_level(static_cast<spdlog::level::level_enum>(level));
			logger::info("Log level set to info");
			break;
		case 3:
			spdlog::set_level(static_cast<spdlog::level::level_enum>(level));
			logger::info("Log level set to warning");
			break;
		case 4:
			spdlog::set_level(static_cast<spdlog::level::level_enum>(level));
			logger::info("Log level set to error");
			break;
		case 5:
			spdlog::set_level(static_cast<spdlog::level::level_enum>(level));
			logger::info("Log level set to critical");
			break;
		case 6:
			spdlog::set_level(static_cast<spdlog::level::level_enum>(level));
			logger::info("Log level set to off");
			break;
		default:
			spdlog::set_level(spdlog::level::info);
			logger::info("Log level set to default (info)");
		}
	}
}
