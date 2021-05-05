#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "liste.h"


void debug(char* message){
    #ifdef DEBUG
    printf("%s\n", message);
    #endif
}

cell* l_init_cell(size_t size_element){
    size_t total_size = size_element + sizeof(cell*)*2;
    cell* c = malloc(total_size);
    if(c==NULL){ //on kill le processus
        perror("malloc : ");
        exit(-1);
    }
    c->next = NULL;
    c->prev = NULL;
    memset(&(c->data), 0, size_element); //on remplit l'element de 0 pour éviter un use after-free
    return c;
}

list* l_init(size_t size_element){
    list* l = malloc(sizeof(list));
    if(l==NULL){ //on kill le processus
        perror("malloc : ");
        exit(-1);
    }
    l->size = 0;
    l->head = NULL;
    l->end = NULL;
    l->cache_index = 0;
    l->sizeofelement = size_element;
    return l;
}

cell* l_add(list* l, void* data){
    cell* new = l_init_cell(l->sizeofelement);
    if(l->head==NULL){
        l->head = new;
    }
    else{
        l->end->next = new;
        new->prev = l->end;
    }
    l->end = new;
    l->size+=1;
    memcpy(&(new->data), data, l->sizeofelement);
    return new;
}

cell* l_get(list* l, size_t index){
    if(index >= l->size)
        return NULL;
    size_t s1 = index;
    size_t s2 = l->size - index;
    size_t s3 = abs((int)l->cache_index - (int)index);
    cell* c;
    if(s1 <= s2 && s1 <= s3){
        debug("from beg");
        c = l->head;
        for(int i=0; i<index; i++)
            c = c->next;
    }
    else if(s2<=s3 || l->cache_index==0){
        debug("from end");
        c = l->end;
        for(int i=l->size-1; i>index; i--)
            c = c->prev;
    }
    else{
        debug("from cache");
        c = l->cache;
        if(index > l->cache_index){
            debug("cache < ");
            for(int i=l->cache_index; i<index; i++)
                c = c->next;
        }
        else if(index < l->cache_index){
            debug("cache > ");
            for(int i=l->cache_index; i>index; i--)
                c = c->prev;
        }
        // sinon cache = cellule cherchee
    }
    l->cache_index = index;
    l->cache = c;
    return c;
}

char l_remove(list* l, size_t index){
    if(index >= l->size)
        return 0;
    cell* c = l_get(l, index);
    if(index==0){
        l->head = c->next;
    }
    else if(index == l->size-1){
        l->end = c->prev;
    }
    else{
        c->prev->next = c->next;
        c->next->prev = c-> prev;
    }
    if(l->cache == c){
        l->cache = NULL;
        l->cache_index = 0;
    }
    free(c);
    l->size-=1;
    return 1;
}

cell* l_find(list* l, size_t* index, void* data){
    cell* c = l->head;
    size_t i=0;
    for(; i<l->size && memcmp(&c->data, data, l->sizeofelement)!=0; i++){
        c = c->next;
    }
    if(memcmp(&c->data, data, l->sizeofelement)==0){
        *index = i;
    }
    else{
        c = NULL;
    }
    return c;
}

char l_store(list* l, char* filename){
    FILE* f = fopen(filename, "w");
    if(!f){
        perror("open : ");
        return -1;
    }
    // on ecrit d'abord la taille de la liste, puis la taille d'un element, puis le contenu
    size_t buffer_size = sizeof(l->size) + sizeof(l->sizeofelement) + l->size*l->sizeofelement;
    char* buffer = (char*) malloc(sizeof(buffer_size));
    if(!buffer){
        perror("malloc : ");
        exit(-1);
    }
    char* buffer_ptr = buffer;
    memcpy(buffer_ptr, &l->size, sizeof(l->size));
    buffer_ptr += sizeof(l->size);
    memcpy(buffer_ptr, &l->sizeofelement, sizeof(l->sizeofelement));
    buffer_ptr += sizeof(l->sizeofelement);
    for(int i=0; i<l->size; i++){
        memcpy(buffer_ptr, &l_get(l, i)->data, l->sizeofelement);
        buffer_ptr += l->sizeofelement;
    }
    if(fwrite(buffer, buffer_size, 1, f)!=1){
        perror("write: ");
        free(buffer);
        return -1;
    }
    if(fclose(f)){
        perror("close: ");
        free(buffer);
        return -1;
    }
    free(buffer);
    return 0;
}
list* l_load(char* filename){
    printf("load\n");
    FILE* f = fopen(filename, "r");
    if(!f){
        perror("open : ");
        return NULL;
    }
    size_t size_list;
    size_t size_element;
    if(fread(&size_list, sizeof(size_t), 1, f) != 1 || \
        fread(&size_element, sizeof(size_t), 1, f)!=1){
        perror("read: ");
        return NULL;
    }
    printf("liste size = %d, size_el = %d\n", size_list, size_element);
    void* buffer = malloc(size_element);
    if(!buffer){
        perror("malloc : ");
        exit(-1);
    }
    list* l = l_init(size_element);
    if(!l){
        return NULL;
    }
    for(int i=0; i<size_list; i++){
        printf("%d\n", i);
        if(fread(buffer, size_element, 1, f)!=1){
            perror("read: ");
            free(buffer);
            return NULL;
        }
        l_add(l, buffer);
    }
    if(fclose(f)){
        perror("close: ");
        return NULL;
    }
    return l;
}
void l_free(list* l){
    while(l->size)
        l_remove(l, 0);
    free(l);
}

#ifdef DEBUG
int main(){
    list* l = l_init(sizeof(int));
    for(int i=0; i<100; i++){
        l_add(l, &i);
    }
    cell* c;
    c = l_get(l, 0);
    printf("value = %d\n", (c->data));
    c = l_get(l, 50);
    printf("value = %d\n", (c->data));
    c = l_get(l, 51);
    printf("value = %d\n", (c->data));
    c = l_get(l, 50);
    printf("value = %d\n", (c->data));
    c = l_get(l, 99);
    printf("value = %d\n", (c->data));

    l_remove(l, 0);
    c = l_get(l, 0);
    printf("value = %d\n", (c->data));

    l_remove(l, l->size-1);
    c = l_get(l, l->size-1);
    printf("value = %d\n", (c->data));

    c = l_get(l, 2);
    printf("before delete value = %d\n", (c->data));
    l_remove(l, 2);
    c = l_get(l, 2);
    printf("after delete value = %d\n", (c->data));

    int searched = 20;
    size_t index;
    c = l_find(l, &index, &searched);
    printf("after find value = %d\n", (c->data));

    l_store(l, "file");
    list* l2 = l_load("file");
    for(int i=0; i<l2->size; i++){
        c = l_get(l2, i);
        printf("l2[%d] = %d\n", i, c->data);
    }
    free(l);
    free(l2);
    return 0;
}
#endif