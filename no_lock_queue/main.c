#include <glib.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mysql.h>
#include "no_lock_queue.h"
#include "thread.h"

int
main(int argc, char *argv[])
{

#if 0
    GPtrArray *thr_array = NULL;
    thr_data *thr = NULL;

    /* init no lock queue */
    no_lock_queue* no_lock = no_lock_queue_new();

    /* init consumer */
    thr = init_consumer(0, no_lock);

    /* init productor */
    thr_array = init_producers(1000, no_lock);
 
    /* recycle threads */
    g_ptr_array_free(thr_array, TRUE);

    thr_data_free(thr);
    no_lock_queue_free(no_lock);
#endif
#if 0
    gint i = 0;
    gint times = 0;

    if (argc < 1) {
        g_message("argc < 1, like ./no_lock 100");
        return 0;
    }

    times = atoi(argv[1]);

    GPtrArray* sqls = g_ptr_array_new();

    /* for init thread env */
    mysql_init(NULL);

    for (i = 0; i < times; i++) {
        thr_data2 *t2 = init_connection("10.4.237.16",
                                            "atlas",
                                            "123456",
                                            "test",
                                            6002,
                                            1);
       g_ptr_array_add(sqls, t2);
    }

    for (i = 0; i < sqls->len; i++) {
        thr_data2 *t2 = g_ptr_array_index(sqls, i);
        g_thread_join(t2->thr);
        g_free(t2->thr_arg);
        g_free(t2);
        
    }
    g_ptr_array_free(sqls, FALSE); 
#endif

    gchar   *shadow_tbl_name="_shadow_order_";
    gchar   *tbl_name = NULL;
    gint    str_length = strlen(shadow_tbl_name);
    gint    prefix_length = strlen("_shadow_");
    gint    suffix_length = 1; /* strlen("_") */

    tbl_name = g_strndup(shadow_tbl_name + prefix_length, str_length - prefix_length - suffix_length);
    //sscanf(shadow_tbl_name, "_shadow_%s_", tbl_name);

    g_message("real_table_name = %s", tbl_name);

    g_free(tbl_name);


    return 0;
}

