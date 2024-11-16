#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>

//192.168.1.1

#define MIN_PORT 1
#define TIMEOUT 1

// Fonction pour vérifier si un port est ouvert
int check_port(const char *host, int port) {
    struct sockaddr_in addr;
    int sockfd;
    
    // Créer le socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        return 0;
    }
    
    // Configuration du timeout modifié
    struct timeval tv;
    tv.tv_sec = 0;                  // 0 secondes
    tv.tv_usec = TIMEOUT * 1000000;           // 100,000 microsecondes = 0.1 secondes
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
    
    // Rendre le socket non-bloquant
    fcntl(sockfd, F_SETFL, O_NONBLOCK);
    
    // Configurer l'adresse
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    
    // Convertir le nom d'hôte en adresse IP si nécessaire
    struct hostent *he;
    if (inet_addr(host) == INADDR_NONE) {
        // C'est un nom d'hôte
        he = gethostbyname(host);
        if (he == NULL) {
            close(sockfd);
            return 0;
        }
        memcpy(&addr.sin_addr, he->h_addr_list[0], he->h_length);
    } else {
        // C'est une adresse IP
        addr.sin_addr.s_addr = inet_addr(host);
    }
    
    // Tenter la connexion
    int res = connect(sockfd, (struct sockaddr*)&addr, sizeof(addr));
    if (res < 0) {
        if (errno == EINPROGRESS) {
            fd_set fdset;
            FD_ZERO(&fdset);
            FD_SET(sockfd, &fdset);
            
            // Attendre la connexion avec timeout
            if (select(sockfd + 1, NULL, &fdset, NULL, &tv) == 1) {
                int so_error;
                socklen_t len = sizeof(so_error);
                getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &so_error, &len);
                if (so_error == 0) {
                    close(sockfd);
                    return 1;  // Port ouvert
                }
            }
        }
        close(sockfd);
        return 0;  // Port fermé
    }
    
    close(sockfd);
    return 1;  // Port ouvert
}

int main(int argc, char *argv[]) {
    if (argc != 2 && argc != 3) {
        printf("Usage: %s <hostname/IP> [max_port]\n", argv[0]);
        return 1;
    }
    
    // Définir MAX_PORT à partir du paramètre ou utiliser la valeur par défaut
    int max_port = 1024; // Valeur par défaut
    if (argc == 3) {
        max_port = atoi(argv[2]);
        if (max_port <= 0) {
            printf("Le port maximum doit être un nombre positif\n");
            return 1;
        }
    }
    
    printf("Scanning des ports TCP pour %s (jusqu'au port %d)...\n", argv[1], max_port);
    printf("Ports ouverts :\n");
    
    // Ignorer les signaux SIGCHLD pour éviter les processus zombies
    signal(SIGCHLD, SIG_IGN);
    
    // Structure pour stocker les résultats
    int *results = calloc(max_port + 1, sizeof(int));
    if (!results) {
        perror("calloc");
        exit(1);
    }
    
    // Créer un tube pour chaque port
    int (*pipes)[2] = malloc((max_port + 1) * sizeof(*pipes));
    if (!pipes) {
        perror("malloc");
        free(results);
        exit(1);
    }
    
    for (int port = MIN_PORT; port <= max_port; port++) {
        if (pipe(pipes[port]) == -1) {
            perror("pipe");
            free(results);
            free(pipes);
            exit(1);
        }
    }
    
    // Scanner chaque port dans un processus séparé
    for (int port = MIN_PORT; port <= max_port; port++) {
        pid_t pid = fork();
        
        if (pid == -1) {
            perror("fork");
            free(results);
            free(pipes);
            exit(1);
        }
        
        if (pid == 0) { // Processus fils
            // Fermer tous les descripteurs de fichiers inutiles
            for (int p = MIN_PORT; p <= max_port; p++) {
                if (p != port) {
                    close(pipes[p][0]);
                    close(pipes[p][1]);
                }
            }
            
            int result = check_port(argv[1], port);
            write(pipes[port][1], &result, sizeof(result));
            close(pipes[port][1]);
            
            free(results);
            free(pipes);
            exit(0);
        }
    }
    
    // Processus parent : collecter les résultats
    int open_ports = 0;
    for (int port = MIN_PORT; port <= max_port; port++) {
        close(pipes[port][1]); // Fermer l'extrémité d'écriture
        
        int result;
        if (read(pipes[port][0], &result, sizeof(result)) > 0) {
            if (result) {
                printf("Port %d : OUVERT\n", port);
                open_ports++;
            }
        }
        close(pipes[port][0]);
    }
    
    printf("\nScan terminé. %d ports ouverts trouvés.\n", open_ports);
    
    // Libérer la mémoire
    free(results);
    free(pipes);
    return 0;
} 