OUT  	= test
CC		= g++
FLAGS   = -Wall -g -c -O3

SOURCE			= test.cpp
HEADER			=

DIR_BUILD = ./Build/
OBJS 		= test.o

LIBS = -lgsl -lgslcblas

# -g option enables debugging mode
# -c flag generates object code for separate files

all: $(OUT)

$(OUT): $(OBJS) $(SOURCE) $(HEADER)
	$(CC) -g $(OBJS) -o $(OUT) $(LIBS)



# clean house
clean:
	rm -f $(OBJS)
	rm -r -f $(DIR_BUILD)
	rm -f $(OUT)

# do a bit of accounting
count:
	wc $(SOURCE) $(HEADER)
