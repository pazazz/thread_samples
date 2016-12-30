#include <stdio.h>
#include <stdlib.h>

#include "no_lock_queue.h"

#define DEFAULT_LOG_MAX_NUM 100

#define CAS(x, y, z)    __sync_bool_compare_and_swap(x, y, z)
#define CAS_ADD(x,y)    __sync_fetch_and_add(x, y)
#define CAS_SUB(x,y)    __sync_fetch_and_sub(x,y)

no_lock_queue*
no_lock_queue_new()
{
    no_lock_queue *lq_no_lock = g_new0(no_lock_queue, 1);

    lq_no_lock->head = lq_no_lock->tail = g_new0(log_q_node_no_lock, 1);

    lq_no_lock->head->next = NULL;
    lq_no_lock->tail->next = NULL;
    lq_no_lock->q_length = 0;
    lq_no_lock->q_max_length = DEFAULT_LOG_MAX_NUM;

    return lq_no_lock;
}

void
no_lock_queue_free(no_lock_queue *lq)
{
    log_q_node_no_lock *lq_node = NULL, *tmp_node = NULL;

    if (lq == NULL) return ;

    lq_node = lq->head;
    while (lq_node) {
        tmp_node = lq_node;
        if (tmp_node->data != NULL) {
            g_string_free(tmp_node->data, TRUE);
        }

        lq_node = lq_node->next;
        g_free(tmp_node);

    }

    g_free(lq);
}

gint
no_lock_queue_push(no_lock_queue *lq, GString *data)
{
    log_q_node_no_lock *q = NULL,  *tail = NULL, *next = NULL;

    /* lq->q_max_length == 0 specify unlimited */
    if (lq->q_max_length > 0 && lq->q_length > lq->q_max_length) {
        g_message("%s: q_length = %d, skip = %s", __func__, lq->q_length, data->str);
        g_string_free(data, TRUE);
        return 1;
    }

    q = g_new0(log_q_node_no_lock, 1);
    q->data = data;
    q->next = NULL;

    while (TRUE) {
        tail = lq->tail;
        next = lq->tail->next;

        if (tail != lq->tail) {
            continue;
        }

        if (next != NULL) {
           continue;
        }

        if (CAS(&tail->next, next, q)) {
            break;
        }
    }

    lq->q_length++;

    g_message("%s: q_length = %d, new_item=%p", __func__, lq->q_length, q);
    CAS(&lq->tail, tail, q);
   
    return 0;
}

GString *
no_lock_queue_pop(no_lock_queue *lq)
{
    log_q_node_no_lock *head = NULL, *tail = NULL, *next = NULL;
    GString *res = NULL;

    while (TRUE) {
        head = lq->head;
        tail = lq->tail;

        next = lq->head->next;
        if (head != lq->head) {
            continue;
        }

        if (next == NULL) {
            return NULL;
        }

        if (next == tail) {
            CAS(&lq->tail, tail, head);
        }

        res = next->data;
        /* set head next pointer */
        if (CAS(&lq->head->next, head->next, next->next)) {
            lq->q_length--;
            g_message("%s: q_length = %d, next = %p", __func__,  lq->q_length, next);
            break;
        }
    }
    g_free(next);

    return res;
}

log_queue*
log_queue_new(gint q_size)
{
    log_queue *lq = g_new0(log_queue, 1);

    lq->log_q = g_queue_new();

    g_mutex_init(&lq->log_q_h_lock);
    g_mutex_init(&lq->log_q_t_lock);

    lq->log_q_max_length = DEFAULT_LOG_MAX_NUM;
    lq->log_q_wait_timeout_us = DEFAULT_LOG_WAIT_TIMEOUT_US;

    lq->q_status = 0;

    return lq;
}

void
log_queue_free(log_queue *lq)
{
    if (lq == NULL) return;

    g_queue_free_full(lq->log_q, g_free);
    g_mutex_clear(&lq->log_q_h_lock);
    g_mutex_clear(&lq->log_q_t_lock);

    g_free(lq);
}

gint
log_queue_push(log_queue *lq, gchar *data)
{
    GList *link = g_list_prepend (NULL, data);

    g_mutex_lock(&lq->log_q_t_lock);

    if (g_atomic_int_get(&lq->q_status) == 1) {
        return -1;
    }

    g_mutex_lock(&lq->log_q_t_lock);
    g_queue_push_tail_link(lq->log_q, link);
    g_mutex_unlock(&lq->log_q_t_lock);

    return 0;
}

gchar *
log_queue_pop(log_queue *lq)
{
    GList *head_item = NULL;
    gchar *res = NULL;

    head_item = g_queue_pop_head_link(lq->log_q);
    if (head_item == NULL || head_item->data == NULL) {
        return NULL;
    }

    res = head_item->data;
    g_list_free_1(head_item);

    return res;
}

#if 0
void mpscq_create(mpscq_t* self, mpscq_node_t* stub) 
{ 
  /* stub->next = 0; */
  /* self->head = stub; */
  /* self->tail = stub; */
  self->head = &self->stub;
  self->tail = &self->stub;
  self->stub.next = NULL;
} 

void mpscq_push(mpscq_t* self, mpscq_node_t* n)
{
  n->next = 0;
  mpscq_node_t* prev;
  atomic_exchange_explicit(&self->head, &n, &prev,  __ATOMIC_SEQ_CST);
  //(*)
  prev->next = n;
}

mpscq_node_t* mpscq_pop(mpscq_t* self)
{
    mpscq_node_t* tail = self->tail;
    mpscq_node_t* next = tail->next;
    if (tail == &self->stub)
    {
        if (0 == next)
            return 0;
        self->tail = next;
        tail = next;
        next = next->next;
    }
    if (next)
    {
        self->tail = next;
        return tail;
    }
    mpscq_node_t* head = self->head;
    if (tail != head)
        return 0;
    mpscq_push(self, &self->stub);
    next = tail->next;
    if (next)
    {
        self->tail = next;
        return tail;
    }
    return 0;
}
#endif
