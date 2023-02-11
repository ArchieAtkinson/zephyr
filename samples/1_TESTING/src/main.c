/*
 * Copyright (c) 2012-2014 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "zephyr/sys/printk.h"
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include "posix_board_if.h"

LOG_MODULE_REGISTER(main);


// #define K_FUTURE_DEFINE(name, )

struct k_future;
typedef void (*future_callback_handler_t)(struct k_future *future);

struct k_future {
	struct k_work work;
	struct k_sem sem;
	struct k_poll_event poll;
	future_callback_handler_t handler; 
};

struct my_future {
	struct k_future future;
	int a;
	int b;
	int out;
};


void add(struct k_future *obj)
{
	struct my_future *future = CONTAINER_OF(obj, struct my_future, future);
	future->out = future->a + future->b;
}

void future_work_handler(struct k_work *item)
{
	struct k_future *future = CONTAINER_OF(item, struct k_future, work);
	future->handler(future);
	k_sem_give(&future->sem);
}

int k_future_init(struct k_future *future, future_callback_handler_t handler)
{
	int ret = k_sem_init(&future->sem, 1, 1);
	if (ret != 0) {
        return ret;
	}
	k_work_init(&future->work, future_work_handler);

	future->handler = handler;

	k_poll_event_init(&future->poll,
                      K_POLL_TYPE_SEM_AVAILABLE,
                      K_POLL_MODE_NOTIFY_ONLY,
                      &future->sem);
	return ret;
}

int k_future_exec(struct k_future *future, k_timeout_t timeout)
{
	int ret = k_sem_take(&future->sem, timeout);
	if (ret != 0) {
        LOG_ERR("Sem Error - %d", ret);
        return ret;
	}
	
	return k_work_submit(&future->work);
}

int k_future_get(struct k_future *future,  k_timeout_t timeout)
{
	return k_poll(&future->poll, 1, timeout);
}

// outputs and inputs

void main(void)
{
    LOG_INF("Hello World! %s", CONFIG_BOARD);
    struct my_future a_future;
    a_future.a = 10;
    a_future.b = 5;

    int ret = k_future_init(&a_future.future, add);
    if (ret != 0) {
        LOG_ERR("Init Fail - %d", ret);
	}

    ret = k_future_exec(&a_future.future, K_NO_WAIT);
    if (ret < 0) {
        LOG_ERR("Exec Fail - %d", ret);
	}
    LOG_INF("waiting");
    ret = k_future_get(&a_future.future, K_FOREVER);
    if (ret != 0) {
        LOG_ERR("Get Fail - %d", ret);
	}
    LOG_INF("Output %d", a_future.out);

    a_future.a = 5;
    a_future.b = 6;

    ret = k_future_exec(&a_future.future, K_NO_WAIT);
    if (ret < 0) {
        LOG_ERR("Exec Fail - %d", ret);
	}
    LOG_INF("waiting");
    ret = k_future_get(&a_future.future, K_FOREVER);
    if (ret != 0) {
        LOG_ERR("Get Fail - %d", ret);
	}
    LOG_INF("Output %d", a_future.out);

    posix_exit(0);
}
