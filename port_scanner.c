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

#define MIN_PORT 1
#define MAX_PORT 1024  // Scanner les 1024 premiers ports
#define TIMEOUT 0.05    // Timeout réduit à 0.05 secondes, pas possible de mettre moins, sinon ça va trop vite

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
    if (argc != 2) {
        printf("Usage: %s <hostname/IP>\n", argv[0]);
        return 1;
    }
    
    printf("Scanning des ports TCP pour %s...\n", argv[1]);
    printf("Ports ouverts :\n");
    
    int open_ports = 0;
    for (int port = MIN_PORT; port <= MAX_PORT; port++) {
        if (check_port(argv[1], port)) {
            printf("Port %d : OUVERT\n", port);
            open_ports++;
        }
    }
    
    printf("\nScan terminé. %d ports ouverts trouvés.\n", open_ports);
    return 0;
} 