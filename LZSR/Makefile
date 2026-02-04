# Makefile per LZSR Data Archiver - Versione Statica e Ottimizzata
# Configurato per GCC 15.2 / C++23

CXX = g++

# Rilevamento automatico del sistema operativo
ifeq ($(OS),Windows_NT)
    TARGET = LZSR.exe
    # Flag del Linker: -static forza l'inclusione di tutte le DLL nell'EXE
    # -flto attiva l'ottimizzazione durante il linking
    LDFLAGS = -flto -static -static-libgcc -static-libstdc++ -s
    CLEAN_CMD = del /f /q $(TARGET) *.o
else
    TARGET = LZSR
    LDFLAGS = -flto -s
    CLEAN_CMD = rm -f $(TARGET) *.o
endif

# Opzioni del compilatore richieste:
# -O4: Massima ottimizzazione (alias di -O3 con flag aggressivi)
# -march=x86-64-v2 -mtune=generic: Bilanciamento tra performance e compatibilità
# -ffast-math: Velocizza i calcoli matematici
# -fpermissive: Tolleranza su codice legacy
# -std=c++23: Ultimo standard del linguaggio
CXXFLAGS = -O4 -march=x86-64-v2 -mtune=generic -ffast-math -fpermissive -std=c++23 -finline-functions -flto -Wall -fexceptions

# File sorgente
SRCS = main.cpp

# Regola principale
all: $(TARGET)

$(TARGET): main.o
	$(CXX) $(CXXFLAGS) -o $(TARGET) main.o $(LDFLAGS)

main.o: main.cpp Analize.h Archive.h BitEncoder.h Common.h DirectCoder.h Engine.h FileOp.h Filters.h
	$(CXX) $(CXXFLAGS) -c main.cpp -o main.o

# Pulizia
clean:
	$(CLEAN_CMD)