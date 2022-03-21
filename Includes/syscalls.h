#pragma once

#include <sys/reent.h>
#include <3ds/types.h>
#include <3ds/result.h>
#include <3ds/svc.h>
#include <3ds/thread.h>

#define THREADVARS_MAGIC  0x21545624 // !TV$

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
	u32 magic;
	Thread thread_ptr;
	struct _reent* reent;
	void* tls_tp;
	u32    fs_magic;
	Handle fs_session;
	bool srv_blocking_policy;
} ThreadVars;

static inline ThreadVars* getThreadVars(void)
{
	return (ThreadVars*)getThreadLocalStorage();
}

void __sync_init();

#ifdef __cplusplus
}
#endif