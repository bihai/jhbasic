OBJDIR = obj
BINDIR = bin

# basic program to include in the VM
#BASIC = count
#BASIC = array
BASIC = index

BAUDRATE=115200
#BAUDRATE=57600

AVRPATH = /usr/local/CrossPack-AVR-20131216
AVRDUDE = $(AVRPATH)/bin/avrdude
AVRCONF = $(AVRPATH)/etc/avrdude.conf

PORT ?= /dev/cu.usbmodem1421

CC = avr-gcc
OBJCOPY = avr-objcopy
OBJSIZE = avr-size

MCU = atmega328p
FREQ = 16000000L

OBJS = \
$(OBJDIR)/db_vmavr.o \
$(OBJDIR)/db_vmint.o \
$(OBJDIR)/db_system.o \
$(OBJDIR)/avruart.o

TARGET_ELF = $(BINDIR)/vmavr.elf
TARGET = $(BINDIR)/vmavr.hex

CFLAGS = -Wall -Os -mmcu=$(MCU) -DF_CPU=$(FREQ) -MMD -DAVR -DAVR_VM -I../hdr
CFLAGS += -funsigned-char -funsigned-bitfields -fpack-struct -fshort-enums

#OBJS += $(OBJDIR)/db_vmdebug.o
#CFLAGS += -DVM_DEBUG

LDFLAGS = -Wl,-section-start=.vmimage=0x6800 -Wl,-Map=$(OBJDIR)/vmavr.map

vpath %.c ../vm
vpath %.c ../common

all:	$(TARGET)

$(OBJDIR) $(BINDIR):
	mkdir -p $@
	
$(OBJDIR)/db_vmavr.o:	vmimage.h

$(OBJDIR)/%.o:	%.c $(OBJDIR)
	$(CC) -c $(CFLAGS) $< -o $@ 

$(TARGET_ELF):	$(OBJS) $(BINDIR)
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJS) $(LIBS) -o $@
	$(OBJSIZE) --format=avr --mcu=atmega328p $(TARGET_ELF)
	
$(TARGET):	$(TARGET_ELF)
	$(OBJCOPY) -O ihex -j .text -j .vmimage -j .data  $< $@

$(BASIC).img:	../$(BASIC).bas
	../compile $< $@
	
vmimage.h:	$(BASIC).img bin2c
	./bin2c $< vmimage.h

bin2c:	bin2c.c
	cc -o bin2c bin2c.c

run:	$(TARGET)
	$(AVRDUDE) -C $(AVRCONF) -v -p atmega328p -c arduino -P $(PORT) -b $(BAUDRATE) -D -U flash:w:$(TARGET):i

clean:
	rm -rf $(OBJDIR) $(BINDIR) *.img vmimage.h bin2c
