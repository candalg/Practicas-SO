
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdbool.h>
#include <fcntl.h>
#include <time.h>
#include <sys/utsname.h>
#include <pwd.h>
#include <grp.h>
#include "list.h"


#define MAX_TOKENS 256

//Parámetros iniciales
tDirParams dirParams = {
    .long_format = false,
    .list_links  = false,
    .show_hidden = false,
    .recurse     = NREC
};

//Funciones auxiliares
int addHistorial(char *entradaOriginal, tList *historial);
char LetraTF(mode_t m);
char *ConvierteModo(mode_t m, char *permisos);
int EsDirectorio(char *dir);
tPosL findDf(int df, tList *ficherosAbiertos);
void borrarRecursivo(const char *path);
const char *get_username(uid_t uid);
const char *get_groupname(gid_t gid);
void print_entry(const char *path, const char *name);
void list_dir(const char *path, int nivel);
void init_standard_fds(tList *ficherosAbiertos);

//Funciones p0
void cmd_authors(char *tokens[]);
void cmd_getpid(char *tokens[]);
void cmd_chdir(char *tokens[]);
void cmd_getcwd();
void cmd_date(char *tokens[]);
void cmd_historic(char *tokens[], tList *historial);
void cmd_open(char *tr[], tList *ficherosAbiertos);
void cmd_close(char *tokens[], tList *ficherosAbiertos);
void cmd_dup(char *tokens[], tList *ficherosAbiertos);
void cmd_listopen(tList *ficherosAbiertos);
void cmd_infosys();
void cmd_help(char *tokens[]);
void cmd_exit();
void cmd_handler(char *entrada, tList *historial, tList *ficherosAbiertos, tDirParams *params);
int TrocearCadena(char *cadena, char *trozos[]);
void leerEntrada(char *entrada);


//Funciones p1
void cmd_create(char *tokens[]);
void cmd_setdirparams(char *tokens[]);
void cmd_getdirparams(char *tokens[]);
void cmd_dir(char *tokens[], tDirParams *params);
void cmd_erase(char *tokens[]);
void cmd_delrec(char *tokens[]);
void cmd_lseek(char *tokens[], tList *ficherosAbiertos);
void cmd_writestr(char *tokens[], tList *ficherosAbiertos);


//FUNCIONES AUXILIARES

// Añade un comando introducido al historial
int addHistorial(char *entradaOriginal, tList *historial){
    // Inserta la cadena (comando original) en la lista 'historial'
    insertItem(historial, entradaOriginal);
    return 0; // Devuelve 0
}

char LetraTF(mode_t m){
    switch (m & S_IFMT){
        case S_IFSOCK: return 's';
        case S_IFLNK: return 'l';
        case S_IFREG: return '-';
        case S_IFBLK: return 'b';
        case S_IFDIR: return 'd';
        case S_IFCHR: return 'c';
        case S_IFIFO: return 'p';
        default: return '?';
    }
}

char *ConvierteModo(mode_t m, char *permisos){
    strcpy(permisos, "---------- ");
    permisos[0] = LetraTF(m);
    if (m & S_IRUSR) permisos[1] = 'r';
    if (m & S_IWUSR) permisos[2] = 'w';
    if (m & S_IXUSR) permisos[3] = 'x';
    if (m & S_IRGRP) permisos[4] = 'r';
    if (m & S_IWGRP) permisos[5] = 'w';
    if (m & S_IXGRP) permisos[6] = 'x';
    if (m & S_IROTH) permisos[7] = 'r';
    if (m & S_IWOTH) permisos[8] = 'w';
    if (m & S_IXOTH) permisos[9] = 'x';
    if (m & S_ISUID) permisos[3] = 's';
    if (m & S_ISGID) permisos[6] = 's';
    if (m & S_ISVTX) permisos[9] = 't';
    return permisos;
}

// Comprueba si una ruta es un directorio
int EsDirectorio(char *dir){
    struct stat s;

    // lstat obtiene información del archivo o directorio indicado
    if (lstat(dir, &s) == -1)
        return 0; // Error → no existe o no se puede acceder

    // Devuelve 1 si el tipo de fichero es un directorio, 0 en caso contrario
    return (S_ISDIR(s.st_mode));
}


tPosL findDf(int df, tList *ficherosAbiertos){
    for (tPosL p = first(*ficherosAbiertos); !listEnd(p); p = next(p)){                                         //Recorre desde el principio hasta el final los ficheros abiertos
        tOpenFile *f = (tOpenFile *)getItem(p);                                                                 //Abre el archivo y consigue la información
        if (f != NULL && f->df == df) return p;                                                                 //Devuelve la posición en caso de que exista el archivo y el descriptor del archivo sea igual al buscado
    }
    return NULL;
}

void borrarRecursivo(const char *path){
    struct stat st;
    if (lstat(path, &st) == -1){ perror(path); return; }
    if (S_ISDIR(st.st_mode)){
        DIR *dir = opendir(path);
        if (!dir){ perror(path); return; }
        struct dirent *entry;
        while ((entry = readdir(dir)) != NULL){
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;
            char fullPath[PATH_MAX];
            snprintf(fullPath, sizeof(fullPath), "%s/%s", path, entry->d_name);
            borrarRecursivo(fullPath);
        }
        closedir(dir);
        if (rmdir(path) == -1) perror(path);
    } else {
        if (unlink(path) == -1) perror(path);
    }
}

// Obtiene el nombre de usuario a partir del UID
const char *get_username(uid_t uid) {
    struct passwd *pw = getpwuid(uid);
    return pw ? pw->pw_name : "?";
}

// Obtiene el nombre del grupo a partir del GID
const char *get_groupname(gid_t gid) {
    struct group *gr = getgrgid(gid);
    return gr ? gr->gr_name : "?";
}

// Muestra la información de un archivo o directorio individual
void print_entry(const char *path, const char *name) {
    struct stat st;
    char fullpath[PATH_MAX]; // Ruta completa (path + nombre)
    char permisos[12];       // Cadena con permisos tipo "drwxr-xr-x"
    char time_str[64];       // Cadena para la fecha/hora
    struct tm tm_info;       // Estructura temporal para formatear la fecha

    // Construye la ruta completa
    snprintf(fullpath, sizeof(fullpath), "%s/%s", path, name);

    // Siempre usamos lstat: así detectamos los enlaces simbólicos correctamente
    if (lstat(fullpath, &st) == -1) {
        perror(fullpath);
        return;
    }

    // Si no se deben mostrar archivos ocultos, se omiten
    if (!dirParams.show_hidden && name[0] == '.')
        return;

    // Formato largo
    if (dirParams.long_format) {
        ConvierteModo(st.st_mode, permisos);
        gmtime_r(&st.st_mtime, &tm_info);
        strftime(time_str, sizeof(time_str), "%Y/%m/%d-%H:%M", &tm_info);

        // Obtener nombres de usuario y grupo
        const char *username = get_username(st.st_uid);
        const char *groupname = get_groupname(st.st_gid);

        printf("%s %4ld (%8ld) %8s %8s %s %8ld %s",
               time_str,
               (long)st.st_nlink,
               (long)st.st_ino,
               username,       // ← Ahora muestra el nombre
               groupname,      // ← Ahora muestra el nombre
               permisos,
               (long)st.st_size,
               name);

        if (S_ISLNK(st.st_mode) && dirParams.list_links) {
            char linkbuf[PATH_MAX];
            ssize_t len = readlink(fullpath, linkbuf, sizeof(linkbuf) - 1);
            if (len >= 0) {
                linkbuf[len] = '\0';
                printf(" -> %s", linkbuf);
            }
        }
        printf("\n");
    }
    // Formato corto
    else {
        printf("%s\t%ld", name, (long)st.st_size);

        // También mostramos el destino si 'link' está activo
        if (S_ISLNK(st.st_mode) && dirParams.list_links) {
            char linkbuf[PATH_MAX];
            ssize_t len = readlink(fullpath, linkbuf, sizeof(linkbuf) - 1);
            if (len >= 0) {
                linkbuf[len] = '\0';
                printf(" -> %s", linkbuf);
            }
        }

        printf("\n");
    }
}


void list_dir(const char *path, int nivel) {
    DIR *dir;
    struct dirent *entry;

    // Intentar abrir el directorio
    if (!(dir = opendir(path))) {
        perror(path);
        return;
    }

    // RECB o NOREC: listar antes de recorrer
    if (dirParams.recurse == RECB || dirParams.recurse == NREC) {
        printf("\n%s:\n", path);
        while ((entry = readdir(dir)) != NULL) {
            if (!dirParams.show_hidden && entry->d_name[0] == '.')
                continue;
            print_entry(path, entry->d_name);
        }
    }
    closedir(dir);

    // Recursión
    if (dirParams.recurse != NREC) {
        dir = opendir(path);
        if (!dir) return;

        while ((entry = readdir(dir)) != NULL) {
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
                continue;
            if (!dirParams.show_hidden && entry->d_name[0] == '.')
                continue;

            char subpath[PATH_MAX];
            snprintf(subpath, sizeof(subpath), "%s/%s", path, entry->d_name);

            struct stat st;
            if ((dirParams.list_links ? stat(subpath, &st) : lstat(subpath, &st)) == -1)
                continue;

            if (S_ISDIR(st.st_mode))
                list_dir(subpath, nivel + 1);
        }
        closedir(dir);
    }

    // RECA: listar después de recorrer
    if (dirParams.recurse == RECA) {
        printf("\n%s:\n", path);
        dir = opendir(path);
        if (!dir) return;
        while ((entry = readdir(dir)) != NULL) {
            if (!dirParams.show_hidden && entry->d_name[0] == '.')
                continue;
            print_entry(path, entry->d_name);
        }
        closedir(dir);
    }
}

void init_standard_fds(tList *ficherosAbiertos) {
    // Nombres descriptivos para cada descriptor estándar
    const char *names[] = {
        "entrada estandar",  // descriptor 0
        "salida estandar",   // descriptor 1
        "error estandar"     // descriptor 2
    };

    // Recorremos los tres descriptores (0, 1 y 2)
    for (int i = 0; i < 3; i++) {

        // Reservamos memoria para una nueva estructura tOpenFile
        tOpenFile *f = malloc(sizeof(tOpenFile));
        if (!f) {
            perror("malloc");  // Si falla, mostramos error y terminamos el programa
            exit(1);
        }

        // Asignamos la información básica del descriptor
        f->df = i;  // Número del descriptor (0, 1 o 2)

        // Copiamos un nombre descriptivo (limitado al tamaño del campo)
        strncpy(f->nombre, names[i], sizeof(f->nombre) - 1);
        f->nombre[sizeof(f->nombre) - 1] = '\0';  // Asegura terminación en '\0'

        // Modo de apertura: lectura y escritura
        // (por simplicidad usamos O_RDWR para los tres)
        f->modo = O_RDWR;

        // Insertamos la entrada en la lista de ficheros abiertos
        insertItem(ficherosAbiertos, f);
    }
}


//FUNCIONES P0
void cmd_authors(char *tokens[]){                                                                                   //Devuelve los nombres de los autores
    const char *nombres[] = {"Joel Candal Gómez", "Ou Zhang"};
    const char *logins[] = {"joel.candal", "ou.zhang"};
    if (tokens[1] != NULL && strcmp(tokens[1], "-n") == 0){
        for (int i = 0; i < 2; i++) printf("%s\n", nombres[i]);
    } else if (tokens[1] != NULL && strcmp(tokens[1], "-l") == 0){
        for (int i = 0; i < 2; i++) printf("%s\n", logins[i]);
    } else {
        for (int i = 0; i < 2; i++) printf("%s: %s\n", nombres[i], logins[i]);
    }
}

void cmd_getpid(char *tokens[]){
    if (tokens[1] != NULL && strcmp(tokens[1], "-p") == 0)
        printf("Pid del padre del shell: %d\n", getppid());                                                 //Devuelve el pid del proceso padre
    else
        printf("Pid de shell: %d\n", getpid());                                                             //Devuelve el pid del shell
}

// Cambia el directorio actual o lo muestra si no se pasa argumento
void cmd_chdir(char *tokens[]){
    if (tokens[1] == NULL){
        // Sin argumentos: muestra el directorio actual
        char *cwd = getcwd(NULL, 0);
        if (cwd == NULL){ perror("getcwd"); return; }
        printf("%s\n", cwd);
        free(cwd);
    } else {
        // Con argumento: intenta cambiar el directorio
        if (chdir(tokens[1]) != 0) perror("chdir");
    }
}

void cmd_getcwd(){
    char cwd[1025];
    if (getcwd(cwd, sizeof(cwd)) != NULL) printf("%s\n", cwd);
    else perror("getcwd()");
}

void cmd_date(char *tokens[]){
    time_t now;
    struct tm *local;
    char buffer[101];
    time(&now);
    local = localtime(&now);
    if (tokens[1] != NULL && strcmp(tokens[1], "-d") == 0){
        strftime(buffer, sizeof(buffer), "%d/%m/%Y", local);                                                //Enseña la fecha actual con el formato DD/MM/YYYY
        printf("%s\n", buffer);
    } else if (tokens[1] != NULL && strcmp(tokens[1], "-t") == 0){                                                       //Enseña la hora actual con el formato hh:mm:ss
        strftime(buffer, sizeof(buffer), "%H:%M:%S", local);
        printf("%s\n", buffer);
    } else {
        char buffer_date[51], buffer_time[50];                                                                           //Enseña fecha y hora en los formatos DD/MM/YYYY y hh:mm:ss
        strftime(buffer_date, sizeof(buffer_date), "%d/%m/%Y", local);
        strftime(buffer_time, sizeof(buffer_time), "%H:%M:%S", local);
        printf("%s %s\n", buffer_date, buffer_time);
    }
}

// Gestiona el historial de comandos del shell
void cmd_historic(char *tokens[], tList *historial){
    if (tokens[1] == NULL){
        // Sin argumentos → muestra todo el historial
        int i = 0;
        for (tPosL p = first(*historial); !listEnd(p); p = next(p)){
            char *command = (char *)getItem(p);
            printf("%d: %s", i, command);
            i++;
        }
    }
    else if (strcmp(tokens[1], "-count") == 0){
        // Muestra cuántos comandos hay guardados
        int total = 0;
        for (tPosL p = first(*historial); !listEnd(p); p = next(p)) total++;
        printf("Número de comandos en el historial: %d\n", total);
    }
    else if (strcmp(tokens[1], "-clear") == 0){
        // Borra todos los comandos del historial
        while(first(*historial) != NULL) deleteAtPosition(historial, first(*historial));
        printf("Historial borrado.\n");
    }
}


// Abre un fichero y lo añade a la lista de ficheros abiertos
void cmd_open(char* tr[], tList* ficherosAbiertos) {
    int i, mode = 0, df;

    // Si no hay argumento, lista los ficheros abiertos actuales
    if (tr[1] == NULL) {
        cmd_listopen(ficherosAbiertos);
        return;
    }

    // Construye los flags de apertura según los parámetros recibidos
    for (i = 2; tr[i] != NULL; i++) {
        if (!strcmp(tr[i], "cr")) mode |= O_CREAT;
        else if (!strcmp(tr[i], "ex")) mode |= O_EXCL;
        else if (!strcmp(tr[i], "ro")) mode |= O_RDONLY;
        else if (!strcmp(tr[i], "wo")) mode |= O_WRONLY;
        else if (!strcmp(tr[i], "rw")) mode |= O_RDWR;
        else if (!strcmp(tr[i], "ap")) mode |= O_APPEND;
        else if (!strcmp(tr[i], "tr")) mode |= O_TRUNC;
        else break;
    }

    // Intenta abrir el fichero con los modos seleccionados
    if ((df = open(tr[1], mode, 0777)) == -1) {
        perror("No se puede abrir fichero");
    } else {
        // Reserva memoria para registrar la información del fichero
        tOpenFile* f = malloc(sizeof(tOpenFile));
        f->df = df;
        strncpy(f->nombre, tr[1], 255);
        f->nombre[255] = '\0';
        f->modo = mode;
        insertItem(ficherosAbiertos, f);

        char modo[128] = "";

        if (f->modo & O_RDWR) strcpy(modo, "O_RDWR");
        if (f->modo & O_RDONLY) strcat(modo, "O_RDONLY");
        if (f->modo & O_WRONLY) strcpy(modo, "O_WRONLY");
        if (f->modo & O_CREAT) strcat(modo, "O_CREAT");
        if (f->modo & O_EXCL) strcat(modo, "O_EXCL");
        if (f->modo & O_APPEND) strcat(modo, "O_APPEND");
        if (f->modo & O_TRUNC) strcat(modo, "O_TRUNC");
        if (f->modo == 0) strcpy(modo, "O_RDONLY");
        // -------------------------------------------------------------------------------

        // Mostrar la información del fichero abierto con el modo en texto
        printf("Archivo '%s' abierto con descriptor %d y modo %s\n",
               f->nombre, f->df, modo);
    }
}

void cmd_close (char *tokens[], tList* ficherosAbiertos) {
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

void cmd_listopen(tList *ficherosAbiertos) {
    for (int fd = 0; fd < 20; fd++) {
        // Intenta obtener la posición actual del descriptor (si aplica)
        off_t offset = lseek(fd, 0, SEEK_CUR);
        if (offset == (off_t)-1) offset = -1;

        // Busca si el descriptor está en la lista de ficheros abiertos
        tPosL pos = findDf(fd, ficherosAbiertos);
        if (pos != NULL) {
            tOpenFile *f = (tOpenFile *)getItem(pos);
            char modo[128] = "";

            // Construye el texto del modo de apertura
            if (f->modo & O_RDWR) strcpy(modo, "O_RDWR");
            if (f->modo & O_RDONLY) strcat(modo, "O_RDONLY");
            if (f->modo & O_WRONLY) strcpy(modo, "O_WRONLY");
            if (f->modo & O_CREAT) strcat(modo, "O_CREAT");
            if (f->modo & O_EXCL) strcat(modo, "O_EXCL");
            if (f->modo & O_APPEND) strcat(modo, "O_APPEND");
            if (f->modo & O_TRUNC) strcat(modo, "O_TRUNC");
            if (f->modo == 0) strcpy(modo, "O_RDONLY");

            // Muestra la información del descriptor abierto
            printf("descriptor: %d, offset: (%ld)-> %s %s\n",
                   fd, (offset >= 0 ? (long)offset : 0), f->nombre, modo);
        } else {
            // Si no está en la lista, se muestra como no usado
            printf("descriptor: %d, offset: ( )-> no usado\n", fd);
        }
    }
}

void cmd_infosys(){
  struct utsname info;
  if (uname(&info) != 0) perror("uname");
  else printf("%s@%s %s, OS: %s-%s-%s\n", info.sysname, info.nodename, info.machine, info.sysname, info.release, info.version);
}

void cmd_help(char *tokens[]) {
    if (tokens[1] == NULL) {
        printf("Comandos disponibles:\n");
        printf("  authors         getpid          chdir           getcwd\n");
        printf("  date            hour            historic        open\n");
        printf("  close           dup             listopen        infosys\n");
        printf("  help            quit            exit            bye\n");
        printf("  create          erase           delrec          setdirparams\n");
        printf("  getdirparams    dir             lseek           writestr\n");
        printf("\nUse 'help comando' para obtener ayuda sobre un comando específico.\n");
    }
    else {
        if (strcmp(tokens[1], "authors") == 0)
            printf("authors [-l|-n]\n  Muestra los nombres y logins de los autores del programa.\n  -l muestra solo los logins, -n solo los nombres.\n");
        else if (strcmp(tokens[1], "getpid") == 0)
            printf("getpid [-p]\n  Muestra el PID del shell o, con -p, el PID de su proceso padre.\n");
        else if (strcmp(tokens[1], "chdir") == 0)
            printf("chdir [dir]\n  Cambia el directorio de trabajo actual a 'dir'.\n  Sin argumentos, muestra el directorio actual.\n");
        else if (strcmp(tokens[1], "getcwd") == 0)
            printf("getcwd\n  Muestra el directorio de trabajo actual.\n");
        else if (strcmp(tokens[1], "date") == 0)
            printf("date [-d|-t]\n  Muestra la fecha y hora actuales.\n  -d solo fecha, -t solo hora.\n");
        else if (strcmp(tokens[1], "hour") == 0)
            printf("hour\n  Muestra la hora actual (equivalente a 'date -t').\n");
        else if (strcmp(tokens[1], "historic") == 0)
            printf("historic [-clear|-count|N|-N]\n  Muestra o gestiona el historial de comandos.\n  -clear borra el historial, -count muestra el número de comandos.\n  N repite el comando N, -N muestra los últimos N comandos.\n");
        else if (strcmp(tokens[1], "open") == 0)
            printf("open [fichero] modo\n  Abre un fichero y lo añade a la lista de ficheros abiertos.\n  Modos: cr (O_CREAT), ap (O_APPEND), ex (O_EXCL), ro (O_RDONLY), rw (O_RDWR), wo (O_WRONLY), tr (O_TRUNC).\n  Sin argumentos, muestra los ficheros abiertos.\n");
        else if (strcmp(tokens[1], "close") == 0)
            printf("close df\n  Cierra el descriptor de fichero 'df' y lo elimina de la lista.\n");
        else if (strcmp(tokens[1], "dup") == 0)
            printf("dup df\n  Duplica el descriptor de fichero 'df' (usa la llamada dup()).\n");
        else if (strcmp(tokens[1], "listopen") == 0)
            printf("listopen\n  Lista los ficheros abiertos por el shell.\n");
        else if (strcmp(tokens[1], "infosys") == 0)
            printf("infosys\n  Muestra información del sistema (uname).\n");
        else if (strcmp(tokens[1], "help") == 0)
            printf("help [comando]\n  Muestra la lista de comandos o ayuda sobre un comando.\n");
        else if (strcmp(tokens[1], "quit") == 0 || strcmp(tokens[1], "exit") == 0 || strcmp(tokens[1], "bye") == 0)
            printf("%s\n  Finaliza la ejecución del shell.\n", tokens[1]);
        else if (strcmp(tokens[1], "create") == 0)
            printf("create [-f] nombre\n  Crea un archivo (-f) o un directorio con el nombre dado.\n");
        else if (strcmp(tokens[1], "erase") == 0)
            printf("erase n1 n2 ...\n  Borra los archivos y/o directorios vacíos indicados.\n");
        else if (strcmp(tokens[1], "delrec") == 0)
            printf("delrec n1 n2 ...\n  Borra recursivamente archivos y directorios no vacíos.\n");
        else if (strcmp(tokens[1], "setdirparams") == 0)
            printf("setdirparams [long|short] [link|nolink] [hid|nohid] [reca|recb|norec]\n  Configura parámetros para listados con 'dir'.\n");
        else if (strcmp(tokens[1], "getdirparams") == 0)
            printf("getdirparams\n  Muestra los parámetros actuales de listado de directorios.\n");
        else if (strcmp(tokens[1], "dir") == 0)
            printf("dir [-d] n1 n2 ...\n  Muestra información de archivos o directorios.\n  Con -d, lista el contenido de los directorios.\n  Admite parámetros long|short, link|nolink, hid|nohid, reca|recb|norec.\n");
        else if (strcmp(tokens[1], "lseek") == 0)
            printf("lseek df off ref\n  Cambia la posición del offset del fichero abierto.\n  ref puede ser SEEK_SET, SEEK_CUR o SEEK_END.\n");
        else if (strcmp(tokens[1], "writestr") == 0)
            printf("writestr df str\n  Escribe la cadena 'str' en el fichero abierto con descriptor df.\n");
        else
            printf("No existe ayuda para el comando '%s'.\n", tokens[1]);
    }
}

void cmd_exit(){
    exit(0);
}

int TrocearCadena(char *cadena, char *trozos[]){
    int i = 1;
    if ((trozos[0] = strtok(cadena, " \n\t")) == NULL) return 0;
    while ((trozos[i] = strtok(NULL, " \n\t")) != NULL) i++;
    return i;
}

void leerEntrada(char *entrada){
    fgets(entrada, MAX_TOKENS, stdin);
}

//FUNCIONES P1

// Crea un archivo (-f) o un directorio
void cmd_create(char* tokens[]) {
    if (tokens[1] == NULL) {
        printf("Uso: create [-f] nombre\n");
        return;
    }

    if (strcmp(tokens[1], "-f") == 0) {
        // Modo archivo
        if (tokens[2] == NULL) {
            printf("Falta el nombre del fichero\n");
            return;
        }

        int fd = open(tokens[2], O_CREAT | O_EXCL, 0666);
        if (fd == -1) perror("No se pudo crear el fichero");
        else {
            printf("Fichero '%s' creado correctamente\n", tokens[2]);
            close(fd);
        }
    } else {
        // Modo directorio
        if (mkdir(tokens[1], 0777) == -1)
            perror("No se pudo crear el directorio");
        else
            printf("Directorio '%s' creado correctamente\n", tokens[1]);
    }
}

void cmd_setdirparams(char *tokens[]){
    if (tokens[1] == NULL){
        printf("Uso: setdirparams [long|short] [link|nolink] [hid|nohid] [reca|recb|norec]\n");
        return;
    }

    for (int i = 1; tokens[i] != NULL; i++){
        if (strcmp(tokens[i], "long") == 0) dirParams.long_format = true;
        else if (strcmp(tokens[i], "short") == 0) dirParams.long_format = false;
        else if (strcmp(tokens[i], "link") == 0) dirParams.list_links = true;
        else if (strcmp(tokens[i], "nolink") == 0) dirParams.list_links = false;
        else if (strcmp(tokens[i], "hid") == 0) dirParams.show_hidden = true;
        else if (strcmp(tokens[i], "nohid") == 0) dirParams.show_hidden = false;
        else if (strcmp(tokens[i], "reca") == 0) dirParams.recurse = RECA;
        else if (strcmp(tokens[i], "recb") == 0) dirParams.recurse = RECB;
        else if (strcmp(tokens[i], "norec") == 0) dirParams.recurse = NREC;
        else printf("Parámetro desconocido: %s\n", tokens[i]);
    }
}

void cmd_getdirparams(char *tokens[]){
    (void)tokens;
    printf("Formato: %s\n", dirParams.long_format ? "long" : "short");
    printf("Tratar enlaces simbólicos: %s\n", dirParams.list_links ? "link" : "nolink");
    printf("Mostrar ocultos: %s\n", dirParams.show_hidden ? "hid" : "nohid");
    printf("Recursión: ");
    if (dirParams.recurse == RECA) printf("reca (después)\n");
    else if (dirParams.recurse == RECB) printf("recb (antes)\n");
    else printf("norec (no recursivo)\n");
}

// Implementa el comando 'dir' que muestra el contenido de archivos o directorios
void cmd_dir(char *tokens[], tDirParams *params) {
    bool list_contents = false;
    int i = 1;

    // Detecta si se pidió la opción "-d" (listar contenidos)
    if (tokens[i] != NULL && strcmp(tokens[i], "-d") == 0) {
        list_contents = true;
        i++;
    }

    // Si no se pasa ningún argumento, se lista el directorio actual
    if (tokens[i] == NULL) {
        list_contents = true;
        tokens[i] = ".";
        tokens[i + 1] = NULL;
    }

    // Recorre todos los argumentos (archivos o directorios)
    for (; tokens[i] != NULL; i++) {
        struct stat st;

        // Obtiene información del archivo/directorio
        if ((params->list_links ? stat(tokens[i], &st) : lstat(tokens[i], &st)) == -1) {
            perror(tokens[i]);
            continue;
        }

        // Si es directorio y hay que listar su contenido
        if (S_ISDIR(st.st_mode) && list_contents) {
            list_dir(tokens[i], 0);
        } else {
            // Si es archivo o no se pidió listar contenido, muestra solo su info
            print_entry(".", tokens[i]);
        }
    }
}

// Borra archivos o directorios vacíos
void cmd_erase(char *tokens[]){
    for (int i = 1; tokens[i] != NULL; i++){
        if (EsDirectorio(tokens[i])){
            // Si es un directorio, intenta borrarlo (solo si está vacío)
            if (rmdir(tokens[i]) == -1) perror(tokens[i]);
        } else {
            // Si es un archivo normal, usa remove()
            if (remove(tokens[i]) == -1) perror(tokens[i]);
        }
    }
}

void cmd_delrec(char *tokens[]){
    for (int i = 1; tokens[i] != NULL; i++) borrarRecursivo(tokens[i]);
}

void cmd_lseek(char *tokens[], tList *ficherosAbiertos){
    if (tokens[1] == NULL || tokens[2] == NULL || tokens[3] == NULL){
        printf("Uso: lseek df off ref   (ref: SEEK_SET|SEEK_CUR|SEEK_END or 0|1|2)\n");
        return;
    }

    int df = atoi(tokens[1]);
    if (df < 0){                                                                                                        //Comprueba que el descriptor que le hemos pasado sea correcto
        printf("Descriptor inválido\n");
        return;
    }

    off_t offset = (off_t)atoll(tokens[2]);
    int ref;

    if (strcmp(tokens[3], "SEEK_SET") == 0 || strcmp(tokens[3], "0") == 0)
        ref = SEEK_SET;                                                                                              //El offset comienza al principio
    else if (strcmp(tokens[3], "SEEK_CUR") == 0 || strcmp(tokens[3], "1") == 0)
        ref = SEEK_CUR;                                                                                              //El offset comienza desde la posición actual
    else if (strcmp(tokens[3], "SEEK_END") == 0 || strcmp(tokens[3], "2") == 0)
        ref = SEEK_END;                                                                                              //El offset comienza desde el final
    else {
        printf("Referencia desconocida: %s\n", tokens[3]);
        return;
    }

    tPosL p = findDf(df, ficherosAbiertos);                                                                             //Devuelve la posición del descriptor a buscar
    if (p == NULL){                                                                                                     //En caso de error da fallo
        printf("Descriptor %d no está abierto\n", df);
        return;
    }

    tOpenFile *f = (tOpenFile *)getItem(p);                                                                             //Devuelve el elemento de la lista
    off_t res = lseek(f->df, offset, ref);                                                                    //Mueve el puntero de posición del descriptor
    if (res == (off_t)-1)
        perror("lseek");                                                                                              //En caso de fallo da error
    else
        printf("Descriptor %d posicionado en %ld\n", df, (long)res);
}

void cmd_writestr(char *tokens[], tList *ficherosAbiertos){
    if (tokens[1] == NULL || tokens[2] == NULL){
        printf("Uso: writestr df str\n");
        return;
    }

    int df = atoi(tokens[1]);
    if (df < 0){
        printf("Descriptor inválido\n");
        return;
    }

    //Calcular longitud total
    size_t len = 0;
    for (int i = 2; tokens[i] != NULL; i++)
        len += strlen(tokens[i]) + (i > 2 ? 1 : 0);

    //Reservar memoria
    char *buf = malloc(len + 1);
    if (!buf){
        perror("malloc");
        return;
    }

    //Reconstruir cadena eficientemente
    char *p = buf;
    for (int i = 2; tokens[i] != NULL; i++){
        if (i > 2) *p++ = ' ';
        size_t l = strlen(tokens[i]);
        memcpy(p, tokens[i], l);
        p += l;
    }
    *p = '\0';

    //Buscar descriptor en la lista
    tPosL pos = findDf(df, ficherosAbiertos);
    if (pos == NULL){
        printf("Descriptor %d no está abierto\n", df);
        free(buf);
        return;
    }

    //Escribir
    tOpenFile *f = (tOpenFile *)getItem(pos);
    ssize_t w = write(f->df, buf, strlen(buf));
    if (w == -1)
        perror("write");
    else
        printf("Escritos %zd bytes en descriptor %d\n", w, df);

    free(buf);
}


void cmd_handler(char *entrada, tList *historial, tList *ficherosAbiertos, tDirParams *params){
    char *tokens[64];
    char *entradaOriginal = strdup(entrada);
    TrocearCadena(entrada, tokens);

    if (tokens[0] != NULL) {
        if (strcmp(tokens[0], "getpid") == 0)
            cmd_getpid(tokens);
        else if (strcmp(tokens[0], "authors") == 0)
            cmd_authors(tokens);
        else if (strcmp(tokens[0], "infosys") == 0)
            cmd_infosys();
        else if (strcmp(tokens[0], "historic") == 0)
            cmd_historic(tokens, historial);
        else if (strcmp(tokens[0], "chdir") == 0 || strcmp(tokens[0], "cd") == 0)
            cmd_chdir(tokens);
        else if (strcmp(tokens[0], "date") == 0)
            cmd_date(tokens);
        else if (strcmp(tokens[0], "help") == 0)
            cmd_help(tokens);
        else if (strcmp(tokens[0], "getcwd") == 0)
            cmd_getcwd();
        else if (strcmp(tokens[0], "create") == 0)
            cmd_create(tokens);
        else if (strcmp(tokens[0], "erase") == 0)
            cmd_erase(tokens);
        else if (strcmp(tokens[0], "delrec") == 0)
            cmd_delrec(tokens);
        else if (strcmp(tokens[0], "setdirparams") == 0)
            cmd_setdirparams(tokens);
        else if (strcmp(tokens[0], "getdirparams") == 0)
            cmd_getdirparams(tokens);
        else if (strcmp(tokens[0], "open") == 0)
            cmd_open(tokens, ficherosAbiertos);
        else if (strcmp(tokens[0], "close") == 0)
            cmd_close(tokens, ficherosAbiertos);
        else if (strcmp(tokens[0], "dup") == 0)
            cmd_dup(tokens, ficherosAbiertos);
        else if (strcmp(tokens[0], "listopen") == 0)
            cmd_listopen(ficherosAbiertos);
        else if (strcmp(tokens[0], "dir") == 0)
            cmd_dir(tokens, params);
        else if (strcmp(tokens[0], "lseek") == 0)
            cmd_lseek(tokens, ficherosAbiertos);
        else if (strcmp(tokens[0], "writestr") == 0)
            cmd_writestr(tokens, ficherosAbiertos);


        addHistorial(entradaOriginal, historial);


        if (strcmp(tokens[0], "quit") == 0 || strcmp(tokens[0], "exit") == 0 || strcmp(tokens[0], "bye") == 0) {
            clear(ficherosAbiertos);
            clear(historial);
            exit(0);
        }
    } else {
        free(entradaOriginal);
    }
}

int main() {
    tList ficherosAbiertos;
    tList historial;
    CreateEmptyList(&ficherosAbiertos);
    CreateEmptyList(&historial);

    // 🔹 Inicializa stdin, stdout, stderr
    init_standard_fds(&ficherosAbiertos);

    char entrada[MAX_TOKENS];
    while (true) {
        printf("->");
        leerEntrada(entrada);
        cmd_handler(entrada, &historial, &ficherosAbiertos, &dirParams);
    }
}
