/*
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * Copyright 2011-2012 Mathieu Desnoyers <mathieu.desnoyers@efficios.com>
 *
 * Userspace RCU - sys_futex/compat_futex header.
 */

#ifndef _LTTNG_UST_FUTEX_H
#define _LTTNG_UST_FUTEX_H

#include <errno.h>
#include <stdint.h>
#include <time.h>
#include <sys/syscall.h>

#include "ust-helper.h"

#ifdef __cplusplus
extern "C" {
#endif

#define FUTEX_WAIT		0
#define FUTEX_WAKE		1

/*
 * sys_futex compatibility header.
 * Use *only* *either of* futex_noasync OR futex_async on a given address.
 *
 * futex_noasync cannot be executed in signal handlers, but ensures that
 * it will be put in a wait queue even in compatibility mode.
 *
 * futex_async is signal-handler safe for the wakeup. It uses polling
 * on the wait-side in compatibility mode.
 *
 * BEWARE: sys_futex() FUTEX_WAIT may return early if interrupted
 * (returns EINTR).
 */

LTTNG_HIDDEN
extern int lttng_ust_compat_futex_noasync(int32_t *uaddr, int op, int32_t val,
		const struct timespec *timeout, int32_t *uaddr2, int32_t val3);
LTTNG_HIDDEN
extern int lttng_ust_compat_futex_async(int32_t *uaddr, int op, int32_t val,
		const struct timespec *timeout, int32_t *uaddr2, int32_t val3);

#if (defined(__linux__) && defined(__NR_futex))

#include <unistd.h>
#include <errno.h>
#include <urcu/compiler.h>
#include <urcu/arch.h>

static inline int lttng_ust_futex(int32_t *uaddr, int op, int32_t val,
		const struct timespec *timeout, int32_t *uaddr2, int32_t val3)
{
	return syscall(__NR_futex, uaddr, op, val, timeout,
			uaddr2, val3);
}

static inline int lttng_ust_futex_noasync(int32_t *uaddr, int op, int32_t val,
		const struct timespec *timeout, int32_t *uaddr2, int32_t val3)
{
	int ret;

	ret = lttng_ust_futex(uaddr, op, val, timeout, uaddr2, val3);
	if (caa_unlikely(ret < 0 && errno == ENOSYS)) {
		/*
		 * The fallback on ENOSYS is the async-safe version of
		 * the compat futex implementation, because the
		 * async-safe compat implementation allows being used
		 * concurrently with calls to futex(). Indeed, sys_futex
		 * FUTEX_WAIT, on some architectures (mips and parisc),
		 * within a given process, spuriously return ENOSYS due
		 * to signal restart bugs on some kernel versions.
		 */
		return lttng_ust_compat_futex_async(uaddr, op, val, timeout,
				uaddr2, val3);
	}
	return ret;

}

static inline int lttng_ust_futex_async(int32_t *uaddr, int op, int32_t val,
		const struct timespec *timeout, int32_t *uaddr2, int32_t val3)
{
	int ret;

	ret = lttng_ust_futex(uaddr, op, val, timeout, uaddr2, val3);
	if (caa_unlikely(ret < 0 && errno == ENOSYS)) {
		return lttng_ust_compat_futex_async(uaddr, op, val, timeout,
				uaddr2, val3);
	}
	return ret;
}

#elif defined(__FreeBSD__)

#include <sys/types.h>
#include <sys/umtx.h>

static inline int lttng_ust_futex_async(int32_t *uaddr, int op, int32_t val,
		const struct timespec *timeout, int32_t *uaddr2, int32_t val3)
{
	int umtx_op;
	void *umtx_uaddr = NULL, *umtx_uaddr2 = NULL;
	struct _umtx_time umtx_timeout = {
		._flags = UMTX_ABSTIME,
		._clockid = CLOCK_MONOTONIC,
	};

	switch (op) {
	case FUTEX_WAIT:
		/* On FreeBSD, a "u_int" is a 32-bit integer. */
		umtx_op = UMTX_OP_WAIT_UINT;
		if (timeout != NULL) {
			umtx_timeout._timeout = *timeout;
			umtx_uaddr = (void *) sizeof(umtx_timeout);
			umtx_uaddr2 = (void *) &umtx_timeout;
		}
		break;
	case FUTEX_WAKE:
		umtx_op = UMTX_OP_WAKE;
		break;
	default:
		errno = EINVAL;
		return -1;
	}

	return _umtx_op(uaddr, umtx_op, (uint32_t) val, umtx_uaddr,
			umtx_uaddr2);
}

static inline int lttng_ust_futex_noasync(int32_t *uaddr, int op, int32_t val,
		const struct timespec *timeout, int32_t *uaddr2, int32_t val3)
{
	return lttng_ust_futex_async(uaddr, op, val, timeout, uaddr2, val3);
}

#elif defined(__CYGWIN__)

/*
 * The futex_noasync compat code uses a weak symbol to share state across
 * different shared object which is not possible on Windows with the
 * Portable Executable format. Use the async compat code for both cases.
 */
static inline int lttng_ust_futex_noasync(int32_t *uaddr, int op, int32_t val,
		const struct timespec *timeout, int32_t *uaddr2, int32_t val3)
{
	return lttng_ust_compat_futex_async(uaddr, op, val, timeout, uaddr2, val3);
}

static inline int lttng_ust_futex_async(int32_t *uaddr, int op, int32_t val,
		const struct timespec *timeout, int32_t *uaddr2, int32_t val3)
{
	return lttng_ust_compat_futex_async(uaddr, op, val, timeout, uaddr2, val3);
}

#else

static inline int lttng_ust_futex_noasync(int32_t *uaddr, int op, int32_t val,
		const struct timespec *timeout, int32_t *uaddr2, int32_t val3)
{
	return lttng_ust_compat_futex_noasync(uaddr, op, val, timeout, uaddr2, val3);
}

static inline int lttng_ust_futex_async(int32_t *uaddr, int op, int32_t val,
		const struct timespec *timeout, int32_t *uaddr2, int32_t val3)
{
	return lttng_ust_compat_futex_async(uaddr, op, val, timeout, uaddr2, val3);
}

#endif

#ifdef __cplusplus
}
#endif

#endif /* _LTTNG_UST_FUTEX_H */
