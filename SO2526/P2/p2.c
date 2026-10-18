
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
#include <limits.h>
#include "list.h"
#include <sys/mman.h>
#include <sys/shm.h>
#include <sys/wait.h>


#define MAX_TOKENS 256
#define TAMANO 1024

float varglobal1;
int varglobal2;
double varglobal3;
int global_ini1 = 5;
char global_ini2[] = "hola";
double global_ini3 = 3.14;

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
void add_block(tList *L, void *addr, size_t size, alloc_type type);
tPosL find_block_by_size(tList *L, size_t size);
void remove_block(tList *L, tPosL pos);  
void ImprimirListaMalloc(tList *L);
void *MapearFichero(char *fichero, int protection, tList *L, tList *ficherosAbiertos);
void *ObtenerMemoriaShmget (key_t clave, size_t tam);
void do_SharedCreate (char *tr[], tList *allocs_list);
void do_Shared (char *tr[], tList *allocs_list);
void doSharedFree (key_t cl, tList *allocs_list);
void do_SharedDelkey (char *args[]);
void InsertarNodoShared(tList *allocs_list, void *addr, size_t size, key_t key);
void *DireccionNodoShared(tList *allocs_list, key_t key); 
int EliminarNodoDireccion(tList *allocs_list, void *addr);
void ImprimirListaShared(tList *allocs_list);
void LlenarMemoria (void *p, size_t cont, unsigned char byte);
void * CadenatoPointer (char * s);
void Recursiva (int n);
ssize_t LeerFichero (char *f, void *p, ssize_t cont);
void *unmapearFichero(char *fichero, tList *allocs_list, tList *ficherosAbiertos);
void memFuncs();
void memVars();
void Do_pmap (void);
void ImprimirListaMMap(tList *L);

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
void cmd_handler(char *entrada, tList *historial, tList *ficherosAbiertos, tDirParams *params, tList *allocs_list);
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

//Funciones p2
void cmd_malloc(char *tokens[], tList *allocs_list);
void cmd_mmap(char *tokens[], tList *allocs_list, tList *ficherosAbiertos);
void cmd_shared(char *tokens[], tList *allocs_list);
void cmd_memfill(char *tokens[]);
void cmd_memdump(char *tokens[]);
void cmd_recurse(char *tokens[]);
void cmd_free(char *tokens[], tList *allocs_list);
void cmd_readfile (char *tokens[]);
void cmd_read(char *tokens[], tList *ficherosAbiertos);
void cmd_writefile(char *tokens[]);
void cmd_write(char *tokens[], tList *ficherosAbiertos);
void cmd_mem(char *tokens[], tList *allocs_list);

//FUNCIONES AUXILIARES

void memFuncs(){
  printf("Funciones programa:\t %p,\t %p,\t %p\n", &cmd_lseek, &cmd_infosys, &cmd_dup);
  printf("Funciones libreria:\t %p,\t %p,\t %p\n", &malloc, &free, &printf);
}

void memVars(){

  int var_local1, var_local2, var_local3;
  static float var_estatica1, var_estatica2, var_estatica3;
  static int estatica_ini1 = 10;
  static float estatica_ini2 = 20.5;
  static char estatica_ini3 = 'z';

  
  printf("Variables locales:\t %p,\t %p,\t %p\n", &var_local1, &var_local2, &var_local3);
  printf("Variables globales:\t %p,\t %p,\t %p\n", &varglobal1, &varglobal2, &varglobal3);
  printf("Var (N.I.)globales:\t %p,\t %p,\t %p\n", &global_ini1, &global_ini2, &global_ini3);
  printf("Variables staticas:\t %p,\t %p,\t %p\n", &estatica_ini1, &estatica_ini2, &estatica_ini3);
  printf("Var (N.I.)staticas:\t %p,\t %p,\t %p\n", &var_estatica1, &var_estatica2, &var_estatica3);
}

void Do_pmap (void) /*sin argumentos*/
 { pid_t pid;       /*hace el pmap (o equivalente) del proceso actual*/
   char elpid[32];
   char *argv[4]={"pmap",elpid,NULL};
   
   sprintf (elpid,"%d", (int) getpid());
   if ((pid=fork())==-1){
      perror ("Imposible crear proceso");
      return;
      }
   if (pid==0){
      if (execvp(argv[0],argv)==-1)
         perror("cannot execute pmap (linux, solaris)");
         
      argv[0]="procstat"; argv[1]="vm"; argv[2]=elpid; argv[3]=NULL;   
      if (execvp(argv[0],argv)==-1)/*No hay pmap, probamos procstat FreeBSD */
         perror("cannot execute procstat (FreeBSD)");
         
      argv[0]="procmap",argv[1]=elpid;argv[2]=NULL;    
            if (execvp(argv[0],argv)==-1)  /*probamos procmap OpenBSD*/
         perror("cannot execute procmap (OpenBSD)");
         
      argv[0]="vmmap"; argv[1]="-interleave"; argv[2]=elpid;argv[3]=NULL;
      if (execvp(argv[0],argv)==-1) /*probamos vmmap Mac-OS*/
         perror("cannot execute vmmap (Mac-OS)");      
      exit(1);
  }
  waitpid (pid,NULL,0);
}

ssize_t LeerFichero (char *f, void *p, ssize_t cont)
{
  struct stat s;
  ssize_t  n;  
  int df,aux;

  if (stat (f,&s)==-1 || (df=open(f,O_RDONLY))==-1)
    return -1;     
  if (cont==-1)   /* si pasamos -1 como bytes a leer lo leemos entero*/
    cont=s.st_size;
  if ((n=read(df,p,cont))==-1){
    aux=errno;
    close(df);
    errno=aux;
    return -1;
  }
  close (df);
  return n;
}


void Recursiva (int n){
  char automatico[TAMANO];
  static char estatico[TAMANO];

  printf ("parametro:%3d(%p) array %p, arr estatico %p\n",n,&n,automatico, estatico);

  if (n>0)
    Recursiva(n-1);
}

void * CadenatoPointer (char * s){
  void *p;
  sscanf(s,"%p",&p);
  if (p==NULL)
    errno=EFAULT;
  return p;
}

void LlenarMemoria (void *p, size_t cont, unsigned char byte){
  unsigned char *arr=(unsigned char *) p;
  size_t i;

  for (i=0; i<cont;i++)
    arr[i]=byte;
}

void InsertarNodoShared(tList *allocs_list, void *addr, size_t size, key_t key) {
  Block_Memory *bm = malloc(sizeof(Block_Memory));
  if (!bm) { perror("malloc"); return; }

  bm->addr = addr;
  bm->size = size;
  bm->allocationTime = time(NULL);
  bm->type = SHARED;
  bm->key = key;
  bm->filename = NULL;
  bm->fd = -1;

  insertItem(allocs_list, bm);
}


/* Devuelve la dirección asociada a una clave SHM */
void *DireccionNodoShared(tList *allocs_list, key_t key) {
  for (tPosL p = first(*allocs_list); !listEnd(p); p = next(p)) {
    Block_Memory *bm = (Block_Memory *)getItem(p);
    if (bm->type == SHARED && bm->key == key)
      return bm->addr;
  }
  return NULL;
}


/* Elimina un nodo por dirección */
int EliminarNodoDireccion(tList *allocs_list, void *addr) {
  for (tPosL p = first(*allocs_list); !listEnd(p); p = next(p)) {
    Block_Memory *bm = getItem(p);

    if (bm->addr == addr) {
      free(bm);
      deleteAtPosition(allocs_list, p);
      return 0;
    }
  }
  return -1;   // no encontrado
}


/* Imprime todos los bloques shared */
void ImprimirListaShared(tList *allocs_list) {
  pid_t pid = getpid();
  printf("******Lista de bloques asignados shared para el proceso %d\n", pid);

  for (tPosL p = first(*allocs_list); !listEnd(p); p = next(p)) {
    Block_Memory *bm = getItem(p);

    if (bm->type == SHARED) {
      char buf[64];
      strftime(buf, sizeof(buf), "%b %d %H:%M",
               localtime(&bm->allocationTime));

      printf("%14p %10zu   %s   shared (key=%d)\n",
             bm->addr, bm->size, buf, bm->key);
    }
  }
}

void *ObtenerMemoriaShmget (key_t clave, size_t tam)
{
  void * p;
  int aux,id,flags=0777; /*los 9 bits menos significativos de los flags:permisos*/
  struct shmid_ds s;

  if (tam)     /*tam distito de 0 indica crear */
    flags=flags | IPC_CREAT | IPC_EXCL; /*cuando no es crear pasamos de tamano 0*/
  if (clave==IPC_PRIVATE)  /*no nos vale*/
  {errno=EINVAL; return NULL;}
  if ((id=shmget(clave, tam, flags))==-1)
    return (NULL);
  if ((p=shmat(id,NULL,0))==(void*) -1){
    aux=errno;
    if (tam)
      shmctl(id,IPC_RMID,NULL);
    errno=aux;
    return (NULL);
  }
  shmctl (id,IPC_STAT,&s); /* si no es crear, necesitamos el tamano, que es s.shm_segsz*/
  /* Guardar en la lista   InsertarNodoShared (&L, p, s.shm_segsz, clave); */
  return (p);
}

void do_SharedCreate (char *tr[], tList *allocs_list)
{
  key_t cl;
  size_t tam;
  void *p;

  if (tr[0]==NULL || tr[1]==NULL) {
    ImprimirListaShared(allocs_list);
    return;
  }

  cl=(key_t)  strtoul(tr[0],NULL,10);
  tam=(size_t) strtoul(tr[1],NULL,10);
  if (tam==0) {
    printf ("No se asignan bloques de 0 bytes\n");
    return;
  }
  if ((p=ObtenerMemoriaShmget(cl,tam))!=NULL)
    printf ("Asignados %lu bytes en %p\n",(unsigned long) tam, p);
  else
    printf ("Imposible asignar memoria compartida clave %lu:%s\n",(unsigned long) cl,strerror(errno));
}

void do_Shared (char *tr[], tList *allocs_list)
{
  key_t cl;
  void *p;

  if (tr[0]==NULL) {
    ImprimirListaShared(allocs_list);
    return;
  }

  cl=(key_t)  strtoul(tr[0],NULL,10);

  if ((p=ObtenerMemoriaShmget(cl,0))!=NULL)
    printf ("Asignada memoria compartida de clave %lu en %p\n",(unsigned long) cl, p);
  else
    printf ("Imposible asignar memoria compartida clave %lu:%s\n",(unsigned long) cl,strerror(errno));
}

void doSharedFree (key_t cl, tList *allocs_list)
{
  void *p;                                 /*llamo a la funcion de la lista*/
  /*que devuelve la direccion donde*/
  if ((p=DireccionNodoShared (allocs_list,cl))==NULL){/*esta la memoria de clave cl*/
    printf ("No hay bloque de esa clave mapeado en el proceso\n");
    return;
  }
  shmdt(p);
  if (EliminarNodoDireccion (allocs_list,p)==-1)  /*elimino el nodo */
    printf ("Imposible quitar de lista: %s\n",strerror(errno));
}

void do_SharedDelkey (char *args[])
{
  key_t clave;
  int id;
  char *key=args[0];

  if (key==NULL || (clave=(key_t) strtoul(key,NULL,10))==IPC_PRIVATE){
    printf ("      delkey necesita clave_valida\n");
    return;
  }
  if ((id=shmget(clave,0,0666))==-1){
    perror ("shmget: imposible obtener memoria compartida");
    return;
  }
  if (shmctl(id,IPC_RMID,NULL)==-1)
    perror ("shmctl: imposible eliminar memoria compartida\n");
}


void add_block(tList *L, void *addr, size_t size, alloc_type type)
{
  Block_Memory *bm = malloc(sizeof(Block_Memory));

  bm->addr = addr;
  bm->size = size;
  bm->type = type;
  bm->allocationTime = time(NULL);
  bm->key = -1;
  bm->filename = NULL;
  bm->fd = -1;

  insertItem(L, bm);
}


tPosL find_block_by_size(tList *L, size_t size)
{
  for (tPosL p = first(*L); !listEnd(p); p = next(p)) 
  {
    Block_Memory *bm = getItem(p);

    if (bm->type == MALLOC && bm->size == size)
      return p;
  }
  return NULL;
}


void remove_block(tList *L, tPosL pos) 
{
  Block_Memory *bm = getItem(pos);
  free(bm);
  deleteAtPosition(L, pos);
}


void ImprimirListaMalloc(tList *L) 
{
    pid_t pid = getpid();
    char buf[64];

    printf("******Lista de bloques asignados malloc para el proceso %d\n", pid);

    for (tPosL p = first(*L); !listEnd(p); p = next(p))
  {
        Block_Memory *bm = getItem(p);

        if (bm->type == MALLOC) {
            strftime(buf, sizeof(buf), "%d %b %y %H:%M", localtime(&bm->allocationTime));
            printf("%14p %10zu   %s\n", bm->addr, bm->size, buf);
        }
    }
}

void ImprimirListaMMap(tList *L) 
{
    pid_t pid = getpid();
    char buf[64];

    printf("******Lista de bloques asignados mmap para el proceso %d\n", pid);

    for (tPosL p = first(*L); !listEnd(p); p = next(p)) {
        Block_Memory *bm = getItem(p);

        if (bm->type == MMAP) {
            strftime(buf, sizeof(buf), "%d %b %y %H:%M", localtime(&bm->allocationTime));
            printf("%14p %10zu   %s   %s  (descriptor %d)\n",
                   bm->addr, bm->size, buf, bm->filename, bm->fd);
        }
    }
}

void ImprimirListaMemoria(tList *L)
{
  pid_t pid_t = getpid();
  char buf[64];

  printf("******Lista de bloques asignados para el proceso %d\n", pid_t);

  for (tPosL p = first(*L); !listEnd(p); p = next(p)) {

      Block_Memory *bm = getItem(p);

      strftime(buf, sizeof(buf), "%d %b %y %H:%M", localtime(&bm->allocationTime));

      if (bm->type == MALLOC) {

          printf("%14p %10zu   %s   malloc\n", bm->addr, bm->size, buf);

      } else if (bm->type == SHARED) {

          printf("%14p %10zu   %s   shared (key=%d)\n", bm->addr, bm->size, buf, bm->key);

      } else if (bm->type == MMAP) {

          printf("%14p %10zu   %s   mmap %s (descriptor %d)\n", bm->addr, bm->size, buf, bm->filename, bm->fd);
      }
  }
}

void *MapearFichero(char *fichero, int protection, tList *allocs_list, tList *ficherosAbiertos){
  int df, map = MAP_PRIVATE, modo = O_RDONLY;
  struct stat s;
  void *p;
  Block_Memory *memBlock;
  tPosL inserted_pos = NULL;                              //Posición del bloque insertado

  memBlock = malloc(sizeof(Block_Memory));                //Reservar memoria para el bloque de memoria
  if (!memBlock) return NULL;                             //Comprobar si malloc ha fallado y devuelve null        

  memBlock->filename = malloc(strlen(fichero) + 1);       //Reservar memoria para el nombre del fichero
  if (!memBlock->filename) {                              //Comprobar si malloc ha fallado y devuelve null
    free(memBlock);                                       //Liberar memoria del bloque de memoria
    return NULL;
  }
  
  if (protection & PROT_WRITE)                           
    modo = O_RDWR;
  if (stat(fichero, &s) == -1 || (df = open(fichero, modo)) == -1) { 
    free(memBlock->filename);                            //Liberar memoria del nombre del fichero
    free(memBlock);                                      //Liberar memoria del bloque de memoria                
    return NULL;
  }
  
  if ((p = mmap(NULL, s.st_size, protection, map, df, 0)) == MAP_FAILED) {      //Mapear el fichero
    close(df);
    free(memBlock->filename);
    free(memBlock);
    return NULL;
  }

  // Rellenar la estructura Block_Memory con los datos correspondientes
  memBlock->addr = p;
  memBlock->size = s.st_size;
  memBlock->allocationTime = time(NULL);
  memBlock->type = MMAP;
  memBlock->key = 0;
  memBlock->fd = df;
  strcpy(memBlock->filename, fichero);

  // Insertar y guardar la posición
  if (insertItem(allocs_list, memBlock) == 0) {
    // Encontrar la posición del elemento recién insertado
    inserted_pos = last(*allocs_list);
  }

  tOpenFile *f = malloc(sizeof(tOpenFile));               //Reservar memoria para la estructura tOpenFile
  if (!f){                                                
    // Eliminar el bloque recién insertado si existe
    if (inserted_pos != NULL) {
      Block_Memory *inserted_bm = getItem(inserted_pos);
      if (inserted_bm == memBlock) {
        deleteAtPosition(allocs_list, inserted_pos);
      }
    }
    //Liberamos memoria y cerramos el fichero
    munmap(p, s.st_size);
    close(df);
    free(memBlock->filename);
    free(memBlock);
    return NULL;
  }
  
  //Añadimos a la lista de ficheros abiertos
  f->df = df;
  f->modo = modo;
  strncpy(f->nombre, fichero, 255);
  f->nombre[255] = '\0';
  insertItem(ficherosAbiertos, f);

  return p;
}

void *unmapearFichero(char *fichero, tList *allocs_list, tList *ficherosAbiertos) {

    // Buscar el bloque de memoria mapeada correspondiente
    for (tPosL p = first(*allocs_list); !listEnd(p); p = next(p)) {
        Block_Memory *bm = getItem(p);

        if (bm->type == MMAP && strcmp(bm->filename, fichero) == 0) {

            void *addr = bm->addr;

            // Desmapea la memoria
            munmap(bm->addr, bm->size);

            // Cerrar descriptor si existe
            if (bm->fd != -1)
                close(bm->fd);

            // Eliminar entrada de ficheros abiertos
            for (tPosL q = first(*ficherosAbiertos); !listEnd(q); q = next(q)) {
                tOpenFile *fo = getItem(q);

                if (fo->df == bm->fd) {   // busca el descriptor en en la lista
                    free(fo);
                    deleteAtPosition(ficherosAbiertos, q);
                    break;  // solo 1
                }
            }

            // Liberar memoria del bloque
            free(bm->filename);
            free(bm);
            deleteAtPosition(allocs_list, p);

            return addr;  // Devuelve la dirección desmapeada
        }
    }

    return NULL;
}




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

  // RECA o NOREC: listar despues de recorrer
  if (dirParams.recurse == RECA || dirParams.recurse == NREC) {
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

  // RECB: listar antes de recorrer
  if (dirParams.recurse == RECB) {
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
    // Sin argumentos -> muestra todo el historial
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
  else if (strcmp(tokens[1], "-clear") == 0) {
    tPosL p = first(*historial);
    while (p != NULL && !listEnd(p)) {
      char *cmd = (char *)getItem(p);
      free(cmd);
      deleteAtPosition(historial, p);
      p = first(*historial);
    }
    printf("Historial borrado.\n");
  }
  else {
    int index = atoi(tokens[1]);  // convierte argumento a número
    int i = 0;
    for (tPosL p = first(*historial); !listEnd(p); p = next(p)) {
      if (i == index) {
        char *command = (char *)getItem(p);
        printf("Ejecutando comando [%d]: %s\n", index, command);
        // Aquí podrías volver a ejecutar el comando, por ejemplo:
        // executeCommand(command);
        return;
      }
      i++;
    }
    printf("Índice %d fuera de rango.\n", index);
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

    // Flags de acceso (mutuamente excluyentes)
    if (f->modo & O_RDWR) strcpy(modo, "O_RDWR");
    else if (f->modo & O_WRONLY) strcpy(modo, "O_WRONLY");
    else strcpy(modo, "O_RDONLY");  // Por defecto

    // Flags adicionales (pueden combinarse)
    if (f->modo & O_CREAT) strcat(modo, "|O_CREAT");
    if (f->modo & O_EXCL) strcat(modo, "|O_EXCL");
    if (f->modo & O_APPEND) strcat(modo, "|O_APPEND");
    if (f->modo & O_TRUNC) strcat(modo, "|O_TRUNC");
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

  tPosL p = findDf(df, ficherosAbiertos);                                                                             // Devuelve la posición del descriptor a buscar
  if (p == NULL){                                                                                                     // En caso de error da fallo
    printf("Descriptor %d no está abierto\n", df);
    return;
  }

  tOpenFile *f = (tOpenFile *)getItem(p);                                                                             // Devuelve el elemento de la lista
  off_t res = lseek(f->df, offset, ref); //Mueve el puntero de posición del descriptor
  if (res == (off_t)-1)
    perror("lseek"); //En caso de fallo da error
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

  // Calcular longitud total
  size_t len = 0;
  for (int i = 2; tokens[i] != NULL; i++)
    len += strlen(tokens[i]) + (i > 2 ? 1 : 0);

  // Reservar memoria
  char *buf = malloc(len + 1);
  if (!buf){
    perror("malloc");
    return;
  }

  // Reconstruir cadena eficientemente
  char *p = buf;
  for (int i = 2; tokens[i] != NULL; i++){
    if (i > 2) *p++ = ' ';
    size_t l = strlen(tokens[i]);
    memcpy(p, tokens[i], l);
    p += l;
  }
  *p = '\0';

  // Buscar descriptor en la lista
  tPosL pos = findDf(df, ficherosAbiertos);
  if (pos == NULL){
    printf("Descriptor %d no está abierto\n", df);
    free(buf);
    return;
  }

  // Escribir
  tOpenFile *f = (tOpenFile *)getItem(pos);
  ssize_t w = write(f->df, buf, strlen(buf));
  if (w == -1)
    perror("write");
  else
    printf("Escritos %zd bytes en descriptor %d\n", w, df);

  free(buf);
}

// FUNCIONES P2

void cmd_malloc(char *tokens[], tList *allocs_list) 
{

  if (tokens[1] == NULL) {
    ImprimirListaMalloc(allocs_list);
    return;
  }

  if (strcmp(tokens[1], "-free") == 0) {

    if (tokens[2] == NULL) {
      printf("Uso: malloc -free <n>\n");
      return;

    }

    size_t s = strtoul(tokens[2], NULL, 10);
    tPosL pos = find_block_by_size(allocs_list, s);

    if (!pos) {
      printf("No hay bloque de ese tamaño asignado con malloc\n");
      return;
    }

    Block_Memory *bm = getItem(pos);
    free(bm->addr);
    remove_block(allocs_list, pos);
    return;
  }

  size_t s = strtoul(tokens[1], NULL, 10);

  if (s == 0) {
    fprintf(stderr, "No se pueden asignar bloques de 0 bytes\n");
    return;
  }

  void *p = malloc(s);

  if (!p) {
    perror("malloc");
    return;
  }

  printf("Asignados %zu bytes en %p\n", s, p);

  add_block(allocs_list, p, s, MALLOC);
}

void cmd_shared(char *tokens[], tList *allocs_list) {

  // Caso: shared listar bloques shared
  if (tokens[1] == NULL) {
    ImprimirListaShared(allocs_list);
    return;
  }

  // OPCIÓN -create
  if (strcmp(tokens[1], "-create") == 0) {
    if (tokens[2] == NULL || tokens[3] == NULL) {
      printf("Uso: shared -create <clave> <tam>\n");
      return;
    }

    key_t cl = (key_t) strtoul(tokens[2], NULL, 10);
    size_t tam = (size_t) strtoul(tokens[3], NULL, 10);

    if (tam == 0) {
      printf("No se asignan bloques de 0 bytes\n");
      return;
    }

    void *p = ObtenerMemoriaShmget(cl, tam);
    if (!p) {
      printf("Imposible asignar memoria compartida clave %lu: %s\n",
             (unsigned long) cl, strerror(errno));
      return;
    }

    printf("Asignados %lu bytes en %p\n", (unsigned long) tam, p);

    // Guardar en la lista 
    Block_Memory *bm = malloc(sizeof(Block_Memory));
    bm->addr = p;
    bm->size = tam;
    bm->allocationTime = time(NULL);
    bm->type = SHARED;
    bm->key = cl;
    bm->filename = NULL;
    bm->fd = -1;

    insertItem(allocs_list, bm);
    return;
  }

  // OPCIÓN -free
  if (strcmp(tokens[1], "-free") == 0) {
    if (tokens[2] == NULL) {
      printf("Uso: shared -free <clave>\n");
      return;
    }

    key_t cl = (key_t) strtoul(tokens[2], NULL, 10);

    // Encontrar en lista
    for (tPosL p = first(*allocs_list); !listEnd(p); p = next(p)) {
      Block_Memory *bm = getItem(p);

      if (bm->type == SHARED && bm->key == cl) {

        // detach
        shmdt(bm->addr);

        // eliminar entrada
        deleteAtPosition(allocs_list, p);
        free(bm);

        return;
      }
    }

    printf("No hay bloque de esa clave mapeado en el proceso\n");
    return;
  }

  // OPCIÓN -delkey
  if (strcmp(tokens[1], "-delkey") == 0) {
    if (tokens[2] == NULL) {
      printf("Uso: shared -delkey <clave>\n");
      return;
    }
    do_SharedDelkey(&tokens[2]);
    return;
  }

  // OPCIÓN sin flags: shared cl
  key_t cl = (key_t) strtoul(tokens[1], NULL, 10);

  int id = shmget(cl, 0, 0);
  if (id == -1) {
    printf("Imposible asignar memoria compartida clave %lu: %s\n",
           (unsigned long) cl, strerror(errno));
    return;
  }

  struct shmid_ds s;
  if (shmctl(id, IPC_STAT, &s) == -1) {
    perror("shmctl");
    return;
  }

  void *p = shmat(id, NULL, 0);
  if (p == (void *) -1) {
    perror("shmat");
    return;
  }

  printf("Memoria compartida de clave %lu en %p\n",
         (unsigned long) cl, p);

  Block_Memory *bm = malloc(sizeof(Block_Memory));
  bm->addr = p;
  bm->size = s.shm_segsz;   
  bm->allocationTime = time(NULL);
  bm->type = SHARED;
  bm->key = cl;
  bm->filename = NULL;
  bm->fd = -1;

  insertItem(allocs_list, bm);

}

void cmd_mmap(char *tokens[], tList *allocs_list, tList *ficherosAbiertos) { 

  if(tokens[1] == NULL) {
    ImprimirListaMMap(allocs_list);
    return;
  }

  if(strcmp(tokens[1], "-free") == 0 && tokens[2] != NULL){
    void *pt = unmapearFichero(tokens[2], allocs_list, ficherosAbiertos);
    if (!pt)
      printf("No hay ningún bloque mapeado con ese nombre\n");
    else
      printf("Bloque en %p desmapeado\n", pt);
    return;
  }

  char *perm = tokens[2];
  int protection = 0;
  if (perm && strlen(perm) < 4) {                             //Establece los permisos de lectura, escritura y ejecución
    if (strchr(perm, 'r')) protection |= PROT_READ;
    if (strchr(perm, 'w')) protection |= PROT_WRITE;
    if (strchr(perm, 'x')) protection |= PROT_EXEC;
  }

  void *p = MapearFichero(tokens[1], protection, allocs_list, ficherosAbiertos);
  if (!p)
    perror("Imposible mapear fichero");
  else
    printf("Fichero %s mapeado en %p\n", tokens[1], p);
}

void cmd_memfill(char *tokens[]){
  void *p;
  size_t cont;
  unsigned char byte;

  if (tokens[1] == NULL || tokens[2] == NULL || tokens[3] == NULL){
    printf("Uso: memfill addr cont ch\n");
    return;
  }

  p = CadenatoPointer(tokens[1]);        //Pasamos la cadena a puntero
  if (p == NULL) {                        //Comprobamos que el puntero sea valido
    perror("Direccion invalida");
    return;
  }
  cont = (size_t) strtoul(tokens[2], NULL, 10);     //convertimos lo que ocupa a unsigned long usando base 10

  if(tokens[3][0] == '\'' && tokens[3][2] == '\'' && tokens[3][1] != '\0'){   //Comprobamos si el tercer token es un caracter entre comillas simples
    byte = (unsigned char) tokens[3][1];                                      //Si es asi, asignamos el byte al valor del caracter
  } else {
    byte = (unsigned char) strtoul(tokens[3], NULL, 10);                      //Si no, convertimos el tercer token a unsigned long usando base 10 y lo asignamos al byte
  }

  char ascii = (byte >= 32 && byte <= 126) ? byte : ' ';                      // Convertir a caracter imprimible o espacio

  printf("Llenando %zu bytes de memoria con el byte %02X(%c) a partir de la direccion %p\n", cont, byte, ascii, p);
  LlenarMemoria(p, cont, byte);
}

void cmd_memdump(char *tokens[]) {
    void *p;
    size_t cont;

    if (tokens[1] == NULL || tokens[2] == NULL) {
        printf("Uso: memdump addr cont\n");
        return;
    }

    p = CadenatoPointer(tokens[1]);       // Convertimos la cadena a puntero
    if (p == NULL) {                      // Comprobamos que el puntero sea válido
        perror("Direccion invalida");
        return;
    }

    cont = (size_t) strtoul(tokens[2], NULL, 10);   // Convertimos string a numero entero sin signo en base 10

    printf("Volcando %zu bytes desde la direccion %p\n", cont, p);

    unsigned char *mem = (unsigned char *)p;    // Convertimos el puntero a unsigned char para manipular byte a byte

    size_t offset = 0;                  //Indica cuantos bytes hemos recorrido
    while (offset < cont) {             //Mientras no hayamos recorrido todos los bytes a mostrar

        size_t line_bytes = (cont - offset >= 20) ? 20 : cont - offset;  //Calculamos cuantos bytes quedan por mostrar en esta linea (maximo 20)

        printf("%p->  ", mem + offset);         //Mostramos la direccion inicial de la linea

        for (size_t i = 0; i < line_bytes; i++) {    
            unsigned char c = mem[offset + i];       //Obtenemos el byte actual

            //Mostramos los caracteres especiales
            if (c == '\n') printf("\\n ");      
            else if (c == '\t') printf("\\t ");
            else if (c == '\r') printf("\\r ");
            else if (c >= 32 && c <= 126) printf(" %c ", c);
            else printf("   ");
        }
        printf("\n");

        printf("%p->  ", mem + offset);      //Mostramos los bytes en hexadecimal
        for (size_t i = 0; i < line_bytes; i++) {
            printf("%02X ", mem[offset + i]);
        }
        printf("\n");

        offset += line_bytes;             //Actualizamos el offset para la siguiente linea de 20 bytes
    }
}


void cmd_recurse(char *tokens[]){

  int n;
  if (tokens[1] == NULL){
    printf("Uso: recurse n\n");
    return;
  }
  n = (int) strtoul(tokens[1], NULL, 10);
  Recursiva(n);
}

void cmd_free(char *tokens[], tList *allocs_list) {
  void *addr;

  if (tokens[1] == NULL) {
    printf("Uso: free addr\n");
    return;
  }

  addr = CadenatoPointer(tokens[1]);
  if (addr == NULL) {
    perror("Direccion invalida");
    return;
  }

  for (tPosL p = first(*allocs_list); !listEnd(p); p = next(p)) {
    Block_Memory *bm = (Block_Memory *) getItem(p);

    if (bm->addr == addr) {
      // Libera según el tipo de bloque
      if (bm->type == MALLOC) {
        free(bm->addr);
      } else if (bm->type == SHARED) {
        if (shmdt(bm->addr) == -1)
          perror("Error al desacoplar memoria compartida");
      } else if (bm->type == MMAP) {
        if (munmap(bm->addr, bm->size) == -1)
          perror("Error al desmapear fichero");
        if (bm->fd != -1)
          close(bm->fd);
      }

      if (bm->filename != NULL)
        free(bm->filename);

      // Eliminar el descriptor de la lista de bloques
      deleteAtPosition(allocs_list, p);
      free(bm);

      return;
    }
  }

  printf("La direccion %s no coincide con ningun bloque conocido\n", tokens[1]);
}

void cmd_readfile (char *tokens[])
{
  void *p;
  size_t cont=-1;  // si no pasamos tamano se lee entero
  ssize_t n;
  if (tokens[1] == NULL || tokens[2] == NULL || tokens[3] == NULL){
    printf ("faltan parametros\n");
    return;
  }
  p=CadenatoPointer(tokens[2]);  // convertimos de cadena a puntero
  if (tokens[3]!=NULL)    // 1 = nombre archivo 2 = direccion 3 = cont
    cont=(size_t) atoll(tokens[3]);

  if ((n=LeerFichero(tokens[1],p,cont))==-1)
    perror ("Imposible leer fichero");
  else
    printf ("leidos %lld bytes de %s en %p\n",(long long) n,tokens[1],p);
}

void cmd_read(char *tokens[], tList *ficherosAbiertos) {

    if (tokens[1] == NULL || tokens[2] == NULL || tokens[3] == NULL){
        printf("Uso: read df addr cont\n");
        return;
    }

    int df = atoi(tokens[1]);
    if (df < 0) {
        printf("Descriptor inválido\n");
        return;
    }

    // Buscar la posición del descriptor en la lista
    tPosL pos = findDf(df, ficherosAbiertos);
    if (pos == NULL) {
        printf("Descriptor %d no está abierto\n", df);
        return;
    }

    // Obtener la estructura del fichero
    tOpenFile *f = (tOpenFile *) getItem(pos);

    // Sustituimos el df por el nombre del fichero abierto para cmd_readfile
    tokens[1] = f->nombre;

    // Llamamos al readfile que ya tienes implementado
    cmd_readfile(tokens);
}

void cmd_writefile(char *tokens[])
{
    void *p;
    size_t cont = 0;
    ssize_t n;
    int df;

    if (tokens[1] == NULL || tokens[2] == NULL || tokens[3] == NULL) {
        printf("Uso: writefile file addr cont\n");
        return;
    }

    // Dirección de memoria origen
    p = CadenatoPointer(tokens[2]);
    if (p == NULL) {
        perror("Direccion invalida");
        return;
    }

    // Número de bytes a escribir
    cont = (size_t) atoll(tokens[3]);

    // Abrimos el fichero para ESCRITURA (creando y truncando)
    df = open(tokens[1], O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if (df == -1) {
        perror("Imposible abrir fichero");
        return;
    }

    // Escritura real
    n = write(df, p, cont);
    if (n == -1) {
        perror("Imposible escribir fichero");
        close(df);
        return;
    }

    printf("escritos %lld bytes en %s desde %p\n",
           (long long) n, tokens[1], p);

    close(df);
}

void cmd_write(char *tokens[], tList *ficherosAbiertos) {

    if (tokens[1] == NULL || tokens[2] == NULL || tokens[3] == NULL){
        printf("Uso: write df addr cont\n");
        return;
    }

    int df = atoi(tokens[1]);
    if (df < 0) {
        printf("Descriptor inválido\n");
        return;
    }

    // Buscar la posición del descriptor en la lista
    tPosL pos = findDf(df, ficherosAbiertos);
    if (pos == NULL) {
        printf("Descriptor %d no está abierto\n", df);
        return;
    }

    // Obtener la estructura del fichero
    tOpenFile *f = (tOpenFile *) getItem(pos);

    // Sustituimos el df por el nombre del fichero abierto para cmd_writefile
    tokens[1] = f->nombre;

    // Llamamos al readfile que ya tienes implementado
    cmd_writefile(tokens);
}

void cmd_mem(char *tokens[], tList *allocs_list) {
    
  if (tokens[1] == NULL){
    memVars();
    memFuncs();
    ImprimirListaMemoria(allocs_list);
    return;
  }

  if (strcmp(tokens[1], "-funcs") == 0){
    memFuncs();
  }else if (strcmp(tokens[1], "-vars") == 0){
    memVars();
  }else if (strcmp(tokens[1], "-blocks") == 0){
    ImprimirListaMalloc(allocs_list);
  }else if (strcmp(tokens[1], "-all") == 0){
    memVars();
    memFuncs();
    ImprimirListaMemoria(allocs_list);
  }else if (strcmp(tokens[1], "-pmap") == 0){
    Do_pmap();
  }else{
    printf("Opcion %s no contemplada\n", tokens[1]);
  }
  
}


void cmd_handler(char *entrada, tList *historial, tList *ficherosAbiertos, tDirParams *params, tList *allocs_list){
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
    else if (strcmp(tokens[0], "malloc") == 0)
      cmd_malloc(tokens, allocs_list);
    else if (strcmp(tokens[0], "mmap") == 0)
      cmd_mmap(tokens, allocs_list, ficherosAbiertos);
    else if (strcmp(tokens[0], "shared") == 0)
      cmd_shared(tokens, allocs_list);
    else if (strcmp(tokens[0], "memfill") == 0)
      cmd_memfill(tokens);
    else if (strcmp(tokens[0], "free") == 0)
      cmd_free(tokens, allocs_list);
    else if (strcmp(tokens[0], "readfile") == 0)
      cmd_readfile(tokens);
    else if (strcmp(tokens[0], "recurse") == 0)
      cmd_recurse(tokens);
    else if (strcmp(tokens[0], "read") == 0)
      cmd_read(tokens, ficherosAbiertos);
    else if (strcmp(tokens[0], "writefile") == 0)
      cmd_writefile(tokens);
    else if (strcmp(tokens[0], "write") == 0)
      cmd_write(tokens, ficherosAbiertos);
    else if (strcmp(tokens[0], "memdump") == 0)
      cmd_memdump(tokens);
    else if (strcmp(tokens[0], "mem") == 0)
      cmd_mem(tokens, allocs_list);

    addHistorial(entradaOriginal, historial);


    if (strcmp(tokens[0], "quit") == 0 || strcmp(tokens[0], "exit") == 0 || strcmp(tokens[0], "bye") == 0) {
      for (tPosL p = first(*allocs_list); !listEnd(p); p = next(p)) {
        Block_Memory *bm = getItem(p);
        if (bm->type == MALLOC) {
          free(bm->addr);
        }
        else if (bm->type == MMAP) {
          munmap(bm->addr, bm->size);
            if (bm->fd != -1)
                close(bm->fd);
            free(bm->filename);
        }
      }
      clear(allocs_list);
      clear(ficherosAbiertos);
      clear(historial);
      exit(0);
    }
  } else {
    free(entradaOriginal);
  }
}

int main(){
  tList ficherosAbiertos;
  tList historial;
  tList allocs_list;
  CreateEmptyList(&ficherosAbiertos);
  CreateEmptyList(&historial);
  CreateEmptyList(&allocs_list);
  // Inicializa stdin, stdout, stderr
  init_standard_fds(&ficherosAbiertos);

  char entrada[MAX_TOKENS];
  while (true) {
    printf("->");
    leerEntrada(entrada);
    cmd_handler(entrada, &historial, &ficherosAbiertos, &dirParams, &allocs_list);
  }
}
