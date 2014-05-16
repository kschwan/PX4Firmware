/****************************************************************************
 *
 *   Copyright (c) 2012, 2013 PX4 Development Team. All rights reserved.
 *   Author: Kim Lindberg Schwaner <kschw10@student.sdu.dk>
 *           Thomas Larsen <thola11@student.sdu.dk>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name PX4 nor the names of its contributors may be
 *    used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 ****************************************************************************/

/**
 * @file sr_tester_main.cpp
 *
 * @author Kim Lindberg Schwaner <kschw10@student.sdu.dk>
 * @author Thomas Larsen <thola11@student.sdu.dk>
 */

#include <cxx/cstdlib>
#include <cxx/cstdio>
#include <cxx/cstring>
#include <systemlib/systemlib.h>
#include <systemlib/getopt_long.h>
#include <systemlib/err.h>
#include <drivers/drv_hrt.h>
#include <uORB/uORB.h>
#include <uORB/topics/actuator_controls.h>
#include <uORB/topics/vehicle_status.h>

extern "C" __EXPORT int sr_tester_main(int argc, char *argv[]);

static void hwtest()
{
	struct actuator_controls_s actuators;
	std::memset(&actuators, 0, sizeof(actuators));

	orb_advert_t actuator_pub_fd = orb_advertise(ORB_ID(actuator_controls_0), &actuators);

	int i;
	int count = 0;
	float rcvalue = -1.0f;
	hrt_abstime stime;

	while (count < 3) {
		stime = hrt_absolute_time();

		while (hrt_absolute_time() - stime < 1000000) {
			for (i = 0; i < 8; i++)
				actuators.control[i] = rcvalue;

			actuators.timestamp = hrt_absolute_time();
			orb_publish(ORB_ID(actuator_controls_0), actuator_pub_fd, &actuators);
		}

		warnx("servos set to %.1f", rcvalue);
		rcvalue *= -1.0f;
		count++;
	}
}

static void vstat()
{
	struct vehicle_status_s status;
	int status_handle = orb_subscribe(ORB_ID(vehicle_status));
	orb_copy(ORB_ID(vehicle_status), status_handle, &status);

	printf("condition_system_sensors_initialized: %s\n", status.condition_system_sensors_initialized ? "true" : "false");
	printf("condition_global_position_valid: %s\n", status.condition_global_position_valid ? "true" : "false");
	printf("condition_home_position_valid: %s\n", status.condition_home_position_valid ? "true" : "false");
	printf("condition_local_position_valid: %s\n", status.condition_local_position_valid ? "true" : "false");
	printf("condition_local_altitude_valid: %s\n", status.condition_local_altitude_valid ? "true" : "false");
	printf("condition_landed: %s\n", status.condition_landed ? "true" : "false");

	orb_unsubscribe(status_handle);
}

static void usage()
{
	printf("Usage: sr_tester [options]\n\n");
	printf("  -h, --help\tthis help\n");
	printf("  -p, --vstat\tvehicle status\n");
	printf("      --hwtest\tsimilar to the built-in hw_test example\n");
}

int sr_tester_main(int argc, char *argv[])
{
	int opt;
	int opt_idx = 0;
	bool continue_parse = true;

	static GETOPT_LONG_OPTION_T options[] = {
		{"help", NO_ARG, NULL, 'h'},
		{"vstat", NO_ARG, NULL, 'v'},
		{"hwtest", NO_ARG, NULL, 'a'},
		{NULL, NULL, NULL, NULL}
	};

	optind = 0; // Reset optind

	while (continue_parse) {
		opt = getopt_long(argc, argv, "hv", options, &opt_idx);

		if (opt == EOF) {
			break;
		}

		switch (opt) {
		case 'v':
			vstat();
			continue_parse = false;
			break;

		case 'a':
			hwtest();
			continue_parse = false;
			break;

		case 'h':
		case '?':
		default:
			usage();
			continue_parse = false;
			break;
		}
	}

	return EXIT_SUCCESS;
}
