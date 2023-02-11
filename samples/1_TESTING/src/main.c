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


#define K_FUTURE_INIT(_name)                                                                       \
	int k_future_init_##_name(struct k_future_data_##_name *future_##_name, future_callback_handler_t handler) \
	{                                                                                          \
        int ret = k_sem_init(&future_##_name->future.sem, 1, 1); \
        if (ret != 0) { \
            return ret; \
        } \
        k_work_init(&future_##_name->future.work, future_work_handler); \
    \
        future_##_name->future.handler = handler; \
    \
        k_poll_event_init(&future_##_name->future.poll, \
                        K_POLL_TYPE_SEM_AVAILABLE, \
                        K_POLL_MODE_NOTIFY_ONLY, \
                        &future_##_name->future.sem);\
        return ret; \
    }


#define K_FUTURE_EXEC(_name, _input_type) \
    int k_future_exec_##_name(struct k_future_data_##_name *future_##_name, _input_type in,  k_timeout_t timeout) \
    { \
        future_##_name->input = in; \
        int ret = k_sem_take(&future_##_name->future.sem, timeout); \
        if (ret != 0) { \
            LOG_ERR("Sem Error - %d", ret); \
            return ret; \
        } \
        return k_work_submit(&future_##_name->future.work); \
    } \

#define K_FUTURE_GET(_name, _output_type)                                                          \
    _output_type k_future_get_##_name(struct k_future_data_##_name *future_##_name, k_timeout_t timeout)            \
    {                                                                                          \
        k_poll(&future_##_name->future.poll, 1, timeout); \
        return future_##_name->output; \
    }



#define K_FUTURE_DEFINE(_name, _input_type, _output_type)                                          \
	struct k_future_data_##_name {                                                             \
		struct k_future future;                                                            \
		_input_type input;                                                                 \
		_output_type output;                                                               \
	};                                                                                         \
	K_FUTURE_INIT(_name)                                                                       \
	K_FUTURE_EXEC(_name, _input_type)                                                          \
    K_FUTURE_GET(_name, _output_type)  \

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

struct add_inputs {
    int a;
    int b;
};

K_FUTURE_DEFINE(adding, struct add_inputs, int);


// outputs and inputs

void main(void)
{
	LOG_INF("Hello World! %s", CONFIG_BOARD);
    struct k_future_data_adding my_adding;
    struct add_inputs inputs = {.a = 5, .b = 10};

    k_future_init_adding(&my_adding, add);
    k_future_exec_adding(&my_adding, inputs, K_NO_WAIT);
    LOG_INF("Output %d", k_future_get_adding(&my_adding, K_FOREVER));

    inputs.a = 10;
	k_future_exec_adding(&my_adding, inputs, K_NO_WAIT);
	LOG_INF("Output %d", k_future_get_adding(&my_adding, K_FOREVER));
	
    // struct my_future a_future;
    // a_future.a = 10;
    // a_future.b = 5;

    // int ret = k_future_init(&a_future.future, add);
    // if (ret != 0) {
    //     LOG_ERR("Init Fail - %d", ret);
	// }

    // ret = k_future_exec(&a_future.future, K_NO_WAIT);
    // if (ret < 0) {
    //     LOG_ERR("Exec Fail - %d", ret);
	// }
    // LOG_INF("waiting");
    // ret = k_future_get(&a_future.future, K_FOREVER);
    // if (ret != 0) {
    //     LOG_ERR("Get Fail - %d", ret);
	// }
    // LOG_INF("Output %d", a_future.out);

    // a_future.a = 5;
    // a_future.b = 6;

    // ret = k_future_exec(&a_future.future, K_NO_WAIT);
    // if (ret < 0) {
    //     LOG_ERR("Exec Fail - %d", ret);
	// }
    // LOG_INF("waiting");
    // ret = k_future_get(&a_future.future, K_FOREVER);
    // if (ret != 0) {
    //     LOG_ERR("Get Fail - %d", ret);
	// }
    // LOG_INF("Output %d", a_future.out);

    posix_exit(0);
}
