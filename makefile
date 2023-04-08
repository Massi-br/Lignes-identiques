.PHONY: clean dist

dist: clean
	tar -hzcf "$(CURDIR).tar.gz" hashtable/* hashtable_test/* holdall/* option/* makefile

clean:
	$(MAKE) -C hashtable_test clean
