VPATH = src
CPPFLAGS = -ansi -pedantic -Wall -Werror

objects = $(addprefix obj/, rshell.o type_prompt.o )

all: bin/rshell bin/ls

bin/rshell: $(objects)  | bin
		g++ -o $@ $(objects)
bin/ls: obj/ls.o  | bin
		g++ -o $@ obj/ls.o
#bin/cp: obj/cp.o | bin
#		g++ -o $@ obj/cp.o

obj/%.o: %.cpp
		g++ $(CPPFLAGS) -c -o $@ $<
obj/type_prompt.o: rshell.h
obj/rshell.o: rshell.h

$(objects): | obj

obj:
		mkdir obj

bin:
	    mkdir bin

clean:
		rm -rf obj bin
