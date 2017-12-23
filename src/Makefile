#
# General targets (all, clean)
#

all: readsup decode_base string_to_char learn guess

clean:
	rm readsup decode_base string_to_char learn guess *.o

#
# Executables
#

readsup: readsup.o subformat.o supformat.o common.o subtitle.o charlist.o \
		output.o subprop.o bitmap.o charutils.o util.o process.o
	gcc -o $@ $^

learn: learn.o common.o charlist.o charutils.o subprop.o bitmap.o util.o \
	process.o minlist.o
	gcc -o $@ $^

guess: guess.o common.o charlist.o charutils.o subprop.o bitmap.o util.o \
	process.o minlist.o
	gcc -o $@ $^

decode_base: decode_base.o common.o charutils.o bitmap.o subprop.o util.o
	gcc -o $@ $^

string_to_char: string_to_char.o common.o charutils.o bitmap.o subprop.o util.o
	gcc -o $@ $^

#
# Modules / objects
#

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

process.o: process.c common.h process.h
	gcc -c -Wall process.c

subprop.o: subprop.c common.h subprop.h
	gcc -c -Wall subprop.c

bitmap.o: bitmap.c common.h bitmap.h
	gcc -c -Wall bitmap.c

charutils.o: charutils.c common.h charutils.h bitmap.h
	gcc -c -Wall charutils.c

minlist.o: minlist.c common.h minlist.h
	gcc -c -Wall minlist.c

readsup.o: readsup.c common.h subtitle.h subformat.h supformat.h charlist.h output.h subprop.h
	gcc -c -Wall readsup.c

decode_base.o: decode_base.c common.h charutils.h
	gcc -c -Wall decode_base.c

string_to_char.o: string_to_char.c common.h charlist.h
	gcc -c -Wall string_to_char.c

util.o: util.c common.h util.h
	gcc -c -Wall util.c

learn.o: learn.c common.h charlist.h
	gcc -c -Wall learn.c

guess.o: guess.c common.h charlist.h
	gcc -c -Wall guess.c
