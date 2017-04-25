SRC=phyreg.c

phyreg: $(SRC)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(SRC)

devmem2: $(SRC)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(SRC)

install:
	mkdir -p $(DESTDIR)/usr/bin
	install devmem2 $(DESTDIR)/usr/bin
	install phyreg $(DESTDIR)/usr/bin

clean:
	rm -f devmem2
	rm -f phyreg
