###################################
#Author: Tapas Sharma
# Generic makefile for C programs,
# write once and forget the rest.
###################################
#Compiler and Linker
CC          := gcc

#The Target Binary Program
TARGET      := statsd_proxy

#The Directories, Source, Includes, Objects, Binary and Resources
SRCDIR      := src
INCDIR      := inc
BUILDDIR    := obj
TARGETDIR   := bin
SRCEXT      := c
DEPEXT      := d
OBJEXT      := o
TESTDIR     := tests

#Flags, Libraries and Includes
CFLAGS      := -Wall -g
LIB         := -lpthread -lhiredis
INC         := -I$(INCDIR) -I/usr/local/include -I/usr/local/include/hiredis
INCDEP      := -I$(INCDIR)

#---------------------------------------------------------------------------------
#DO NOT EDIT BELOW THIS LINE
#---------------------------------------------------------------------------------
SOURCES     := $(shell find $(SRCDIR) -type f -name "*.$(SRCEXT)")
OBJECTS     := $(patsubst $(SRCDIR)/%,$(BUILDDIR)/%,$(SOURCES:.$(SRCEXT)=.$(OBJEXT)))

#Defauilt Make
all: resources directories $(TARGET)

#Remake
remake: cleaner all

#Make the Directories
directories:
	@mkdir -p $(TARGETDIR)
	@mkdir -p $(BUILDDIR)

#Clean only Objecst
clean:
	@$(RM) -rf $(BUILDDIR)
	@$(RM) -rf $(TARGETDIR)	
	@$(RM) -rf $(TESTDIR)/statsd_server.out

#Full Clean, Objects and Binaries
cleaner: clean
	@$(RM) -rf $(TARGETDIR)

#Pull in dependency info for *existing* .o files
-include $(OBJECTS:.$(OBJEXT)=.$(DEPEXT))

#Link
$(TARGET): $(OBJECTS)
	@echo "Starting Build"
	$(CC) -o $(TARGETDIR)/$(TARGET) $^ $(LIB)

#Compile
$(BUILDDIR)/%.$(OBJEXT): $(SRCDIR)/%.$(SRCEXT)
	@mkdir -p $(dir $@)
	@echo "Compiling : "$<
	$(CC) $(CFLAGS) $(INC) -c -o $@ $<
	@$(CC) $(CFLAGS) $(INCDEP) -MM $(SRCDIR)/$*.$(SRCEXT) > $(BUILDDIR)/$*.$(DEPEXT)
	@cp -f $(BUILDDIR)/$*.$(DEPEXT) $(BUILDDIR)/$*.$(DEPEXT).tmp
	@sed -e 's|.*:|$(BUILDDIR)/$*.$(OBJEXT):|' < $(BUILDDIR)/$*.$(DEPEXT).tmp > $(BUILDDIR)/$*.$(DEPEXT)
	@sed -e 's/.*://' -e 's/\\$$//' < $(BUILDDIR)/$*.$(DEPEXT).tmp | fmt -1 | sed -e 's/^ *//' -e 's/$$/:/' >> $(BUILDDIR)/$*.$(DEPEXT)
	@rm -f $(BUILDDIR)/$*.$(DEPEXT).tmp

#test target
test:
	@echo "Starting tests, this will also build the server, make sure redis is running and statsd is in /opt/statsd/bin"
	cd $(TESTDIR);./start_test.sh

#Non-File Targets
.PHONY: all remake clean cleaner resources test

