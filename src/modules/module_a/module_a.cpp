#include <px4_platform_common/px4_config.h>
#include <px4_platform_common/log.h>
#include <px4_platform_common/posix.h>

#include <drivers/drv_hrt.h>
#include <uORB/uORB.h>
#include <uORB/topics/fruits.h>

#include <cstring>

extern "C" __EXPORT int module_a_main(int argc, char *argv[]);

static int usage(const char *reason)
{
	if (reason != nullptr) {
		PX4_WARN("%s", reason);
	}

	PX4_INFO("usage: module_a {start|help}");
	return 1;
}

int module_a_main(int argc, char *argv[])
{
	if (argc < 2) {
		return usage("missing command");
	}

	if (!std::strcmp(argv[1], "help")) {
		return usage(nullptr);
	}

	if (std::strcmp(argv[1], "start") != 0) {
		return usage("unknown command");
	}

	static const char *fruits[] = {
		"apple",
		"banana",
		"cherry",
		"grape",
		"kiwi",
		"mango",
		"orange",
		"pear",
		"pineapple",
		"strawberry"
	};

	fruits_s msg{};
	orb_advert_t pub = nullptr;
	const size_t fruit_count = sizeof(fruits) / sizeof(fruits[0]);

	PX4_INFO("publishing %zu fruits", fruit_count);

	for (size_t i = 0; i < fruit_count; i++) {
		msg.timestamp = hrt_absolute_time();
		std::strncpy(msg.name, fruits[i], sizeof(msg.name));
		msg.name[sizeof(msg.name) - 1] = '\0';

		if (pub == nullptr) {
			pub = orb_advertise(ORB_ID(fruits), &msg);

			if (pub == nullptr) {
				PX4_ERR("failed to advertise fruits topic");
				return 1;
			}

		} else {
			orb_publish(ORB_ID(fruits), pub, &msg);
		}

		PX4_INFO("published fruit: %s", msg.name);
		px4_usleep(200000);
	}

	return 0;
}
