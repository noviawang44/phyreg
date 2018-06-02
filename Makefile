SRC=phyreg.c

phyreg: $(SRC)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(SRC)


install: phyreg
	mkdir -p $(DESTDIR)/usr/bin
	install phyreg $(DESTDIR)/usr/bin

clean:
	rm -f devmem2
	rm -f phyreg
