all: readsup decode_base

clean:
	rm readsup decode_base *.o

readsup: readsup.o subformat.o supformat.o common.o subtitle.o charlist.o \
		output.o subprop.o bitmap.o charmatch.o util.o
	gcc -o $@ $^

decode_base: decode_base.o common.o charlist.o bitmap.o subprop.o util.o
	gcc -o $@ $^

common.o: common.c common.h
	gcc -c -Wall common.c

subtitle.o: subtitle.c common.h subtitle.h
	gcc -c -Wall subtitle.c

subformat.o: subformat.c common.h subtitle.h subformat.h
	gcc -c -Wall subformat.c

supformat.o: supformat.c common.h subtitle.h supformat.h
	gcc -c -Wall supformat.c

charlist.o: charlist.c common.h charlist.h subprop.h
	gcc -c -Wall charlist.c

output.o: output.c common.h output.h
	gcc -c -Wall output.c

subprop.o: subprop.c common.h subprop.h
	gcc -c -Wall subprop.c

bitmap.o: bitmap.c common.h bitmap.h
	gcc -c -Wall bitmap.c

charmatch.o: charmatch.c common.h charmatch.h bitmap.h
	gcc -c -Wall charmatch.c

readsup.o: readsup.c common.h subtitle.h subformat.h supformat.h charlist.h output.h subprop.h
	gcc -c -Wall readsup.c

decode_base.o: decode_base.c common.h charlist.h
	gcc -c -Wall decode_base.c

util.o: util.c common.h util.h
	gcc -c -Wall util.c
