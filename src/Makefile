all: readsup decode_base string_to_char

clean:
	rm readsup decode_base string_to_char *.o

readsup: readsup.o subformat.o supformat.o common.o subtitle.o charlist.o \
		output.o subprop.o bitmap.o charutils.o util.o
	gcc -o $@ $^

decode_base: decode_base.o common.o charutils.o bitmap.o subprop.o util.o
	gcc -o $@ $^

string_to_char: string_to_char.o common.o charutils.o bitmap.o subprop.o util.o
	gcc -o $@ $^

common.o: common.c common.h
	gcc -c -Wall common.c

subtitle.o: subtitle.c common.h subtitle.h
	gcc -c -Wall subtitle.c

subformat.o: subformat.c common.h subtitle.h subformat.h
	gcc -c -Wall subformat.c

supformat.o: supformat.c common.h subtitle.h supformat.h
	gcc -c -Wall supformat.c

charlist.o: charlist.c common.h charlist.h charutils.h subprop.h
	gcc -c -Wall charlist.c

output.o: output.c common.h output.h
	gcc -c -Wall output.c

subprop.o: subprop.c common.h subprop.h
	gcc -c -Wall subprop.c

bitmap.o: bitmap.c common.h bitmap.h
	gcc -c -Wall bitmap.c

charutils.o: charutils.c common.h charutils.h bitmap.h
	gcc -c -Wall charutils.c

readsup.o: readsup.c common.h subtitle.h subformat.h supformat.h charlist.h output.h subprop.h
	gcc -c -Wall readsup.c

decode_base.o: decode_base.c common.h charutils.h
	gcc -c -Wall decode_base.c

string_to_char.o: string_to_char.c common.h charlist.h
	gcc -c -Wall string_to_char.c

util.o: util.c common.h util.h
	gcc -c -Wall util.c
