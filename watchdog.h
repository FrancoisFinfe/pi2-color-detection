#ifndef __WATCHDOG_H

#define __WATCHDOG_H

#include "time.h"


#ifdef __cplusplus
extern "C" {
#endif

typedef enum watchdog_status_e {
	NOT_INIT = 0,		/* not yet initialised or init failed */
	STARTED = 1,		/* must be clear periodically */
	DISABLED = 2		/* disabled, no need to kill it */
} watchdog_status_t;
	

typedef struct watchdog_ctx_s {
	int fd;			/* File handler for watchdog */
	int period;	/* Watchdog timeout interval (in secs) */
	time_t lastclear;
	int clear_period;	/* min time between watchdog clear */
	watchdog_status_t status;
	char watchdog_dev[64];
} watchdog_ctx_t;



int watchdog_init(watchdog_ctx_t *ctx, char *watchdog_dev, int period);
int watchdog_close(watchdog_ctx_t *ctx);
int watchdog_clear(watchdog_ctx_t *ctx);
int watchdog_clear_if_timeout(watchdog_ctx_t *ctx);

#ifdef __cplusplus
}
#endif

#endif
