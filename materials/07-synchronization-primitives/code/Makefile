PROGS =	mutex cv lock
LDFLAGS = -lpthread

all:	$(PROGS)

%:	%.c
	$(CC) $(CFLAGS) $@.c -o $@ $(LDFLAGS) $(LDLIBS)

clean:
	rm -f $(PROGS) $(TEMPFILES)
