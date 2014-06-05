# basic gnu tools
CC = propeller-elf-gcc
LD = propeller-elf-ld
LOADER = propeller-load

BASIC = count

vpath %c ../vm ../common

OBJS = \
db_system.o \
db_vmint.o \
db_vmprop.o

OBJS += db_vmdebug.o

ifndef MODEL
MODEL = xmmc
endif

#DEBUG += -DCOMPILER_DEBUG
#DEBUG += -DVM_DEBUG

CFLAGS = -Wall -Os -DPROPELLER_GCC -I ../hdr $(DEBUG)
LDFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti

all:    basic.elf

db_vmprop.o:     vmimage.h

%.dat:	%.spin
	@spin2cpp --dat -o $@ $<
	@echo $@
	
%.o: %.dat
	@propeller-elf-objcopy -I binary -B propeller -O propeller-elf-gcc --rename-section .data=.text $< $@
	@echo $@
	
$(BASIC).img:	../$(BASIC).bas
	../compile $< $@
	
vmimage.h:	$(BASIC).img bin2c
	./bin2c $< vmimage.h

bin2c:	bin2c.c
	cc -o bin2c bin2c.c

basic.elf: $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $<

run:	basic.elf
	$(LOADER) -b eeprom basic.elf -r -t

clean:
	rm -rf *.o *.elf *.img vmimage.h bin2c
