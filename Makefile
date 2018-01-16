dec64.o: dec64.asm dec64.h
	/usr/local/bin/nasm --prefix _ -f macho64 dec64_unix.asm -o dec64.o

dec64_math.o: dec64_math.c dec64_math.h dec64.o
	clang -c dec64_math.c

dec64_string.o: dec64_string.c dec64_string.h dec64.o
	clang -c dec64_string.c

dec64_test.o: dec64_test.c dec64.o
	clang -c dec64_test.c

dec64_math_test.o: dec64_math_test.c dec64_math.o dec64.o
	clang -c dec64_math_test.c

dec64_string_test.o: dec64_string.c dec64_string.o dec64.o
	clang -c dec64_string_test.c

dec64_test: dec64_test.o
	clang -o dec64_test dec64.o dec64_test.o

dec64_math_test: dec64_math_test.o dec64_string.o
	clang -o dec64_math_test dec64.o dec64_math.o dec64_string.o dec64_math_test.o

dec64_string_test: dec64_string_test.o
	clang -o dec64_string_test dec64.o dec64_string.o dec64_string_test.o

test: dec64_test
	./dec64_test

test_string: dec64_string_test
	./dec64_string_test

test_math: dec64_math_test
	./dec64_math_test

test_all: test test_string test_math

clean :
	rm dec64_test dec64_math_test dec64_string_test *.o
