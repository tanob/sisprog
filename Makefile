FILES = montador.c montador_io.c
OUT = montador
FLAGS = -Wall -D NIX

all:
	clear
	gcc $(FLAGS) $(FILES) -o $(OUT)	

	

