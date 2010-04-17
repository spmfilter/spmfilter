/* spmfilter - mail filtering framework
 * Copyright (C) 2009-2010 Axel Steiner and SpaceNet AG
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/shm.h>
#include <sys/file.h>
#include <errno.h>

#include "smf_daemon.h"
#include "smf_daemon_scoreboard.h"
#include "smf_trace.h"

static volatile SMFScoreBoard_T *scoreboard;
static int shmid;
static int sb_lockfd;
static int set_lock(int type);

int set_lock(int type) {
	int result, serr;
	struct flock lock;
	static int retry = 0;
	lock.l_type = type; /* F_RDLCK, F_WRLCK, F_UNLOCK */
	lock.l_start = 0;
	lock.l_whence = 0;
	lock.l_len = 1;
	result = fcntl(sb_lockfd, F_SETLK, &lock);
	if (result == -1) {
		serr = errno;
		switch (serr) {
			case EACCES:
			case EAGAIN:
			case EDEADLK:
				if (retry++ > 2)
					TRACE(TRACE_WARNING, "Error setting lock. Still trying...");
				usleep(10);
				set_lock(type);
				break;
			default:
				// ignore the rest
				retry = 0;
				break;
		}
		errno = serr;
	} else {
		retry = 0;
	}
	return result;
}

char *smf_daemon_scoreboard_lock_getfilename(void) {
	return g_strdup_printf("%s_%d.LCK", SCOREBOARD_LOCK_FILE, getpid());
}

void smf_daemon_scoreboard_delete(void) {
	gchar *statefile;
	extern int isGrandChildProcess;

	/* The middle process removes the scoreboards, so only bail out
	 * if we are a grandchild / connection handler process. */
	if (isGrandChildProcess)
		return;

	if (shmdt((const void *)scoreboard) == -1)
		TRACE(TRACE_ERR, "detach shared mem failed");
	if (shmctl(shmid, IPC_RMID, NULL) == -1)
		TRACE(TRACE_ERR, "delete shared mem segment failed");

	statefile = smf_daemon_scoreboard_lock_getfilename();
	if (unlink(statefile) == -1)
		TRACE(TRACE_ERR, "error deleting scoreboard lock file [%s]",
		      statefile);
	g_free(statefile);

	return;
}

void smf_daemon_scoreboard_lock_new(void) {
	gchar *statefile = smf_daemon_scoreboard_lock_getfilename();
	if ((sb_lockfd = open(statefile,O_EXCL|O_RDWR|O_CREAT|O_TRUNC,0600)) < 0) {
		TRACE(TRACE_FATAL, "Could not open lockfile [%s]", statefile);
	}
	g_free(statefile);
}

void smf_daemon_scoreboard_setup(void) {
	int i;
	scoreboard_wrlck();
	for (i = 0; i < HARD_MAX_CHILDREN; i++)
		state_reset((child_state_t *)&scoreboard->child[i]);
	scoreboard_unlck();
}


void smf_daemon_scoreboard_conf_check(void) {
	/* some sanity checks on boundaries */
	scoreboard_wrlck();
	if (scoreboard->config->max_children > HARD_MAX_CHILDREN) {
		TRACE(TRACE_WARNING, "MAXCHILDREN too large. Decreasing to [%d]",
		      HARD_MAX_CHILDREN);
		scoreboard->config->max_children = HARD_MAX_CHILDREN;
	} else if (scoreboard->config->max_children < scoreboard->config->start_children) {
		TRACE(TRACE_WARNING, "MAXCHILDREN too small. Increasing to NCHILDREN [%d]",
		      scoreboard->config->start_children);
		scoreboard->config->max_children = scoreboard->confg->start_children;
	}

	if (scoreboard->config->max_spare_children > scoreboard->config->max_children) {
		TRACE(TRACE_WARNING, "MAXSPARECHILDREN too large. Decreasing to MAXCHILDREN [%d]",
		      scoreboard->config->max_children);
		scoreboard->config->max_spare_children = scoreboard->config->max_children;
	} else if (scoreboard->config->max_spare_children < scoreboard->config->min_spare_children) {
		TRACE(TRACE_WARNING, "MAXSPARECHILDREN too small. Increasing to MINSPARECHILDREN [%d]",
		      scoreboard->config->min_spare_children);
		scoreboard->config->max_spare_children = scoreboard->config->min_spare_children;
	}
	scoreboard_unlck();
}

void smf_daemon_scoreboard_new(SMFDaemonConfig_T *config) {
	int serr;
	if ((shmid = shmget(IPC_PRIVATE, sizeof(SMFScoreBoard_T), 0644 | IPC_CREAT)) == -1) {
		serr = errno;
		TRACE(TRACE_FATAL, "shmget failed [%s]", strerror(serr));
	}
	scoreboard = shmat(shmid, (void *) 0, 0);
	serr=errno;
	if (scoreboard == (SMFScoreBoard_T *) (-1)) {
		TRACE(TRACE_FATAL, "scoreboard init failed [%s]",
		      strerror(serr));
		smf_daemon_scoreboard_delete();
	}
	smf_daemon_scoreboard_lock_new();
	scoreboard->config = config;
	smf_daemon_scoreboard_setup();
	smf_daemon_scoreboard_conf_check();

	/* Make sure that we clean up our shared memory segments when we exit
	 * normally (i.e. not by kill -9, if you do that, you get to clean this
	 * up yourself!)
	 * */
	atexit(smf_daemon_scoreboard_delete);
}


