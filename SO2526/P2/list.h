/*
* Participantes:
 * Joel Candal Gómez
 * Ou Zhang
 * Grupo: 2.3
 */

#ifndef SO_2526_LIST_HISTORIC_H
#define SO_2526_LIST_HISTORIC_H

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

struct Node {
    void *data;
    struct Node *next;
};


typedef struct {
    int df;
    char nombre[256];
    int modo;
} tOpenFile;

typedef enum { NREC = 0, RECA = 1, RECB = 2 } tRecurse;

typedef struct {
    bool long_format;   //true = long, false = short
    bool list_links;    //true = list symbolic link target (link), false = nolink
    bool show_hidden;   // true = hid, false = nohid
    tRecurse recurse;   // reca, recb, norec
} tDirParams;

typedef enum { MALLOC, SHARED, MMAP } alloc_type;

typedef struct{
    void *addr;                 // Dirección devuelta por malloc
    size_t size;                // Tamaño pedido
    time_t allocationTime;           // Momento de asignación
    alloc_type type;            // Tipo de bloque → MALLOC en nuestro caso
    int key;             // Clave de memoria compartida (para shared)   
    char* filename;           // Nombre de fichero (para mmap)
    int fd;                   // Descriptor de fichero (para mmap)
}Block_Memory;

typedef struct Node *tList;
typedef struct Node *tPosL;


void CreateEmptyList(tList *);
int insertItem(tList *, void *);
void deleteAtPosition(tList *L, tPosL pos);
tPosL first(tList);
tPosL next(tPosL);
int listEnd(tPosL);
void *getItem(tPosL);
void clear(tList *);
tPosL last(tList);
tPosL prev(tList, tPosL);
int length(tList);


#endif


