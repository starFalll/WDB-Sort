CPPOPT=-g -D_DEBUG
LIBS=-lstring -stdlib -liostream -lfstream
# -O2 -Os -Ofast
# -fprofile-generate -fprofile-use
CPPFLAGS=$(CPPOPT) -Wall -ansi -pedantic -std=c++11
# -Wparentheses -Wno-unused-parameter -Wformat-security
# -fno-rtti -std=c++11 -std=c++98


# documents and scripts
DOCS=Tasks.txt
SCRS=

# headers and code sources
HDRS=	defs.h \
		Iterator.h LoserTree.h Scan.h Filter.h Sort.h Ovc.h
SRCS=	defs.cpp Assert.cpp Test.cpp \
		Iterator.cpp LoserTree.cpp Scan.cpp Filter.cpp Sort.cpp Ovc.cpp

# compilation targets
OBJS=	defs.o Assert.o Test.o \
		Iterator.o LoserTree.o Scan.o Filter.o Sort.o Ovc.o

# RCS assists
REV=-q -f
MSG=no message

# default target
#
Test : Makefile $(OBJS)
	g++ $(CPPFLAGS) -o Test $(OBJS)

trace : Test Makefile
	@date > trace
	./Test >> trace
	@size -t Test $(OBJS) | sort -r >> trace

$(OBJS) : Makefile defs.h
Test.o : Iterator.h LoserTree.h Scan.h Filter.h Sort.h
Iterator.o LoserTree.o Scan.o Filter.o Sort.o Ovc.o: Iterator.h
LoserTree.o : LoserTree.h Ovc.h
Scan.o : Scan.h
Filter.o : Filter.h
Sort.o : Sort.h LoserTree.h
Ovc.o : Ovc.h

list : Makefile
	echo Makefile $(HDRS) $(SRCS) $(DOCS) $(SCRS) > list
count : list
	@wc `cat list`

ci :
	ci $(REV) -m"$(MSG)" $(HDRS) $(SRCS) $(DOCS) $(SCRS)
	ci -l $(REV) -m"$(MSG)" Makefile
co :
	co $(REV) -l $(HDRS) $(SRCS) $(DOCS) $(SCRS)

clean :
	@rm -f $(OBJS) Test Test.stackdump trace
