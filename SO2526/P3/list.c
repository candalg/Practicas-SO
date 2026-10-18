/*
* Participantes:
 * Joel Candal Gómez
 * Ou Zhang
 * Grupo: 2.3
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "list.h"



void CreateEmptyList(tList *L) {
    *L = NULL;
}

int insertItem(tList *L, void *data) {
    if (*L == NULL) {
        *L = malloc(sizeof(struct Node));
        if (*L == NULL) return -1;
        (*L)->data = data;
        (*L)->next = NULL;
    } else {
        tList actual = *L;
        while (actual->next != NULL) {
            actual = actual->next;
        }

        actual->next = malloc(sizeof(struct Node));
        if (actual->next == NULL) return -1;
        actual->next->data = data;
        actual->next->next = NULL;
    }

    return 0;
}

tPosL prev(tList L, tPosL p) {
    if (L == NULL || p == L) return NULL;

    tList actual = L;
    while (actual->next != NULL && actual->next != p) {
        actual = actual->next;
    }

    return actual;
}


void deleteAtPosition(tList *L, tPosL p) {
    if (L == NULL || *L == NULL || p == NULL) {
        return;
    }

    if (p == *L) {
        *L = p->next;
        free(p);
        return;
    }

    tPosL prevNode = prev(*L, p);
    if (prevNode != NULL) {
        prevNode->next = p->next;
        free(p);
    }
}

tPosL first(tList L) {
    return L;
}

tPosL next(tPosL actual) {
    if (actual == NULL) return NULL;
    return actual->next;
}

int listEnd(tPosL p) {
    return p == NULL;
}

void *getItem(tPosL p) {
    if (p == NULL) {
        return NULL;
    }

    return p->data;
}

void clear(tList *L) {
    tList actual = *L;
    tList next_node;

    while (actual != NULL) {
        next_node = actual->next;
        free(actual->data);
        free(actual);
        actual = next_node;
    }

    *L = NULL;
}

tPosL last(tList L) {
    if (L == NULL) return NULL;

    tList actual = L;
    while (actual->next != NULL) {
        actual = actual->next;
    }

    return actual;
}


int length(tList L) {
    int len = 0;

    tList actual = L;
    while (actual != NULL) {
        len++;
        actual = actual->next;
    }

    return len;
}

bool isEmptyList(tList L) {
    return L == NULL;
}


