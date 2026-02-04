# Makefile per LZSR Data Archiver
# Creato sulla base del progetto Code::Blocks caricato

CXX = g++

# Rilevamento automatico del sistema operativo
ifeq ($(OS),Windows_NT)
    TARGET = LZSR.exe
    # Aggiunge le librerie statiche per far girare l'exe ovunque su Windows
    LDFLAGS = -static-libgcc -static-libstdc++ -s
    CLEAN_CMD = del /f /q $(TARGET) *.o
else
    TARGET = LZSR
    LDFLAGS = -s
    CLEAN_CMD = rm -f $(TARGET) *.o
endif

# Opzioni del compilatore:
# -O3: Massima velocità (migliore del -O2 di CodeBlocks per un codec)
# -march=x86-64-v2: Compatibilità CPU ultimi 10 anni
# -Wall: Mostra avvisi utili
CXXFLAGS = -O3 -march=x86-64-v2 -Wall -fexceptions

# File sorgente
SRCS = main.cpp

# Regola principale (Default)
all: $(TARGET)

$(TARGET): main.o
	$(CXX) $(CXXFLAGS) -o $(TARGET) main.o $(LDFLAGS)

main.o: main.cpp Analize.h Archive.h BitEncoder.h Common.h DirectCoder.h Engine.h FileOp.h Filters.h
	$(CXX) $(CXXFLAGS) -c main.cpp -o main.o

# Pulizia file temporanei
clean:
	$(CLEAN_CMD)