COMPILER_SRCDIR=compiler
VM_SRCDIR=vm
COMMON_SRCDIR=common
HDRDIR=hdr
COMPILER_OBJDIR=compiler_obj
VM_OBJDIR=vm_obj
LIBDIR=lib

COMPILER_OBJS = \
$(COMPILER_OBJDIR)/db_compiler.o \
$(COMPILER_OBJDIR)/db_expr.o \
$(COMPILER_OBJDIR)/db_generate.o \
$(COMPILER_OBJDIR)/db_scan.o \
$(COMPILER_OBJDIR)/db_statement.o \
$(COMPILER_OBJDIR)/db_symbols.o \
$(COMPILER_OBJDIR)/db_system.o \
$(COMPILER_OBJDIR)/db_vmdebug.o

VM_OBJS = \
$(VM_OBJDIR)/db_system.o \
$(VM_OBJDIR)/db_vmdebug.o \
$(VM_OBJDIR)/db_vmint.o

COMPILER_HDRS = \
db_compiler.h \
db_image.h \
db_system.h \
db_types.h \
db_vmdebug.h

VM_HDRS = \
db_image.h \
db_system.h \
db_types.h \
db_vm.h \
db_vmdebug.h

COMPILE_OBJS = \
$(COMPILER_OBJDIR)/compile.o \
$(COMPILER_OBJDIR)/osint_posix.o

EXECUTE_OBJS = \
$(VM_OBJDIR)/execute.o \
$(VM_OBJDIR)/osint_posix.o

#DEBUG += -DCOMPILER_DEBUG
#DEBUG += -DVM_DEBUG

#CFLAGS = -Wall -Os -I$(HDRDIR)
CFLAGS = -Wall -g -I$(HDRDIR) $(DEBUG)
LFLAGS = $(CFLAGS) -L$(LIBDIR)

all:	compile execute

compile:	$(COMPILE_OBJS) $(LIBDIR)/libcompiler.a
	cc $(LFLAGS) -o $@ $(COMPILE_OBJS) -lcompiler

execute:	$(EXECUTE_OBJS) $(LIBDIR)/libvm.a
	cc $(LFLAGS) -o $@ $(EXECUTE_OBJS) -lvm

$(LIBDIR)/libcompiler.a:	$(LIBDIR) $(COMPILER_OBJS)
	ar crs $@ $(COMPILER_OBJS)

$(LIBDIR)/libvm.a:	$(LIBDIR) $(VM_OBJS)
	ar crs $@ $(VM_OBJS)

$(COMPILER_OBJDIR)/%.o:	$(COMPILER_SRCDIR)/%.c $(COMPILER_OBJDIR)
	cc $(CFLAGS) -c -o $@ $<
	
$(COMPILER_OBJDIR)/%.o:	$(COMMON_SRCDIR)/%.c $(COMPILER_OBJDIR)
	cc $(CFLAGS) -c -o $@ $<
	
$(VM_OBJDIR)/%.o:	$(VM_SRCDIR)/%.c $(VM_OBJDIR)
	cc $(CFLAGS) -c -o $@ $<
	
$(VM_OBJDIR)/%.o:	$(COMMON_SRCDIR)/%.c $(VM_OBJDIR)
	cc $(CFLAGS) -c -o $@ $<
	
$(COMPILER_OBJDIR) $(VM_OBJDIR) $(LIBDIR):
	mkdir -p $@

run:	compile execute
	./compile count.bas count.img
	./execute count.img

clean:
	rm -rf $(COMPILER_OBJDIR) $(VM_OBJDIR) $(LIBDIR) *.img compile execute
	$(MAKE) -C vmavr clean
