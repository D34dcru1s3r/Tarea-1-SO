# Compilador y flags
CC = gcc
CFLAGS = -Wall -Wextra -g
OBJ = src/main.o src/parser.o src/exec.o src/miprof.o

# Nombre del ejecutable
TARGET = mishell

# Regla principal
all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $(OBJ)

# Compilar cada .c a .o
src/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Limpiar archivos generados
clean:
	rm -f src/*.o $(TARGET)

# Limpiar también binarios y resultados
distclean: clean
	rm -f resultados.txt
