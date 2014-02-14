CC =    cc
CCC =    c++
CFLAGS =  -pipe -O -W -Wall -Winline -Wpointer-arith -Wno-unused-parameter -Werror -g -std=c11 -funit-at-a-time
CCFLAGS =  -pipe -O -W -Wall -Winline -Wpointer-arith -Wno-unused-parameter -Werror -g -std=c++11 -funit-at-a-time
CPP =   cc -E
LINK =  $(CC)

all: generate-input-file points-in-boxes

generate-input-file: generate-input-file.o
	$(CC) -o $@ $^ -luuid

generate-input-file.o: generate-input-file.c
	$(CC) -c $(CFLAGS) $(ALL_INCS) -o $@ $<

points-in-boxes: points-in-boxes.o
	$(CC) -o $@ $^ -luuid

points-in-boxes.o: points-in-boxes.cc
	$(CCC) -c $(CCFLAGS) $(ALL_INCS) -o $@ $<

