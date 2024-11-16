## Comment faire pour tester les différentes fonctionnalités ?

Pour commencer il faut faire un ```make```

Si on voit le message d'erreur "Impossible de faire le bind" , veuillez essayer de relancer la commande avec un autre port

### Exo 2 :  En vous inspirant de l’exemple précédent écrire un client pour le protocole Time (RFC 868). Le client affichera la date.

Lancer le client time : 

```./time_client 129.6.15.28```

Et ça nous renvoi l'heure en utilisant le protocole Time

### Exo 3 : Réécrivez le programme exemple (socket_server et socket_client) de façon à ce que les clients soient traités en parallèle par le serveur.

On commence par lancer le serveur sur le port 9999 : 

```./socket_server_html 9999```

Puis on peut lancer le client_msg en localhost sur le port 9999 :

```./socket_client_msg 127.0.0.1 9999```

On peut maintenant essayer d'envoyer un message et voir qu'il apparaît sur le serveur.  
On peut aussi lancer plusieurs clients en même temps

### Exo 4 : Ecrire un programme qui lit le contenu d’une page web (avec http il suffit d’envoyer la requête GET puis sauter deux lignes donc deux “”). Un défi : dans la page reçue extraire les éléments title.

On peut essayer de récupérer la page de google en faisant : 

```./socket_client_http google.fr 80``` 

Cela nous affiche dans le terminal la page reçu et à la fin son titre (balise ```<title></title>```)

### Exo 5 : Ecrire un programme qui renvoie la liste des ports TCP ouverts sur une machine dont on passera en paramètre, soit le nom (cf. man gethostbyname), soit l’adresse IP.

Pour cela on lance port_scanner en précisant (ou pas) jusqu'à quel port on veut scanner :

```./port_scanner 192.168.1.1 1000``` 

Pour scanner jusqu'au port 1000, si on ne met rien on scanne jusqu'au port 1024 par défaut

### Exo Bonus : Faire un serveur qui renvoie  une page html quand on lui envoie une requête GET

On lance server_html sur le port 9999: 
 
 ```./socket_server_html 9999```

Puis on peut soit ouvrir dans notre explorateur : ```localhost:9999```

Et on verra apparaître la page html avec les consignes du TP

Sinon on lance socket_client_http, qui lui va envoyer une requête GET au serveur : 

```./socket_client_http 127.0.0.1 9999``` 

La page sera alors affichée dans le termial ainsi que son titre : "Requête bien reçue"


