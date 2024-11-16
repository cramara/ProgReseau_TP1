#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <time.h>

#define TIME_PORT 37
#define TIMEOUT_SECONDS 5

// ./time_client 129.6.15.28 pour recevoir la date et l'heure

int main(int argc, char **argv) {
    int sockfd;
    struct sockaddr_in serv_addr;
    uint32_t time_value;
    time_t unix_time;
    struct timeval timeout;
    
    if (argc != 2) {
        printf("Usage: %s server_ip\n", argv[0]);
        exit(1);
    }

    // Création du socket UDP au lieu de TCP
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("Erreur lors de la création du socket");
        exit(1);
    }

    // Configuration du timeout
    timeout.tv_sec = TIMEOUT_SECONDS;
    timeout.tv_usec = 0;
    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0) {
        perror("Erreur lors de la configuration du timeout");
        exit(1);
    }

    // Configuration de l'adresse du serveur
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(TIME_PORT);
    
    if (inet_pton(AF_INET, argv[1], &serv_addr.sin_addr) <= 0) {
        perror("Erreur avec l'adresse IP");
        exit(1);
    }

    // Envoi d'un datagramme vide pour déclencher la réponse
    if (sendto(sockfd, NULL, 0, 0, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Erreur d'envoi");
        exit(1);
    }

    // Lecture de la réponse (32 bits)
    socklen_t len = sizeof(serv_addr);
    if (recvfrom(sockfd, &time_value, sizeof(time_value), 0, 
                 (struct sockaddr*)&serv_addr, &len) != sizeof(time_value)) {
        perror("Erreur de lecture");
        exit(1);
    }

    // Conversion du format réseau vers le format hôte
    time_value = ntohl(time_value);
    
    // Conversion du temps Time Protocol (depuis 1900) vers Unix time (depuis 1970)
    unix_time = time_value - 2208988800UL;
    
    // Affichage de la date
    printf("Date et heure reçues : %s", ctime(&unix_time));

    close(sockfd);
    return 0;
} 