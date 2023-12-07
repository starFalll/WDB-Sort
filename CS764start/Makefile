CPPOPT=-g -Og -D_DEBUG
LIBS=-lstring -stdlib -liostream -lfstream -pthread -lpthread
# -O2 -Os -Ofast
# -fprofile-generate -fprofile-use
CPPFLAGS=$(CPPOPT) -Wall -ansi -pedantic -lpthread -pthread -std=c++11
# -Wparentheses -Wno-unused-parameter -Wformat-security
# -fno-rtti -std=c++11 -std=c++98


# documents and scripts
DOCS=Tasks.txt
SCRS=

# headers and code sources
HDRS=	defs.h \
		Iterator.h LoserTree.h Scan.h Filter.h Sort.h ovc.h File.h SharedBuffer.h DiskScan.h Item.h Verify.h
SRCS=	defs.cpp Assert.cpp Test.cpp \
		Iterator.cpp LoserTree.cpp Scan.cpp Filter.cpp Sort.cpp ovc.cpp File.cpp SharedBuffer.cpp DiskScan.cpp Item.cpp Verify.cpp

# compilation targets
OBJS=	defs.o Assert.o Test.o \
		Iterator.o LoserTree.o Scan.o Filter.o Sort.o ovc.o File.o SharedBuffer.o DiskScan.o Item.o Verify.o

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
Test.o : Iterator.h LoserTree.h Scan.h Filter.h Sort.h DiskScan.h Verify.h
Iterator.o LoserTree.o Scan.o Filter.o Sort.o : Iterator.h
Iterator.o : Item.h File.h
LoserTree.o : LoserTree.h ovc.h
Scan.o : Scan.h File.h
Filter.o : Filter.h
Sort.o : Sort.h LoserTree.h
ovc.o : ovc.h
File.o : File.h Iterator.h
SharedBuffer.o : SharedBuffer.h File.h
DiskScan.o : DiskScan.h SharedBuffer.h LoserTree.h
Item.o : Item.h
Verify.o : Verify.h File.h

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
