

all:	t_bitmap t_subtitle t_output

clean:
	rm t_bitmap t_subtitle t_output *.o

t_bitmap: t_bitmap.o unittest.o ../bitmap.o
	gcc -o $@ $^

t_subtitle: t_subtitle.o unittest.o ../subtitle.o ../bitmap.o
	gcc -o $@ $^

t_output: t_output.o unittest.o ../output.o ../common.o
	gcc -o $@ $^


unittest.o: unittest.c unittest.h
	gcc -c -Wall unittest.c

t_bitmap.o: t_bitmap.c ../bitmap.h unittest.h
	gcc -c -Wall t_bitmap.c

t_subtitle.o: t_subtitle.c ../subtitle.h ../bitmap.h unittest.h
	gcc -c -Wall t_subtitle.c

t_output.o: t_output.c ../output.h ../charlist.h unittest.h
	gcc -c -Wall t_output.c
