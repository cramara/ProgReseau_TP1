#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#define BUFFER_SIZE 200000

// Fonction pour extraire le contenu de la balise <title>
void extract_title(char *response) {
  char *start = strstr(response, "<title>");
  char *end = strstr(response, "</title>");

  if (start != NULL && end != NULL && start < end) {
    start += 7;  // Avancer après <title>
    *end = '\0'; // Terminer la chaîne avant </title>
    printf("Title: %s\n", start);
  } else {
    printf("Aucun titre trouvé\n");
  }
}

int main(int argc, char **argv) {
  int sockfd;
  char buffer[BUFFER_SIZE];
  struct sockaddr_in serv_addr;
  struct hostent *server;

  if (argc != 3) {
    printf("Usage: socket_client <server_hostname> <port>\n");
    exit(0);
  }

  printf("Client starting\n");

  // Résoudre le nom d'hôte en adresse IP
  server = gethostbyname(argv[1]);
  if (server == NULL) {
    printf("Erreur : impossible de résoudre l'hôte\n");
    exit(0);
  }

  // Initialiser la structure de données
  bzero((char *)&serv_addr, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr,
        server->h_length);                   // IP du serveur résolue
  serv_addr.sin_port = htons(atoi(argv[2])); // Port du serveur

  // Ouvrir le socket
  if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    printf("Socket error\n");
    exit(0);
  }

  // Effectuer la connexion
  if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
    perror("Connect error");
    close(sockfd);
    exit(1);
  }

  printf("Connexion établie avec le serveur.\n");

  // Préparer et envoyer la requête HTTP
  char request[] = "GET / HTTP/1.1\r\n"
                   "Host: www.google.fr\r\n"
                   "Connection: close\r\n"
                   "\r\n";

  // Envoyer la requête
  if (write(sockfd, request, strlen(request)) < 0) {
    perror("Erreur lors de l'envoi de la requête");
    close(sockfd);
    exit(1);
  }

  printf("Requête envoyée :\n%s\n", request);

  // Lire la réponse du serveur
  int n;
  char response[BUFFER_SIZE * 10] = {0}; // Stocker toute la réponse
  int offset = 0;

  while ((n = read(sockfd, buffer, BUFFER_SIZE - 1)) > 0) {
    buffer[n] = '\0';         // Terminer la chaîne pour l'affichage
    strcat(response, buffer); // Ajouter le buffer à la réponse complète
    offset += n;
  }

  if (n < 0) {
    perror("Erreur de lecture");
  }
  // Afficher la réponse complète
  printf("Réponse reçue :\n%s\n", response);

  // Extraire et afficher le titre
  extract_title(response);

  printf("Connexion fermée.\n");
  close(sockfd);

  return 0;
}
