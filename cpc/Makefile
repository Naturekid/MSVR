SRC = list.c routing_table.c cpc_prot.c cpc_rt.c cpc_queue.c \
	  linux/nl.c linux/cpc_prot_linux.c linux/cpc_rt_linux.c
SRC_CPP = ns/cpc_queue_ns.cc ns/cpc_agent.cc

NS_SRC = list.c routing_table.c cpc_prot.c cpc_rt.c
NS_SRC_CPP = ns/cpc_queue_ns.cc ns/cpc_agent.cc

NS_OBJ = $(NS_SRC:%.c=%.o)
NS_CPP_OBJ = $(NS_SRC_CPP:%.cc=%.o)

OBJS = $(SRC:%.c=%.o)

CC = gcc
LD = ld
CPP = g++
AR = ar
ARFLAGS = rc
CFLAGS = -Wall -g -DCPC_NS

NS_INC=
NS_DEFS=

#default: libcpc.a kcpc

#$(OBJS): %.o: %.c Makefile
#	$(CC) $(CFLAGS) -c -o $@ $<

libcpc.a: $(NS_OBJ) $(NS_CPP_OBJ)
	$(AR) $(ARFLAGS) $@ $(NS_CPP_OBJ) $(NS_OBJ)

$(NS_OBJ): %.o: %.c Makefile
	$(CPP) $(CFLAGS) $(NS_INC) $(NS_DEFS) -c -o $@ $<

$(NS_CPP_OBJ): %.o: %.cc Makefile
	$(CPP) $(CFLAGS) $(NS_INC) $(NS_DEFS) -c -o $@ $<

#libcpc.a: $(OBJS)
#	$(AR) $(ARFLAGS) $@ $(OBJS)

kcpc:
	$(MAKE) -C linux_kernel

.PHONY: clean tags cscope

clean:
	rm -f *.o libcpc.a ns/*.o cpc.ko cscope.* tags linux/*.o
	cd linux_kernel && $(MAKE) clean

tags:
	ctags -R .

cscope:
	find . -name "*.c" -o -name "*.cc" -o -name "*.h" > cscope.files
	cscope -bkq -i cscope.files
