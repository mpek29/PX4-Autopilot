#include <px4_platform_common/px4_config.h>
#include <px4_platform_common/log.h>

#include <cstring>

extern "C" __EXPORT int my_test_app_main(int argc, char *argv[]);

static int usage(const char *reason)
{
	if (reason != nullptr) {
		PX4_WARN("%s", reason);
	}

	PX4_INFO("usage: my_test_app {start|status|help}");
	return 1;
}

int my_test_app_main(int argc, char *argv[])
{
	if (argc < 2) {
		return usage("missing command");
	}

	if (!std::strcmp(argv[1], "help")) {
		return usage(nullptr);
	}

	if (!std::strcmp(argv[1], "status")) {
		PX4_INFO("my_test_app is a simple one-shot app");
		return 0;
	}

	if (!std::strcmp(argv[1], "start")) {
		PX4_INFO("Hello from your computer! Variable x = %d", 42);
		return 0;
	}

	return usage("unknown command");
}
