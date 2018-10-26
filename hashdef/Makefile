CC = gcc
CFLAGS = -g -std=c99 -Wall -Wconversion -Wno-sign-conversion -Werror 
VFLAGS = --leak-check=full --track-origins=yes --show-reachable=yes

CFILES = testing.c hash.c pruebas_catedra.c main.c murmurhash.c
HFILES = testing.h hash.h murmurhash.h
EXEC_S = main

build: $(CFILES)
	$(CC) $(CFLAGS) -o $(EXEC_S) $(CFILES)

run: build
	./$(EXEC_S)

valgrind: build
	valgrind $(VFLAGS) ./$(EXEC_S)

time: build
	./tiempos_volumen.sh ./$(EXEC_S)

gdb: build
	gdb ./$(EXEC_S)

pruebas: hashjulian.c pruebas_volumen.c testing.c 
	$(CC) $(CFLAGS) -o ./pruebas_volumen hashjulian.c testing.c pruebas_volumen.c
	
CFILES_G = testing.c hashguido.c pruebas_catedra.c main.c
HFILES_G = testing.h hash.h
EXEC_S_G = pruebasguido

build_g: $(CFILES_G)
	$(CC) $(CFLAGS) -o $(EXEC_S_G) $(CFILES_G)

run_g: build_g
	./$(EXEC_S_G)

valgrind_g: build_g
	valgrind $(VFLAGS) ./$(EXEC_S_G)

time_g: build_g
	./tiempos_volumen.sh ./$(EXEC_S_G)

CFILES_J = testing.c hashjulian.c pruebas_catedra.c main.c
HFILES_J = testing.h hash.h
EXEC_S_J = pruebasjulian

build_j: $(CFILES_J)
	$(CC) $(CFLAGS) -o $(EXEC_S_J) $(CFILES_J)

run_j: build_j
	./$(EXEC_S_J)

valgrind_j: build_j
	valgrind $(VFLAGS) ./$(EXEC_S_J)

time_j: build_j
	./tiempos_volumen.sh ./$(EXEC_S_J)
