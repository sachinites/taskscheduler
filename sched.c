#include <assert.h>
#include "sched.h"

static void
task_scheduler_task_schedule_run (task_sched_t *sched, watched_object_t *watched_object, task_t *task) {

    task->state = TASK_READY;


}


void *
task_scheduler_fn (void *arg) {

    task_t *task;
    std::list<task_t *> *task_lst;
    std::list<task_t *>::iterator it;

    watched_object_t *watched_object;

    task_sched_t *sched = (task_sched_t *)arg;

    pthread_mutex_lock (&sched->mutex);
    
    while (true) {

        while (sched->signaled_watched_objects.empty()) {
            pthread_cond_wait (&sched->cv, &sched->mutex);
        }

        watched_object = sched->signaled_watched_objects.front();
         sched->signaled_watched_objects.pop_front();

        task_lst = &sched->watched_objects[watched_object];

        for (it = task_lst->begin(); it != task_lst->end(); ++it) {

            task = *it;
            
            if (task->state == TASK_READY || task->state == TASK_RUNNING) {
                continue;
            }

            assert (task->state == TASK_WAITING);

            task_scheduler_task_schedule_run (sched, watched_object, task);
        }
    }

    pthread_mutex_unlock (&sched->mutex);

    return NULL;
}