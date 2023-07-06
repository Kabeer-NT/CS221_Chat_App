PROG = project05
OBJS = project05.o presence.o tcp.o

%.o: %.c
	gcc -c -g -o $@ $<

$(PROG): $(OBJS)
	gcc -g -o $@ $^

clean: 
	rm -rf $(PROG) $(OBJS)
