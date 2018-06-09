SRC=phyreg.c

phyreg: $(SRC)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(SRC)


install: phyreg
	mkdir -p /usr/bin
	install phyreg /usr/bin

clean:
	rm -f devmem2
	rm -f phyreg
