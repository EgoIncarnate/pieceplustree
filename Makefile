all: test-piece-table timed test-linked-array test-iqueue

PKGS = glib-2.0 gtk+-3.0
CFLAGS = $(shell pkg-config --cflags $(PKGS))
LDFLAGS = $(shell pkg-config --libs $(PKGS))

#DEBUG = -ggdb -fprofile-arcs -ftest-coverage -DG_DISABLE_ASSERT
DEBUG = -DG_DISABLE_ASSERT
WARNINGS = -Wall
OPTS = -march=native -O3

HEADERS = iqueue.h linked-array.h piece-table.h

test-iqueue: test-iqueue.c iqueue.h
	$(CC) -o $@ $(WARNINGS) $(CFLAGS) $(LDFLAGS) $(DEBUG) $(OPTS) test-iqueue.c

test-piece-table: test-piece-table.c iqueue.h linked-array.h piece-table.c
	$(CC) -o $@ $(WARNINGS) $(CFLAGS) $(LDFLAGS) $(DEBUG) $(OPTS) test-piece-table.c piece-table.c

test-linked-array: test-linked-array.c linked-array.h iqueue.h
	$(CC) -o $@ $(WARNINGS) $(CFLAGS) $(LDFLAGS) $(DEBUG) $(OPTS) test-linked-array.c

timed: timed.c piece-table.c piece-table.h iqueue.h linked-array.h
	$(CC) -o $@ $(WARNINGS) $(CFLAGS) $(LDFLAGS) $(DEBUG) $(OPTS) timed.c piece-table.c

gtk-widget: piece-table.c gtk/gcw-view.h gtk/gcw-view.c gtk/main.c gtk/gcw-buffer.c gtk/gcw-buffer.h
	$(CC) -o $@ $(WARNINGS) $(CFLAGS) $(LDFLAGS) $(DEBUG) $(OPTS) piece-table.c gtk/gcw-view.c gtk/main.c gtk/gcw-buffer.c

clean:
	rm -f test-piece-table *.o *.gcno *.gcda timed test-linked-array test-iqueue gtk-widget
