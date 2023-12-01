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
		Iterator.h LoserTree.h Scan.h Filter.h Sort.h OVC.h File.h SharedBuffer.h Item.h
SRCS=	defs.cpp Assert.cpp Test.cpp \
		Iterator.cpp LoserTree.cpp Scan.cpp Filter.cpp Sort.cpp OVC.cpp File.cpp SharedBuffer.cpp Item.cpp

# compilation targets
OBJS=	defs.o Assert.o Test.o \
		Iterator.o LoserTree.o Scan.o Filter.o Sort.o OVC.o File.o SharedBuffer.o Item.o

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
Test.o : Iterator.h Scan.h Filter.h Sort.h
Iterator.o : Iterator.h Item.h
LoserTree.o : LoserTree.h Iterator.h OVC.h
Scan.o : Scan.h Iterator.h File.h
Filter.o : Filter.h Iterator.h
Sort.o : Sort.h Iterator.h LoserTree.h SharedBuffer.h
OVC.o : OVC.h
File.o : File.h Iterator.h
SharedBuffer.o : SharedBuffer.h File.h
Item.o : Item.h

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
