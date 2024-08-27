#pragma once

#include <spdlog/sinks/basic_file_sink.h>

namespace logger = SKSE::log;

namespace Log
{
	void SetupLog();

	void UpdateLogLevel(int level);
}

	



