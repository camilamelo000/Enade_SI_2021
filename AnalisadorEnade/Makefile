# Makefile para análise ENADE 2021 (versão agregada por curso)

CC = mpicc
CFLAGS = -Wall -O2 -std=c99
TARGET = enade_analysis
SOURCE = enade_analysis.c
PROCESSES = 4
MPIRUN = mpirun

all: $(TARGET)

$(TARGET): $(SOURCE)
	$(CC) $(CFLAGS) -o $(TARGET) $(SOURCE)

run: $(TARGET)
	$(MPIRUN) -np $(PROCESSES) ./$(TARGET)

run-log: $(TARGET)
	$(MPIRUN) -np $(PROCESSES) ./$(TARGET) | tee resultado_enade.txt

clean:
	rm -f $(TARGET) resultado_enade.txt *.o *.obj

.PHONY: all run run-log clean