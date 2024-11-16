# Compilateur et options
CC = gcc
CFLAGS = -Wall -Wextra -g
LDFLAGS = 

# Dossier pour les exécutables
BIN_DIR = bin

# Création du dossier bin si il n'existe pas
$(shell mkdir -p $(BIN_DIR))

# Trouver les fichiers sources existants
SRCS = $(wildcard *.c)
TARGETS = $(basename $(SRCS))

# Cible principale
all: $(TARGETS:%=$(BIN_DIR)/%)

# Règle de compilation
$(BIN_DIR)/%: %.c
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

# Nettoyage
clean:
	rm -rf $(BIN_DIR)

# Pour le debug
debug: CFLAGS += -DDEBUG -g
debug: all

# Règle .PHONY pour les cibles qui ne sont pas des fichiers
.PHONY: all clean debug