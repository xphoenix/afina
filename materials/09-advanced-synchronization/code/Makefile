PROGS =	shared_mutex_demo count_down_latch_demo
LDFLAGS = -lpthread

all:	$(PROGS)

shared_mutex_demo:
	$(CXX) $(CFLAGS) shared_mutex.cpp shared_mutex_demo.cpp -o $@ $(LDFLAGS) $(LDLIBS)

count_down_latch_demo:
		$(CXX) $(CFLAGS) count_down_latch.cpp count_down_latch_demo.cpp -o $@ $(LDFLAGS) $(LDLIBS)
clean:
	rm -f $(PROGS) $(TEMPFILES)
