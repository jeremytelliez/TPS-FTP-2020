Parties optionnelles implémentées :
- [x] Mode Debug
- [x] Mode Actif / Passif
- [x] Gestion fichiers locaux
- [x] Gestion fichiers distants
- [x] Gestion répertoire distants


Pour la partie optionnelle de Gestion des répertoires distants,
pour la suppression d'un répertoire, si le répertoire n'est pas vide,
il n'est pas supprimé et le serveur nous renvoie la réponse
550 Remove directory operation Failed

Nous aurions pu régler le problème en utilisant notre fonction de
manière récursive mais nous ne l'avons pas fait par manque de temps
