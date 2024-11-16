#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#define BUFFER_SIZE 1024

//server qui reçoit un GET / et envoie le contenu du fichier html
//ou bien lit le message et l'affiche

int send_file_content(int newsockfd, const char *filepath) {
    FILE *file = fopen(filepath, "r");
    char buffer[BUFFER_SIZE];
    
    if (file == NULL) {
        // Envoyer un message d'erreur si le fichier n'est pas trouvé
        char error_message[] = "HTTP/1.1 404 Not Found\r\n\r\nFile not found!";
        write(newsockfd, error_message, strlen(error_message));
        return -1;
    }

    // Envoyer l'entête HTTP
    char http_header[] = "HTTP/1.1 200 OK\r\n"
                     "Content-Type: text/html; charset=UTF-8\r\n"
                     "\r\n";
    write(newsockfd, http_header, strlen(http_header));

    // Lire et envoyer le contenu du fichier
    while (fgets(buffer, sizeof(buffer), file)) {
        write(newsockfd, buffer, strlen(buffer));
    }

    fclose(file);
    return 0;
}

int main(int argc, char** argv) {
    int sockfd, newsockfd, clilen;
    char buffer[BUFFER_SIZE];
    struct sockaddr_in cli_addr, serv_addr;
    char c;

    if (argc != 2) {
        printf("Usage: socket_server <port>\n");
        exit(0);
    }

    printf("Server starting...\n");

    /* ouverture du socket */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        printf("Impossible d'ouvrir le socket\n");
        exit(0);
    }

    /* initialisation des paramètres */
    bzero((char*)&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(atoi(argv[1]));

    /* effectue le bind */
    if (bind(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("Impossible de faire le bind\n");
        exit(0);
    }

    /* préparation à l'écoute des connexions */
    listen(sockfd, 5);
    clilen = sizeof(cli_addr);
    int pid;

    while (1) {
        newsockfd = accept(sockfd, (struct sockaddr*)&cli_addr, &clilen);

        if (newsockfd < 0) {
            printf("Erreur lors de l'acceptation de la connexion\n");
            continue;
        }
        
        pid = fork();

        if (pid == 0)
        {
          close(sockfd);
          printf("Connexion d'un client\n");

          /* Lire la requête du client */
          bzero(buffer, BUFFER_SIZE);
          read(newsockfd, buffer, BUFFER_SIZE - 1);
          printf("Requête reçue :\n%s\n", buffer);

          /* Si la requête est un GET, envoyer le fichier HTML */
          if (strncmp(buffer, "GET /", 5) == 0) {
              send_file_content(newsockfd, "./../TP1_Sockets_C.html"); //mettre le chemin du fichier html
          }
          else {
            while(read(newsockfd,&c,1) > 0)
            {
              write(1,&c,1);
            }
          }
          close(newsockfd);
          exit(0);
        }
        else {
          close(newsockfd);
        }
    }

    close(sockfd);
    return 0;
}
