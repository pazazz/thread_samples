#ifndef __THREAD__H__
#define __THREAD__H__

#include "no_lock_queue.h"

typedef struct thr_args {
    gint thr_id;
    gint magic;
    no_lock_queue *q;
} thr_args;

typedef struct thr_data {
    GThread     *thr;
    thr_args    *thr_arg;
} thr_data;

typedef struct conn_args {
    gchar   *host;
    gchar   *user;
    gchar   *passwd;
    gchar   *dbname;
    gint    port;
    gint    conn_num;
} conn_args;

typedef struct thr_data2 {
    GThread      *thr;
    conn_args    *thr_arg;
} thr_data2;



thr_data *init_consumer(gint item_num, no_lock_queue *q);
GPtrArray *init_producers(gint item_numi, no_lock_queue *q);
thr_data2 *init_connection(gchar *host, gchar *user,
                            gchar *passwd, gchar *dbname,
                            gint port, gint conn_num);

void thr_data_free(thr_data* t_data);

#endif /* __THREAD__H__ */
