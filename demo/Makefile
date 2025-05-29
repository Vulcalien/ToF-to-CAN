OUT := demo

CPPFLAGS := -I../include
CFLAGS   := -Wall -pedantic

.PHONY: all run build

all: run

run: build
	./$(OUT)

build:
	gcc $(CPPFLAGS) $(CFLAGS) demo.c -o $(OUT)

clean:
	rm $(OUT)
