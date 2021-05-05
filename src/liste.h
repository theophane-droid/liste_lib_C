/* On implemente ici une liste doublement chainee avec 
un mécanisme de cache pour optimiser le temps de parcours
Cette liste a aussi la particularité de pouvoir stocker n'importe quel type de donnée */
#ifndef LISTE_H
#define LISTE_H
typedef struct cell{
    struct cell* prev;
    struct cell* next;
    void* data;
} cell;

typedef struct list{
    cell* head;
    cell* cache;
    size_t cache_index;
    cell* end;
    size_t size;
    size_t sizeofelement;
} list;

/* initialisation d'une cellule, en précisant la taille d'un élement stocké*/
cell* l_init_cell(size_t size_element);

/* initialisation d'une liste, en précisant la taille d'un élément */
list* l_init(size_t size_element);

/* ajoute un élément qui contient data, retourne la cellule ajoutée*/
cell* l_add(list* l, void* data);

/* recupère l'element a l'index index */
cell* l_get(list* l, size_t index);

/* supprimer la case a l'index index, retourne 1 si succes */
char l_remove(list* l, size_t index);

/* trouver la case contenant data, retourne NULL si pas trouvée. 
*index contiendra l'index de la position de data */
cell* l_find(list* l, size_t* index, void* data);

/* stocke dans un fichier binaire filename, la liste l. retourne 0 si succes */
char l_store(list* l, char* filename);

/* charge depuis un fichier binaire filename une liste retournee par la fonctione, retourne NULL si echec */
list* l_load(char* filename);

/* libere une liste */
void l_free(list* l);

#endif