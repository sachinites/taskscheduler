#ifndef __TASK_SCHED__
#define __TASK_SCHED__

#include <pthread.h>
#include <semaphore.h>
#include <stdint.h>
#include <stdbool.h>
#include <list>
#include <time.h>
#include <map>

#define TASK_NAME_LEN   32

typedef enum watched_type_ {

    WATCHED_TYPE_BOOL,
    WATCHED_TYPE_Q,
    WATCHED_TYPE_MAX
} watched_type_t;

typedef struct boolean_watched_ {

    bool val;
    uint8_t id;
    pthread_mutex_t mutex;
    pthread_cond_t cv;
} boolean_watched_t ;

typedef struct q_watched_ {

    std::list <void *>q;
    uint8_t id;
    pthread_mutex_t mutex;
    pthread_cond_t cv;
} q_watched_t ;

typedef struct watched_object_ {

    watched_type_t type;
    uint8_t n_watchers;
    void *watched_entity;
} watched_object_t;

typedef enum task_state_ {

    TASK_INITIALIZED,
    TASK_READY,
    TASK_RUNNING,
    TASK_WAITING,
    TASK_TERMINATED
} task_state_t;

typedef void (*task_cbk)(void *);

typedef struct task_ {

    unsigned char task_name[TASK_NAME_LEN];
    pthread_t task_thread;
    sem_t zero_sema;
    task_state_t state;
    task_cbk cbk;
    time_t start_time;
    time_t fin_time;
    uint32_t n_scheduled;
    std::list<watched_object_t *> watched_objects;
} task_t;

typedef struct task_sched_ {

    pthread_t sched_thread;
    pthread_mutex_t mutex;
    pthread_cond_t cv;
    /* List of tasks watching the objects */
    std::map<watched_object_t *, std::list<task_t *>> watched_objects;
    /* List of tasks ready to run because watched object is signaled */
    std::map<watched_object_t *, std::list<task_t *>> run_q;
    /* List of watched objects signaled by the application */
    std::list<watched_object_t *> signaled_watched_objects;
    task_t *curr_task;
    std::list<task_t *>task_global_lst;
    std::list<watched_object_t *>watched_objects_global_lst;
} task_sched_t;

static inline bool 
watched_object_state (watched_object_t *watched_object, bool lock) {

    bool bool_rc;
    watched_type_t type = watched_object->type;

    switch (type) {

        case WATCHED_TYPE_BOOL:
        {
            boolean_watched_t *boolean_watched = (boolean_watched_t *) watched_object->watched_entity;
            if (lock) {
                pthread_mutex_lock(&boolean_watched->mutex);
                bool_rc = boolean_watched->val;
                pthread_mutex_unlock(&boolean_watched->mutex);
                return bool_rc;
            }
            return boolean_watched->val;
        }
        break;
        case WATCHED_TYPE_Q:
        {
            q_watched_t *q_watched = (q_watched_t *) watched_object->watched_entity;
            if (lock) {
                pthread_mutex_lock(&q_watched->mutex);
                bool_rc = q_watched->q.empty();
                pthread_mutex_unlock(&q_watched->mutex);
                return bool_rc;
            }
            return q_watched->q.empty();
        }
        break;
        default: ;
    }
    return false;
}

#endif 