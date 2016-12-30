OBJ := main.o thread.o no_lock_queue.o
CFLAGS := -Wall -g
LIB := -L/usr/local/lib -lglib-2.0 -L/usr/lib64/ -lmysqlclient_r
INCLUDE := -I. -I/usr/include/glib-2.0 -I/usr/lib64/glib-2.0/include -I/usr/include/mysql/

no_lock: $(OBJ)
	gcc $(CFLAGS)  $(LIB) $(INCLUDE) -o $@ $(OBJ)

main.o : main.c thread.h no_lock_queue.h
	gcc $(CFLAGS) -c  $(LIB) $(INCLUDE) -o $@ main.c
thread.o : thread.c thread.h no_lock_queue.h
	gcc $(CFLAGS) -c   $(LIB) $(INCLUDE) -o $@ thread.c 

no_lock_queue.o : no_lock_queue.c no_lock_queue.h
	gcc $(CFLAGS) -c $(LIB) $(INCLUDE) -o $@ no_lock_queue.c

.PHONY : clean
clean:
	rm -rf no_lock $(OBJ)
