all:	objdir $(OLD)/orangol.so

MODULENAME=ORANGOL
include ../makefile.defs
-include makefile.deps

$(OLD)/orangol.so:	ppp/stamp px/stamp $(ORANGOL_OBJECTS)
	$(LINKER) $(ORANGOL_OBJECTS) $(LINKOPTIONS) -o $(OLD)/orangol.so
ifeq ($(OS), Darwin)
	install_name_tool -id $(DESTDIR)/orangol.so $(OLD)/orangol.so
endif

clean:	cleantemp
	rm -f $(OLD)/orangol.so
