/*
 * Copyright 2022 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.openssl.org/source/license.html
 */

#ifndef OSSL_INTERNAL_TIME_H
# define OSSL_INTERNAL_TIME_H
# pragma once

# include <openssl/e_os2.h>     /* uint64_t */
# include "internal/e_os.h"     /* for struct timeval */
# include "internal/safe_math.h"

/*
 * Internal type defining a time.
 * This should be treated as an opaque structure.
 *
 * The time datum is Unix's 1970 and at nanosecond precision, this gives
 * a range of 584 years roughly.
 */
typedef struct {
    uint64_t t;     /* Ticks since the epoch */
} OSSL_TIME;

/* The precision of times allows this many values per second */
# define OSSL_TIME_SECOND ((uint64_t)1000000000)

/* One millisecond. */
# define OSSL_TIME_MS     (OSSL_TIME_SECOND / 1000)

/* One microsecond. */
# define OSSL_TIME_US     (OSSL_TIME_MS     / 1000)

/* Convert a tick count into a time */
static ossl_unused ossl_inline OSSL_TIME ossl_ticks2time(uint64_t ticks)
{
    OSSL_TIME r;

    r.t = ticks;
    return r;
}

/* Convert a time to a tick count */
static ossl_unused ossl_inline uint64_t ossl_time2ticks(OSSL_TIME t)
{
    return t.t;
}

/* Get current time */
OSSL_TIME ossl_time_now(void);

/* The beginning and end of the time range */
static ossl_unused ossl_inline OSSL_TIME ossl_time_zero(void)
{
    return ossl_ticks2time(0);
}

static ossl_unused ossl_inline OSSL_TIME ossl_time_infinite(void)
{
    return ossl_ticks2time(~(uint64_t)0);
}


/* Convert time to timeval */
static ossl_unused ossl_inline
void ossl_time_time_to_timeval(OSSL_TIME t, struct timeval *out)
{
#ifdef _WIN32
    out->tv_sec = (long int)(t.t / OSSL_TIME_SECOND);
#else
    out->tv_sec = (time_t)(t.t / OSSL_TIME_SECOND);
#endif
    out->tv_usec = (t.t % OSSL_TIME_SECOND) / (OSSL_TIME_SECOND / 1000000);
}

/* Compare two time values, return -1 if less, 1 if greater and 0 if equal */
static ossl_unused ossl_inline
int ossl_time_compare(OSSL_TIME a, OSSL_TIME b)
{
    if (a.t > b.t)
        return 1;
    if (a.t < b.t)
        return -1;
    return 0;
}

/*
 * Arithmetic operations on times.
 * These operations are saturating, in that an overflow or underflow returns
 * the largest or smallest value respectively.
 */
OSSL_SAFE_MATH_UNSIGNED(time, uint64_t)

static ossl_unused ossl_inline
OSSL_TIME ossl_time_add(OSSL_TIME a, OSSL_TIME b)
{
    OSSL_TIME r;
    int err = 0;

    r.t = safe_add_time(a.t, b.t, &err);
    return err ? ossl_time_infinite() : r;
}

static ossl_unused ossl_inline
OSSL_TIME ossl_time_subtract(OSSL_TIME a, OSSL_TIME b)
{
    OSSL_TIME r;
    int err = 0;

    r.t = safe_sub_time(a.t, b.t, &err);
    return err ? ossl_time_zero() : r;
}

/* Returns |a - b|. */
static ossl_unused ossl_inline
OSSL_TIME ossl_time_abs_difference(OSSL_TIME a, OSSL_TIME b)
{
    return a.t > b.t ? ossl_time_subtract(a, b)
                     : ossl_time_subtract(b, a);
}

static ossl_unused ossl_inline
OSSL_TIME ossl_time_multiply(OSSL_TIME a, uint64_t b)
{
    OSSL_TIME r;
    int err = 0;

    r.t = safe_mul_time(a.t, b, &err);
    return err ? ossl_time_infinite() : r;
}

static ossl_unused ossl_inline
OSSL_TIME ossl_time_divide(OSSL_TIME a, uint64_t b)
{
    OSSL_TIME r;
    int err = 0;

    r.t = safe_div_time(a.t, b, &err);
    return err ? ossl_time_zero() : r;
}

/* Return higher of the two given time values. */
static ossl_unused ossl_inline
OSSL_TIME ossl_time_max(OSSL_TIME a, OSSL_TIME b)
{
    return a.t > b.t ? a : b;
}

/* Return the lower of the two given time values. */
static ossl_unused ossl_inline
OSSL_TIME ossl_time_min(OSSL_TIME a, OSSL_TIME b)
{
    return a.t < b.t ? a : b;
}

#endif
