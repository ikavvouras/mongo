/*-
 * Public Domain 2008-2014 WiredTiger, Inc.
 *
 * This is free and unencumbered software released into the public domain.
 *
 * Anyone is free to copy, modify, publish, use, compile, sell, or
 * distribute this software, either in source code form or as a compiled
 * binary, for any purpose, commercial or non-commercial, and by any
 * means.
 *
 * In jurisdictions that recognize copyright laws, the author or authors
 * of this software dedicate any and all copyright interest in the
 * software to the public domain. We make this dedication for the benefit
 * of the public at large and to the detriment of our heirs and
 * successors. We intend this dedication to be an overt act of
 * relinquishment in perpetuity of all present and future rights to this
 * software under copyright law.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#include "test_checkpoint.h"

static void *worker(void *);

typedef struct {
	int id;
	table_type type;			/* Type for threads table. */
	char uri[128];
	WT_SESSION *session;
	WT_CURSOR *cursor;
} COOKIE;

/*
 * r --
 *	Return a 32-bit pseudo-random number.
 *
 * This is an implementation of George Marsaglia's multiply-with-carry pseudo-
 * random number generator.  Computationally fast, with reasonable randomness
 * properties.
 */
static inline uint32_t
r(void)
{
	static uint32_t m_w = 0, m_z = 0;

	if (m_w == 0) {
		struct timeval t;
		(void)gettimeofday(&t, NULL);
		m_w = (uint32_t)t.tv_sec;
		m_z = (uint32_t)t.tv_usec;
	}

	m_z = 36969 * (m_z & 65535) + (m_z >> 16);
	m_w = 18000 * (m_w & 65535) + (m_w >> 16);
	return (m_z << 16) + (m_w & 65535);
}

static void
table_create(COOKIE *cookie)
{
	WT_SESSION *session;
	int ret;
	char *p, *end, config[128];

	session = cookie->session;

	p = config;
	end = config + sizeof(config);
	p += snprintf(p, (size_t)(end - p),
	    "key_format=%s,value_format=S",
	    cookie->type == COL ? "r" : "u");
	if (cookie->type == LSM)
		(void)snprintf(p, (size_t)(end - p), ",type=lsm");

	if ((ret = session->create(session, cookie->uri, config)) != 0)
		if (ret != EEXIST)
			die("session.create", ret);
}

int
start_workers(u_int count, table_type type)
{
	COOKIE *cookies;
	struct timeval start, stop;
	double seconds;
	pthread_t *tids;
	u_int i;
	int ret;
	void *thread_ret;

	/* Create statistics and thread structures. */
	if ((cookies = calloc(
	    (size_t)(count), sizeof(COOKIE))) == NULL ||
	    (tids = calloc((size_t)(count), sizeof(*tids))) == NULL)
		die("calloc", errno);

	(void)gettimeofday(&start, NULL);

	/* Create threads. */
	for (i = 0; i < count; ++i) {
		cookies[i].id = i;
		if (type == MIX)
			cookies[i].type = (i % MAX_TABLE_TYPE) + 1;
		else
			cookies[i].type = type;
		//assert(cookies[i].type == COL ||
		//    cookies[i].type == ROW || cookies[i].type == LSM);
		if ((ret = pthread_create(
		    &tids[i], NULL, worker, &cookies[i])) != 0)
			die("pthread_create", ret);
	}

	/* Wait for the threads. */
	for (i = 0; i < count; ++i)
		(void)pthread_join(tids[i], &thread_ret);

	(void)gettimeofday(&stop, NULL);
	seconds = (stop.tv_sec - start.tv_sec) +
	    (stop.tv_usec - start.tv_usec) * 1e-6;
	printf("Ran workers for: %f seconds\n", seconds);

	free(tids);
	free(cookies);

	return (0);
}

/*
 * worker_op --
 *	Write operation.
 */
static inline void
worker_op(COOKIE *cookie)
{
	WT_CURSOR *cursor;
	WT_ITEM *key, _key, *value, _value;
	u_int keyno;
	int ret;
	char keybuf[64], valuebuf[64];

	cursor = cookie->cursor;
	key = &_key;
	value = &_value;

	keyno = r() % g.nkeys + 1;
	if (cookie->type == COL)
		cursor->set_key(cursor, (uint32_t)keyno);
	else {
		key->data = keybuf;
		key->size = (uint32_t)
		    snprintf(keybuf, sizeof(keybuf), "%017u", keyno);
		cursor->set_key(cursor, key);
	}

	value->data = valuebuf;
	value->size = (uint32_t)snprintf(
	    valuebuf, sizeof(valuebuf), "XXX %37u", keyno);
	cursor->set_value(cursor, value);
	if ((ret = cursor->update(cursor)) != 0)
			die("cursor.update", ret);
}

/*
 * worker --
 *	Worker thread start function.
 */
static void *
worker(void *arg)
{
	COOKIE *cookie;
	WT_CURSOR *cursor;
	WT_SESSION *session;
	pthread_t tid;
	u_int i;
	int ret;

	cookie = (COOKIE *)arg;
	tid = pthread_self();
	printf("worker thread %2d starting: tid: %p\n",
	    cookie->id, (void *)tid);

	snprintf(cookie->uri, 128, "%s%04d", URI_BASE, cookie->id);
	if ((ret = g.conn->open_session(g.conn, NULL, NULL, &session)) != 0)
		die("conn.open_session", ret);
	cookie->session = session;

	/* Create our table */
	table_create(cookie);
	if ((ret = session->open_cursor(
	    session, cookie->uri, NULL, NULL, &cursor)) != 0)
		die("session.open_cursor", ret);
	cookie->cursor = cursor;
	for (i = 0; i < g.nops; ++i, sched_yield())
		worker_op(cookie);
	/*
	 * Work around a bug where close can fail if another thread has
	 * a transaction running.
	 */
	sleep(1);
	if ((ret = session->close(session, NULL)) != 0)
		die("session.close", ret);

	printf("worker thread %2d finishing: tid: %p\n",
	    cookie->id, (void *)tid);

	return (NULL);
}
