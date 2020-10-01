# -*- Makefile -*-

main: mylibrary.c
	clang -shared -fPIC -Wl,-soname,libmylibrary.so -ldl -o libmylibrary.so mylibrary.c -lc
	@echo "\n----------"
	


clean: 
	rm -f libmylibrary.so


