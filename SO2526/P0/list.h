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

struct Node {
 void *data;
 struct Node *next;
};

typedef struct {
 int df;
 char nombre[256];
 int modo;
} tOpenFile;

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



