OUT := demo

CPPFLAGS := -I../include
CFLAGS   := -Wall -pedantic

.PHONY: all run build

all: run

run: build
	./$(OUT)

build: $(OUT)

clean:
	rm -fv $(OUT)

$(OUT): demo.c
	gcc $(CPPFLAGS) $(CFLAGS) demo.c -o $(OUT)
