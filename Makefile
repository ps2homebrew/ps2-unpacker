SOURCES = InterTables.c Interpreter.c memory.c ps2-unpacker.c DisR5900asm.c DisR5900.c MMI.c
HEADERS = Debug.h InterTables.h Interpreter.h R5900.h defines.h memory.h
TARGET = ps2-unpacker
VERSION = 0.1.2

CC = gcc
CPPFLAGS = -I . -O3 -Wall
CPPFLAGS += -DVERSION=\"$(VERSION)\"
LDFLAGS =

all: $(TARGET)

$(TARGET): $(subst .c,.o,$(SOURCES)) $(HEADERS)
	$(CC) -o $(TARGET) $(CPPFLAGS) $(LDFLAGS) $(subst .c,.o,$(SOURCES))

clean:
	rm -f *.o $(TARGET) $(TARGET).exe *.gz *.zip

rebuild: clean all

mingw: $(TARGET).exe

$(TARGET).exe: $(SOURCES) $(HEADERS)
	i586-mingw32msvc-gcc -o $(TARGET).exe $(CPPFLAGS) $(LDFLAGS) $(SOURCES) mingw-getopt/*.c -I mingw-getopt

dist: all mingw COPYING README.txt $(SOURCES) $(HEADERS)
	strip $(TARGET)
	i586-mingw32msvc-strip $(TARGET).exe
	upx-nrv --best $(TARGET)
	tar cvfz $(TARGET)-$(VERSION)-linux.tar.gz $(TARGET) COPYING README.txt
	zip -9 $(TARGET)-$(VERSION)-win32.zip $(TARGET).exe COPYING README.txt
	tar cvfz $(TARGET)-$(VERSION)-source.tar.gz $(SOURCES) $(HEADERS) COPYING README.txt Makefile

redist: clean dist

release: redist
	rm -rf /var/www/softwares/ps2-unpacker/*
	cp *.gz *.zip COPYING README.txt /var/www/softwares/ps2-unpacker
