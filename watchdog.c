#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "watchdog.h"
#include <fcntl.h>
#include <errno.h>
#include <linux/watchdog.h>
#include <sys/ioctl.h>

#define WATCHDOGDEV "/dev/watchdog"

int watchdog_init(watchdog_ctx_t *ctx, char *watchdog_dev, int period) {
	int res = 0;
	int interval;
	int bootstatus;
	/* Clear the struct, fill it with 0 */
	memset(ctx, 0, sizeof(watchdog_ctx_t));

	/* initialise context structure */
	ctx->status = NOT_INIT;

	/*if watchdog_dev specified, use this define instead of the default value */
	strncpy(ctx->watchdog_dev, watchdog_dev ? watchdog_dev : WATCHDOGDEV, sizeof(ctx->watchdog_dev));

	if (period)
		ctx->period = period;

	/* Once the watchdog device file is open, the watchdog will be activated by the driver */
	ctx->fd = open(ctx->watchdog_dev, O_RDWR);

	if (ctx->fd == -1) {
		printf("Watchdog: init failed, can't open \"%s\"(%s)\n", ctx->watchdog_dev, strerror(errno));
		return -1;
	}

	/* Display current watchdog interval */
	if (!ioctl(ctx->fd, WDIOC_GETTIMEOUT, &interval)) {
		printf("Current watchdog interval is %d\n", interval);
	} else {
		printf("Watchdog: cannot read watchdog interval\n");
		return -1;
	}

	/* If user wants to change the watchdog interval */
	if (ctx->period) {
		printf("Set watchdog interval to %d\n", ctx->period);
		if (ioctl(ctx->fd, WDIOC_SETTIMEOUT, &ctx->period)) {
			printf("Watchdog: set watchdog interval failed\n");
			return -1;
		} 
	} else {
		ctx->period = interval;
	}

	/* Verify current watchdog interval */
	if (!ioctl(ctx->fd, WDIOC_GETTIMEOUT, &ctx->period)) {
		printf("Current watchdog interval is %d\n", ctx->period);
	} else {
		printf("Watchdog: cannot read watchdog interval\n");
		return -1;
	}
	
	/* set max period between watchdog clear */
	ctx->clear_period = ctx->period / 2;

	if (ctx->clear_period > 1)
		ctx->clear_period = 1;

 	/* Check if last boot is caused by watchdog */
	if (!ioctl(ctx->fd, WDIOC_GETBOOTSTATUS, &bootstatus)) {
		if (bootstatus)
			printf("Watchdog: last boot is caused by watchdog !!!\n");
	} else {
		fprintf(stdout, "Watchdog: cannot read watchdog status\n");
		/* we can continue, no need to return */
	}

	ctx->status = STARTED;
	ctx->lastclear = time(NULL);

 
	return res;
}



int watchdog_close(watchdog_ctx_t *ctx) {

	if (ctx->status != STARTED)
		return -1;

	/* Magic Close */
	write(ctx->fd, "V", 1);
	printf("Watchdog: closed\n");
	close(ctx->fd);
	ctx->status = DISABLED;

	return 0;
}

int watchdog_clear(watchdog_ctx_t *ctx) {
	if (ctx->status != STARTED)
		return -1;

	ioctl(ctx->fd, WDIOC_KEEPALIVE, NULL);
	ctx->lastclear = time(NULL);

	return 0;
}


int watchdog_clear_if_timeout(watchdog_ctx_t *ctx) {
	int elapsed = time(NULL) - ctx->lastclear;

	if (ctx->status != STARTED)
		return -1;

	if (elapsed > ctx->clear_period) {
		watchdog_clear(ctx);
		return 1;
	}

	return 0;
}
