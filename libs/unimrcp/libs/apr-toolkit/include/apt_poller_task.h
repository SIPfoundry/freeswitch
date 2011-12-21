/*
 * Copyright 2008-2010 Arsen Chaloyan
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 * 
 * $Id: apt_poller_task.h 1708 2010-05-24 17:03:25Z achaloyan $
 */

#ifndef APT_POLLER_TASK_H
#define APT_POLLER_TASK_H

/**
 * @file apt_poller_task.h
 * @brief Poller Task
 */ 

#include "apt_pollset.h"
#include "apt_task.h"
#include "apt_timer_queue.h"

APT_BEGIN_EXTERN_C

/** Opaque poller task declaration */
typedef struct apt_poller_task_t apt_poller_task_t;

/** Function prototype to handle signalled descripors */
typedef apt_bool_t (*apt_poll_signal_f)(void *obj, const apr_pollfd_t *descriptor);


/**
 * Create poller task.
 * @param max_pollset_size the maximum number of descriptors pollset can hold
 * @param signal_handler the handler of signalled descriptors
 * @param obj the external object to pass to callback
 * @param msg_pool the pool of task messages
 * @param pool the pool to allocate memory from
 */
APT_DECLARE(apt_poller_task_t*) apt_poller_task_create(
										apr_size_t max_pollset_size,
										apt_poll_signal_f signal_handler,
										void *obj,
										apt_task_msg_pool_t *msg_pool,
										apr_pool_t *pool);

/**
 * Destroy poller task.
 * @param task the task to destroy
 */
APT_DECLARE(apt_bool_t) apt_poller_task_destroy(apt_poller_task_t *task);

/**
 * Cleanup poller task.
 * @param task the task to cleanup
 *
 * @remark This function should be considered in protected scope. 
 * It will be called on task destroy unless you override the behavior.
 */
APT_DECLARE(void) apt_poller_task_cleanup(apt_poller_task_t *task);

/**
 * Start poller task and wait for incoming messages.
 * @param task the task to start
 */
APT_DECLARE(apt_bool_t) apt_poller_task_start(apt_poller_task_t *task);

/**
 * Terminate poller task.
 * @param task the task to terminate
 */
APT_DECLARE(apt_bool_t) apt_poller_task_terminate(apt_poller_task_t *task);

/**
 * Get task base.
 * @param task the poller task to get task base from
 */
APT_DECLARE(apt_task_t*) apt_poller_task_base_get(const apt_poller_task_t *task);

/**
 * Get task vtable.
 * @param task the poller task to get vtable from
 */
APT_DECLARE(apt_task_vtable_t*) apt_poller_task_vtable_get(apt_poller_task_t *task);

/**
 * Get external object.
 * @param task the poller task to get object from
 */
APT_DECLARE(void*) apt_poller_task_object_get(const apt_poller_task_t *task);

/**
 * Get pollset.
 * @param task the poller task to get pollset from
 */
APT_DECLARE(apt_pollset_t*) apt_poller_task_pollset_get(const apt_poller_task_t *task);

/**
 * Create timer.
 * @param task the poller task to create timer in the scope of
 * @param proc the timer callback
 * @param obj the object to pass to callback
 * @param pool the pool to allocate memory from
 */
APT_DECLARE(apt_timer_t*) apt_poller_task_timer_create(
								apt_poller_task_t *task, 
								apt_timer_proc_f proc, 
								void *obj, 
								apr_pool_t *pool);


APT_END_EXTERN_C

#endif /* APT_POLLER_TASK_H */
