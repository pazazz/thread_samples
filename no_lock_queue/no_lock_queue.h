#ifndef __NO_LOCK_QUEUE_H__
#define __NO_LOCK_QUEUE_H__

#include <glib.h>

#define DEFAULT_LOG_WAIT_TIMEOUT_US 1

typedef struct log_q_node_no_lock {
    GString               *data;
    struct log_q_node_no_lock *next;
} log_q_node_no_lock;

typedef struct no_lock_queue {
    log_q_node_no_lock *head;
    log_q_node_no_lock *tail;
    gint q_length;
    gint q_max_length;
} no_lock_queue;

no_lock_queue* no_lock_queue_new();
void no_lock_queue_free(no_lock_queue *lq);
gint no_lock_queue_push(no_lock_queue *lq, GString *data);
GString* no_lock_queue_pop(no_lock_queue *lq);


typedef struct log_queue {
    GQueue *log_q;
    GMutex log_q_h_lock;
    GMutex log_q_t_lock;
    gint  log_q_max_length;
    gint  log_q_wait_timeout_us;
    volatile gint q_status;  
} log_queue;

log_queue* log_queue_new();
void log_queue_free(log_queue *lq);
gint log_queue_push(log_queue *lq, gchar *data);
gchar* log_queue_pop();

typedef struct mpscq_node_t {
    GString *data;
    struct mpscq_node * volatile next;
} mpscq_node_t;

typedef struct mpscq_t {
  mpscq_node_t * volatile  head;
  mpscq_node_t             *tail;
  struct mpscq_node_t      stub;
} mpscq_t;

void mpscq_create(mpscq_t *self, mpscq_node_t *stub);
void mpscq_push(mpscq_t *self, mpscq_node_t  *n);
mpscq_node_t *mpscq_pop(mpscq_t *self);

#endif
