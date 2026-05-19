NSPIRE_MAKEFILE := Makefile.nspire

.PHONY: all lib example clean nspire

all lib example clean:
	$(MAKE) -f $(NSPIRE_MAKEFILE) $@

nspire: all
