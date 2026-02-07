#include <px4_platform_common/px4_config.h>
#include <px4_platform_common/log.h>
#include <px4_platform_common/posix.h>
#include <px4_platform_common/tasks.h>

#include <uORB/uORB.h>
#include <uORB/topics/fruits.h>

#include <cstring>

extern "C" __EXPORT int module_b_main(int argc, char *argv[]);

static bool thread_should_exit = false;
static bool thread_running = false;
static int daemon_task = -1;

static int module_b_thread_main(int argc, char *argv[]);

static int usage(const char *reason)
{
	if (reason != nullptr) {
		PX4_WARN("%s", reason);
	}

	PX4_INFO("usage: module_b {start|stop|status|help}");
	return 1;
}

int module_b_main(int argc, char *argv[])
{
	if (argc < 2) {
		return usage("missing command");
	}

	if (!std::strcmp(argv[1], "help")) {
		return usage(nullptr);
	}

	if (!std::strcmp(argv[1], "start")) {
		if (thread_running) {
			PX4_INFO("already running");
			return 0;
		}

		thread_should_exit = false;
		daemon_task = px4_task_spawn_cmd("module_b",
					 SCHED_DEFAULT,
					 SCHED_PRIORITY_DEFAULT,
					 1500,
					 module_b_thread_main,
					 (argv) ? (char *const *)&argv[2] : nullptr);

		return 0;
	}

	if (!std::strcmp(argv[1], "stop")) {
		thread_should_exit = true;

		if (!thread_running) {
			PX4_WARN("not running");
		}

		return 0;
	}

	if (!std::strcmp(argv[1], "status")) {
		PX4_INFO(thread_running ? "running" : "not started");
		return 0;
	}

	return usage("unknown command");
}

static int module_b_thread_main(int argc, char *argv[])
{
	thread_running = true;

	int fruits_sub = orb_subscribe(ORB_ID(fruits));
	px4_pollfd_struct_t fds[] = {
		{ .fd = fruits_sub, .events = POLLIN }
	};

	const int expected_count = 10;
	int received_count = 0;

	PX4_INFO("listening for fruits...");

	while (!thread_should_exit && received_count < expected_count) {
		int poll_ret = px4_poll(fds, 1, 1000);

		if (poll_ret < 0) {
			PX4_ERR("poll error: %d", poll_ret);
			break;
		}

		if (poll_ret == 0) {
			continue;
		}

		if (fds[0].revents & POLLIN) {
			fruits_s msg{};
			orb_copy(ORB_ID(fruits), fruits_sub, &msg);
			received_count++;
			PX4_INFO("received fruit: %s (%d/%d)", msg.name, received_count, expected_count);
		}
	}

	if (received_count >= expected_count) {
		PX4_INFO("received all fruits");
	}

	orb_unsubscribe(fruits_sub);
	thread_running = false;
	return 0;
}
