BOFNAME := $(shell pwd | awk -F/ '{print $$NF}')
COMINCLUDE := -I ../../common
LIBINCLUDE := 
CC_x64 := x86_64-w64-mingw32-gcc
CC_x86 := i686-w64-mingw32-gcc
CC=x86_64-w64-mingw32-clang

all:
	$(CC_x64) -o $(BOFNAME).x64.o $(COMINCLUDE) -Os -c $(BOFNAME).c -DBOF

clean:
	rm $(BOFNAME).*.exe
