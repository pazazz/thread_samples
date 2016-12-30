#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <mysql.h>
#include "thread.h"

#define SEC_LOGO    "########"

static void *
connection_fn(void *args)
{
    MYSQL       *mysqls = NULL;
    conn_args   *thr = (conn_args *)args;
    gint        i = 0;

    g_message("%s connector start %s", SEC_LOGO, SEC_LOGO);

    mysqls = g_new0(MYSQL, thr->conn_num);

    mysql_init(NULL);
 
    /* init data */
    for (i = 0; i < thr->conn_num; i++)
    {
        MYSQL_RES *result = NULL;
        gchar *query = "select SQL_CALC_FOUND_ROWS id, title, desc_before as descBefore, desc_in as descIn, desc_after as descAfter, share, "
                "share_img as shareImg, status, list_jump_to_touch as listJumpToTouch, touch_url_for_list as touchUrlForList, "
                "next_activity_notice_close as nextActivityNoticeClose , type, effect_time as effectTime, time_info as timeInfo, "
                "content, banner_img_url as BannerImgUrl, banner_img_link as BannerImgLink, create_time as createTime, creator, "
                "activity_img_url as activityImgUrl, is_show_cate_desc as isShowCateDesc, is_show_time_countdown as isShowTimeCountdown, " 
                "countdown_text as countdownText, approve_operator as approveOperator from activity "
                 "where status=1 order by id";
        mysql_init(&mysqls[i]);
        mysql_real_connect(&mysqls[i], thr->host, thr->user, thr->passwd, thr->dbname, thr->port, NULL, 0);
        g_message("connect %d success", thr->conn_num);

        mysql_query(&mysqls[i], query);
        if (mysql_errno(&mysqls[i]) == 0) {
            result = mysql_store_result(&mysqls[i]);
            g_message("mysql_num_fields(result) = %d, mysql_num_rows(result) = %llu",
                            mysql_num_fields(result), mysql_num_rows(result));
            mysql_free_result(result);
        } else {
            g_message("error:%s", mysql_error(&mysqls[i]));
        }

    }
    g_message("ready to close %d connection", thr->conn_num);
    for (i = 0; i < thr->conn_num; i++)
    {
        mysql_close(&mysqls[i]);
    }
    g_message("close %d success", thr->conn_num);

    g_free(mysqls);

    g_thread_exit(0);
    return 0;
}


static void *
producer_fn(void *args)
{
    gint i = 0;

    /* init data */
    thr_args *thr = (thr_args *)args;

    for (i = 0; i < thr->magic; i++)
    {
        GString *data = g_string_new(NULL);
        g_string_append_printf(data, "thr_%d_%d", thr->thr_id, i);

        /* push to lock */
        usleep(1000);
        
        if (no_lock_queue_push(thr->q, data));
    }

    g_thread_exit(0);

    return 0;
}

static void *
consumer_fn(void *args)
{
    GString *data;

    /* init data */
    thr_args *thr = (thr_args *)args;
    while (TRUE) {
        /* pop data */
#if 0
        gint q_len = g_queue_get_length(thr->q);
        if (q_len > thr->log_q_max_length) {
            g_atomic_int_set(&thr->q->length_status, 1);
        } else {
            g_atomic_int_set(&thr->q->length_status, 0);
        }
#endif
        if (/*q_len < 3 */1) {
            usleep(50000);
            continue;
        }

        data = no_lock_queue_pop(thr->q);

        /* print data */
        if (data) {
            g_message("data:%s", data->str);
            g_string_free(data, TRUE);
        } else {
            usleep(50000);
        }
    }

    g_thread_exit(0);
    return 0;
}

thr_args *
thr_args_new(gint thr_id, gint magic, no_lock_queue *q)
{
    thr_args *args = g_new0(thr_args, 1);   

    args->thr_id = thr_id;
    args->magic = magic;
    args->q = q;

    return args;
}

void
thr_args_free(thr_args *args)
{
    g_free(args);
}

conn_args *
conn_args_new(gchar *host, gchar *user, gchar *passwd, gchar *dbname,
                            gint port, gint conn_num)
{
    conn_args *connector = g_new0(conn_args, 1);

    connector->host = host;
    connector->user = user;
    connector->passwd = passwd;
    connector->dbname = dbname;
    connector->port = port;
    connector->conn_num = conn_num;

    return connector;
}

void
conn_args_free(conn_args *connector)
{
    if (connector == NULL) return ;

    if (connector->host) g_free(connector->host);
    if (connector->user) g_free(connector->user);
    if (connector->passwd) g_free(connector->passwd);
    if (connector->dbname)  g_free(connector->dbname);
 
    g_free(connector);

    return ;

}
thr_data*
thr_data_new(GThread *thr, thr_args *thr_arg)
{
    thr_data* t_data = g_new0(thr_data, 1);

    t_data->thr = thr;
    t_data->thr_arg = thr_arg;
    return t_data;
}

void
thr_data_free(thr_data* t_data)
{
    g_thread_join(t_data->thr);
    g_free(t_data->thr_arg);

    g_free(t_data);
}


GPtrArray *
init_producers(gint item_num, no_lock_queue *q)
{
    gint i = 0;

    GPtrArray *thrs = g_ptr_array_new_with_free_func((GDestroyNotify)thr_data_free);

    for (i = 0; i < item_num; i++)
    {
        GError      *gerr = NULL;
        thr_args    *thr_arg = thr_args_new(i, item_num, q);
        GThread     *thr = g_thread_try_new("producers",
                                   (GThreadFunc)producer_fn,
                                   (gpointer)thr_arg, &gerr);

        thr_data *t_data = thr_data_new(thr, thr_arg);
        g_ptr_array_add(thrs, t_data);
    }

    return thrs;
}

thr_data *
init_consumer(gint item_num, no_lock_queue *q)
{
    GError      *gerr = NULL;
    thr_args    *thr_arg = thr_args_new(0, item_num, q);
    GThread     *consumer = g_thread_try_new("producers",
                                   (GThreadFunc)consumer_fn,
                                   (gpointer)thr_arg, &gerr);
    thr_data    *t_data = thr_data_new(consumer, thr_arg);

    return t_data;
}

thr_data2 *
init_connection(gchar *host, gchar *user,
                            gchar *passwd,
                            gchar *dbname,
                            gint port,
                            gint conn_num)
{
    GError      *gerr = NULL;
    conn_args    *thr_arg = conn_args_new(host, user, passwd, dbname, port, conn_num);
    GThread     *connector = g_thread_try_new("connector",
                                   (GThreadFunc)connection_fn,
                                   (gpointer)thr_arg, &gerr);
    thr_data2    *t_data = g_new0(thr_data2, 1);
    t_data->thr = connector;
    t_data->thr_arg = thr_arg;

    return t_data;
}

