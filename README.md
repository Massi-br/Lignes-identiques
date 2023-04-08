# Lignes-identiques
Généralités
Un projet consiste à écrire un programme en C dont le but est :
  — si le nom d’un seul fichier figure sur la ligne de commande, d’afficher, pour chaque ligne
de texte non vide apparaissant au moins deux fois dans le fichier, les numéros des lignes où elle
apparait et le contenu de la ligne ;
  — si au moins deux noms de fichiers figurent sur la ligne de commande, d’afficher, pour chaque
ligne de texte non vide apparaissant au moins une fois dans tous les fichiers, le nombre d’occurrences
de la ligne dans chacun des fichiers et le contenu de la ligne.
L’affichage se fait en colonnes sur la sortie standard. Les colonnes sont (uniquement) séparées
par le caractère de tabulation horizontale ’\t’ ; les lignes sont (uniquement) terminées par le
caractère de fin de ligne ’\n’. La première ligne affichée reprend les noms des fichiers. Pour les
lignes suivantes :
  — dans le premier cas, l’affichage se fait sur deux colonnes. La première colonne est réservée
à la suite des numéros de ligne, la deuxième, au contenu. Les numéros de lignes sont (uniquement)
séparés par une virgule ;
  — dans le deuxième cas, l’affichage se fait sur une colonne de plus que le nombre de noms
fichiers : figurent dans les premières colonnes les nombres d’occurrences, dans la dernière, le contenu.
