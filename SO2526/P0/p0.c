
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <sys/utsname.h>
#include <fcntl.h>
#include <stdbool.h>

#include "list.h"


#define MAX_INPUT 1024
#define MAX_TOKENS 256

void cmd_authors(char* tokens[]);
void cmd_open(char* tokens[], tList* ficherosAbiertos);
void cmd_listopen(tList* ficherosAbiertos);
void cmd_getpid(char* tokens[]);
void cmd_getcwd(char* tokens[]);
void cmd_chdir(char* tokens[]);
void cmd_date(char* tokens[]);
void cmd_infosys(char* tokens[]);
void cmd_exit(char* tokens[]);
void cmd_historic(char* tokens[], tList* historial);
void cmd_help(char* tokens[]);
void cmd_close(char* tokens[], tList* ficherosAbiertos);
void cmd_dup(char *tokens[], tList* ficherosAbiertos);
void cmd_handler(char *entrada, tList *historial, tList *ficherosAbiertos);
void leerEntrada(char* entrada);
int TrocearCadena(char* cadena, char* trozos[]);


int TrocearCadena(char *cadena, char *trozos[]) {
    int i = 1;
    if ((trozos[0] = strtok(cadena, " \n\t")) == NULL)
        return 0;
    while ((trozos[i] = strtok(NULL, " \n\t")) != NULL)
        i++;
    return i;
}

void leerEntrada(char* entrada){
    fgets(entrada, MAX_TOKENS, stdin);
}

void cmd_authors(char* tokens[]) {
    const char *nombres[] = {"Joel Candal Gómez", "Ou Zhang"};
    const char *logins[] = {"joel.candal", "ou.zhang"};
    if (tokens[1] != NULL && strcmp(tokens[1], "-n") == 0) {
        for (int i = 0; i < 2; i++) printf("%s\n", nombres[i]);
    } else if (tokens[1] != NULL && strcmp(tokens[1], "-l") == 0) {
        for (int i = 0; i < 2; i++) printf("%s\n", logins[i]);
    } else {
        for (int i = 0; i < 2; i++) printf("%s: %s\n", nombres[i], logins[i]);
    }
}

void cmd_getpid(char* tokens[]) {
    if (tokens[1] != NULL && strcmp(tokens[1], "-p") == 0) {
        printf("Pid del padre del shell: %d\n", getppid());
    } else {
        printf("Pid de shell: %d\n", getpid());
    }
}

void cmd_getcwd(char* tokens[]) {
    char cwd[1025];
    if (getcwd(cwd, sizeof(cwd)) != NULL) printf("%s\n", cwd);
    else perror("getcwd()");
}

void cmd_chdir(char *tokens[]) {
    if (tokens[1] == NULL) {
        char *cwd = getcwd(NULL, 0);
        if (cwd == NULL) { perror("getcwd"); return; }
        printf("%s\n", cwd);
        free(cwd);
    } else {
        if (chdir(tokens[1]) != 0) perror("chdir");
    }
}

void cmd_date(char* tokens[]) {
    time_t now;
    struct tm *local;
    char buffer[101];
    time(&now);
    local = localtime(&now);
    if (tokens[1] != NULL && strcmp(tokens[1], "-d") == 0) {
        strftime(buffer, sizeof(buffer), "%d/%m/%Y", local);
        printf("%s\n", buffer);
    } else if (tokens[1] != NULL && strcmp(tokens[1], "-t") == 0) {
        strftime(buffer, sizeof(buffer), "%H:%M:%S", local);
        printf("%s\n", buffer);
    } else {
        char buffer_date[51], buffer_time[50];
        strftime(buffer_date, sizeof(buffer_date), "%d/%m/%Y", local);
        strftime(buffer_time, sizeof(buffer_time), "%H:%M:%S", local);
        printf("%s %s\n", buffer_date, buffer_time);
    }
}

void cmd_help(char *tokens[]) {
    if (tokens[1] == NULL) {
        printf("Comandos disponibles:\nauthors getpid chdir getcwd date historic open close dup listopen infosys help quit exit bye\n");
    } else {
        if (strcmp(tokens[1], "authors") == 0) {
            printf("authors [-n|-l]	Muestra los nombres y/o logins de los autores\n");
        } else if (strcmp(tokens[1], "getpid") == 0) {
            printf("getpid [-p]	Muestra el pid del shell o de su proceso padre\n");
        } else if (strcmp(tokens[1], "chdir") == 0) {
            printf("chdir [dir]	Cambia (o muestra) el directorio actual del shell\n");
        } else if (strcmp(tokens[1], "getcwd") == 0) {
            printf("getcwd 	Muestra el directorio actual del shell\n");
        } else if (strcmp(tokens[1], "date") == 0) {
            printf("date [-d|-t]	Muestra la fecha y/o la hora actual\n");
        } else if (strcmp(tokens[1], "historic") == 0) {
            printf("historic [-clear|-count|-N|N]	Muestra (o borra)el historico de comandos\n\t-clear: borra el historico\n\t-count: dimension del historico\n\t-N: muestra los N ultimos\n\tN: repite el comando N\n");
        } else if (strcmp(tokens[1], "open") == 0) {
            printf("open fich m1 m2...	Abre el fichero fich y lo anade a la lista de ficheros abiertos del shell\n\tm1, m2.. son modos de apertura (cr, ex, ro, rw, wo, ap, tr)\n");
        } else if (strcmp(tokens[1], "close") == 0) {
            printf("close df	Cierra el dfriptor df y elimina el correspondiente fichero de la lista de ficheros abiertos\n");
        } else if (strcmp(tokens[1], "dup") == 0) {
            printf("dup df	Duplica el dfriptor de fichero df y anade una nueva entrada a la lista ficheros abiertos\n");
        } else if (strcmp(tokens[1], "listopen") == 0) {
            printf("listopen [n]	Lista los ficheros abiertos del shell\n");
        } else if (strcmp(tokens[1], "infosys") == 0) {
            printf("infosys 	Muestra informacion de la maquina donde corre el shell\n");
        } else if (strcmp(tokens[1], "help") == 0) {
            printf("help [cmd|-lt|-T|-all]	Muestra ayuda sobre los comandos\n");
        } else if (strcmp(tokens[1], "quit") == 0 || strcmp(tokens[1], "exit") == 0 || strcmp(tokens[1], "bye") == 0) {
            printf("Termina la ejecución del shell\n");
        } else {
            printf("%s no encontrado\n", tokens[1]);
        }
    }
}

void cmd_infosys(char* tokens[]) {
    struct utsname info;
    if (uname(&info) != 0) perror("uname");
    else printf("%s@%s %s, OS: %s-%s-%s\n", info.sysname, info.nodename, info.machine, info.sysname, info.release, info.version);
}

void cmd_exit(char* tokens[]){
    exit(0);
}

void cmd_historic(char* tokens[], tList* historial) {
    if (tokens[1] == NULL) {
        int i = 0;
        for (tPosL p = first(*historial); !listEnd(p); p = next(p)) {
            char* command = (char*)getItem(p);
            printf("%d: %s", i, command);
            i++;
        }
    } else if (strcmp(tokens[1], "-count") == 0) {
        int total = 0;
        for (tPosL p = first(*historial); !listEnd(p); p = next(p)) total++;
        printf("Número de comandos en el historial: %d\n", total);
    } else if (strcmp(tokens[1], "-clear") == 0) {
        while(first(*historial) != NULL) deleteAtPosition(historial, first(*historial));
        printf("Historial borrado.\n");
    } else if (tokens[1][0] == '-') {
        int n = atoi(tokens[1] + 1), total = 0, start;
        for (tPosL p = first(*historial); !listEnd(p); p = next(p)) total++;
        start = (n < total) ? total - n : 0;
        int i = 0;
        for (tPosL p = first(*historial); !listEnd(p); p = next(p)) {
            if (i >= start) printf("%d: %s\n", i, (char*)getItem(p));
            i++;
        }
    } else {
        int index = atoi(tokens[1]), i = 0;
        for (tPosL p = first(*historial); !listEnd(p); p = next(p)) {
            if (i == index) { printf("%d: %s", index, (char*)getItem(p)); break; }
            i++;
        }
    }
}

int addHistorial(char* entradaOriginal, tList* historial) {
    insertItem(historial, entradaOriginal);
    return 0;
}

void cmd_open(char* tr[], tList* ficherosAbiertos) {
    int i, mode = 0, df;
    if (tr[1] == NULL) {
        cmd_listopen(ficherosAbiertos); 
        return; 
    }
    for(i = 2; tr[i] != NULL; i++) {
        if (!strcmp(tr[i], "cr")) mode |= O_CREAT;
        else if (!strcmp(tr[i], "ex")) mode |= O_EXCL;
        else if (!strcmp(tr[i], "ro")) mode |= O_RDONLY;
        else if (!strcmp(tr[i], "wo")) mode |= O_WRONLY;
        else if (!strcmp(tr[i], "rw")) mode |= O_RDWR;
        else if (!strcmp(tr[i], "ap")) mode |= O_APPEND;
        else if (!strcmp(tr[i], "tr")) mode |= O_TRUNC;
        else break;
    }
     
    if ((df=open(tr[1],mode,0777))== -1){
        perror ("No se puede abrir fichero");
    } else {
        tOpenFile* f = malloc(sizeof(tOpenFile));
        f -> df = df;
        strncpy(f -> nombre, tr[1], 255);
        f-> nombre[255] = '\0';
        f -> modo = mode;
        insertItem(ficherosAbiertos, f);
        printf("Archivo '%s' abierto con dfriptor %d y modo %d\n", f->nombre, f->df, f->modo);
    }
}

void cmd_listopen(tList* ficherosAbiertos) {
    tPosL p = first(*ficherosAbiertos);
    while(!listEnd(p)) {
        tOpenFile* f = (tOpenFile*)getItem(p);
        printf("df: %d\t%s\tmodo: %d\n", f->df, f->nombre, f->modo);
        p = next(p);
    }
}

tPosL findDf(int df, tList* ficherosAbiertos) {
    for (tPosL p = first(*ficherosAbiertos); !listEnd(p); p = next(p)) {
        tOpenFile* f = (tOpenFile*) getItem(p);
        if (f != NULL && f->df == df) return p;
    }
    return NULL;
}

void cmd_close (char *tokens[], tList* ficherosAbiertos)
{
    int df;

    if (tokens[1] == NULL || (df = atoi(tokens[1])) < 0) {
        cmd_listopen(ficherosAbiertos);
        return;
    }

    if (close(df) == -1) {
        perror("Imposible cerrar descriptor");
    } else {
        tPosL p = findDf(df, ficherosAbiertos);
        if (p != NULL) {
            tOpenFile* f = (tOpenFile*) getItem(p);
            printf("Descriptor %d cerrado correctamente (%s)\n", df, f->nombre);
            deleteAtPosition(ficherosAbiertos, p);
            free(f);
        } else {
            printf("Descriptor %d no encontrado en la lista de ficheros abiertos.\n", df);
        }
    }
}

void cmd_dup(char *tokens[], tList* ficherosAbiertos) {
    int df, duplicado;
    char aux[256], *p;

    if (tokens[1] == NULL || (df = atoi(tokens[1])) < 0) {
        cmd_listopen(ficherosAbiertos);
        return;
    }

    tPosL pos = findDf(df, ficherosAbiertos);
    if (pos == NULL) {
        fprintf(stderr, "Descriptor %d no encontrado en la lista de ficheros abiertos.\n", df);
        return;
    }
    tOpenFile* of = (tOpenFile*) getItem(pos);
    p = of->nombre;

    duplicado = dup(df);
    if (duplicado == -1) {
        perror("Imposible duplicar descriptor");
        return;
    }

    snprintf(aux, sizeof(aux), "dup %d (%.*s)", df, (int)(sizeof(aux) - 20), p);


    tOpenFile* nf = (tOpenFile*) malloc(sizeof(tOpenFile));
    if (nf == NULL) {
        close(duplicado);
        fprintf(stderr, "Error de memoria al crear la nueva entrada\n");
        return;
    }
    nf->df = duplicado;
    strncpy(nf->nombre, aux, 255);
    nf->nombre[255] = '\0';
    nf->modo = fcntl(duplicado, F_GETFL);
    insertItem(ficherosAbiertos, nf);
    printf("Descriptor %d duplicado como %d (%s)\n", df, duplicado, nf->nombre);
}


void cmd_handler(char* entrada, tList* historial, tList* ficherosAbiertos){

    char* tokens[64];
    char* entradaOriginal = strdup(entrada);

    TrocearCadena(entrada, tokens);

    if (tokens[0] != NULL) {


        if (strcmp(tokens[0], "getpid" )== 0){
            cmd_getpid(tokens);
        } else if (strcmp(tokens[0], "authors") == 0){
            cmd_authors(tokens);
        } else if (strcmp(tokens[0], "infosys") == 0){
            cmd_infosys(tokens);
        } else if (strcmp(tokens[0], "historic") ==0) {
            cmd_historic(tokens, historial);
        } else if (strcmp(tokens[0], "chdir")== 0){
            cmd_chdir(tokens);
        } else if (strcmp(tokens[0], "date")== 0){
            cmd_date(tokens);
        } else if (strcmp(tokens[0], "help") == 0){
            cmd_help(tokens);
        } else if (strcmp(tokens[0], "open") == 0){
            cmd_open(tokens, ficherosAbiertos);
        } else if (strcmp(tokens[0], "listopen") == 0){
            cmd_listopen(ficherosAbiertos);
        } else if (strcmp(tokens[0], "close") == 0) {
            cmd_close(tokens, ficherosAbiertos);
        } else if (strcmp(tokens[0], "dup") == 0) {
            cmd_dup(tokens, ficherosAbiertos);
        } else if (strcmp(tokens[0], "getcwd") == 0) {
            cmd_getcwd(tokens);
        }
        addHistorial(entradaOriginal, historial);

    if (strcmp(tokens[0], "quit") == 0 || strcmp(tokens[0], "exit") == 0 || strcmp(tokens[0], "bye") == 0){
        clear(ficherosAbiertos);
        clear(historial);
        exit(0);
    }

    }
}


int main(){

    tList ficherosAbiertos;
    tList historial;

    CreateEmptyList(&ficherosAbiertos);
    CreateEmptyList(&historial);

    char entrada[MAX_TOKENS];

    while (true){
       printf("->");
       leerEntrada(entrada);
       cmd_handler(entrada, &historial, &ficherosAbiertos);
    }
}



