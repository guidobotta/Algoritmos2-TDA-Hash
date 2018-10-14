#include "hash.h"
#include <stdbool.h>
#include <stdlib.h>
#include <stddef.h>

#define TAM 33
#define FACTOR_LIMITE 0,7
#define VACIO '0'
#define OCUPADO '1'
#define BORRADO '2'

/* ******************************************************************
 *                DEFINICION DE LOS TIPOS DE DATOS
 * *****************************************************************/

 typedef struct hash_campo{
    char *clave;
    void *valor;
    char estado; //0 = vacio, 1 = ocupado, 2 = borrado;
 }hash_campo_t;

 struct hash{
    size_t cantidad;
    size_t largo;
    float carga;
    hash_campo_t *tabla;
    hash_destruir_dato_t destruir_dato;
 };

 /* ******************************************************************
  *                      PRIMITIVAS PRIVADAS
  * *****************************************************************/

////
//FUNCION DE HASHING
////

//https://stackoverflow.com/questions/32795453/use-of-murmurhash-in-c
uint32_t hashing(const char *key) {

    static const uint32_t c1 = 0xcc9e2d51;
    static const uint32_t c2 = 0x1b873593;
    static const uint32_t r1 = 15;
    static const uint32_t r2 = 13;
    static const uint32_t m = 5;
    static const uint32_t n = 0xe6546b64;

    uint32_t len = strlen(key);
    uint32_t seed = 42 //Un numero aleatorio, pero constante
    uint32_t hash = seed;

    const int nblocks = len / 4;
    const uint32_t *blocks = (const uint32_t *) key;
    int i;
    for (i = 0; i < nblocks; i++) {
        uint32_t k = blocks[i];
        k *= c1;
        k = (k << r1) | (k >> (32 - r1));
        k *= c2;

        hash ^= k;
        hash = ((hash << r2) | (hash >> (32 - r2))) * m + n;
    }

    const uint8_t *tail = (const uint8_t *) (key + nblocks * 4);
    uint32_t k1 = 0;

    switch (len & 3) {
    case 3:
        k1 ^= tail[2] << 16;
    case 2:
        k1 ^= tail[1] << 8;
    case 1:
        k1 ^= tail[0];

        k1 *= c1;
        k1 = (k1 << r1) | (k1 >> (32 - r1));
        k1 *= c2;
        hash ^= k1;
    }
    hash ^= len;
    hash ^= (hash >> 16);
    hash *= 0x85ebca6b;
    hash ^= (hash >> 13);
    hash *= 0xc2b2ae35;
    hash ^= (hash >> 16);

   return hash;
}

////
//FUNCION DE CALCULADO DE POSICION
////

// Devuelve -1 si no se encuentra la clave o la posicion de la clave.
// Asigna la posicion del primer borrado, si hay, a borrado.
// Asigna la posicion del vacio en caso de no estar el elemento.
int calcular_posicion(hash_campo_t* tabla, int fact, int posicion, char* clave, int* borrado, int* vacio, size_t largo){
    if((*borrado == -1) && (tabla[posicion]->estado == BORRADO)){
        *borrado = posicion;
    }
    else if(tabla[posicion]->estado == VACIO){
        *vacio = posicion;
        return -1;
    }
    else if(tabla[posicion] == clave) return posicion;
    fact++;
    return calcular_posicion(tabla, fact, (posicion+(fact*fact))%largo, clave, borrado, largo);
}

////
//FUNCION DE REDIMENSION
////

bool redimensionar_hash(hash_t *hash){

    size_t tam_nuevo = hash->largo*3; //Acá podemos llamar a una funcion que devuelva un numero primo como tam_nuevo
    hash_campo_t* tabla_nueva = malloc(sizeof(hash_campo_t)*tam_nuevo);
    size_t tope = hash->largo;
    if(!tabla_nueva) return false;

    hash_campo_t* tabla_vieja = hash->tabla;
    hash->tabla = tabla_nueva;
    hash->largo = tam_nuevo; //Debe haber una forma mejor de hacerlo, es para que no redimensione nuevamente dentro de hash_guardar
    
    for(int i=0; i<tope; i++){
        //Cuidado. Hay que ver como se inicializa cada campo de la tabla
        if(!hash_guardar(hash_nuevo, tabla_vieja[i]->clave, tabla_vieja[i]->dato)){
            free(tabla_nueva);
            hash->tabla = tabla_vieja;
            hash->largo = tope;
            hash->carga = hash->cantidad/hash->largo; 
            return false;
        }
    }
    
    hash->carga = hash->cantidad/hash->largo;
    free(tabla_vieja);

    return true;
}

/* ******************************************************************
 *                DEFINICION DEL STRUCT HASH
 * *****************************************************************/

 struct hash_iter{
    hash_t* hash;
    size_t posicion;
 };

/* ******************************************************************
 *                      PRIMITIVAS DEL HASH
 * *****************************************************************/

hash_t *hash_crear(hash_destruir_dato_t destruir_dato){

    hash_t* hash = malloc(sizeof(hash_t));
    if (!hash) return NULL;

    hash_campo_t* tabla = malloc(sizeof(hash_campo_t)*TAM); //#define TAM 33

    if (!tabla){
        free(hash);
        return NULL;
    }

    hash_campo_t campo;
    campo->clave = NULL;
    campo->valor = NULL;
    campo->estado = VACIO;

    for(int i=0, i<TAM, i++){
        tabla[i] = campo; //Asigno el mismo campo vacio;
    }

    hash->cantidad = 0;
    hash->largo = TAM;
    hash->carga = 0;
    hash->tabla = tabla;
    hash->destruir_dato = destruir_dato;

    return hash;
}

void modificar_campo(hash_t* hash, hash_campo_t* tabla, int posicion){
    hash->tabla[posicion]->clave = clave;
    hash->tabla[posicion]->valor = dato;
    hash->tabla[posicion]->estado = OCUPADO;
}

bool hash_guardar(hash_t *hash, const char *clave, void *dato){

    if(hash->carga >= FACTOR_LIMITE){
        if(redimensionar_hash(hash)) return false;
    }

    int indice = hashing(clave);
    int borrado = -1;
    int vacio = -1;

    int posicion = calcular_posicion(hash->tabla, indice, clave, &borrado, &vacio, hash->largo);
 
    if(posicion == -1){
        //ASIGNAR CLAVE EN BORRADO
        if(borrado != -1) modificar_campo(hash, hash->tabla, borrado);
        //ASIGNAR CLAVE EN VACIO
        else modificar_campo(hash, hash->tabla, vacio); 
               
    }else hash->tabla[posicion]->valor = dato; //PISAR CLAVE ANTERIOR

    /*hash->tabla[indice]->estado = OCUPADO;
    strcpy(hash->tabla[indice]->clave, clave); //Es necesario?
    hash->tabla[indice]->valor = dato;
    hash->cantidad++;

    hash->carga = hash->cantidad/hash->largo;*/

    return true;
}

void *hash_borrar(hash_t *hash, const char *clave);

void *hash_obtener(const hash_t *hash, const char *clave){
    int indice = hashing(clave);
    int borrado = 0;
    int vacio = -1;
    int posicion = (calcular_posicion(hash->tabla, indice, clave, borrado, vacio, hash->largo);

    if (posicion == -1) return NULL;
    return hash->tabla[posicion]->valor;
}

bool hash_pertenece(const hash_t *hash, const char *clave){
    int indice = hashing(clave);
    int borrado = 0;
    int vacio = -1;
    if(calcular_posicion(hash->tabla, indice, clave, borrado, vacio, hash->largo) != -1) return true;
    return false;
}

size_t hash_cantidad(const hash_t *hash){
    return hash->cantidad;
}

void hash_destruir(hash_t *hash){
    for(int i=0; i < hash->largo; i++){
        //Si el estado no es ocupado no hago nada
        if(hash->tabla[i]->estado == OCUPADO){
            hash->destruir_dato(hash->tabla[i]->clave);
            hash->destruir_dato(hash->tabla[i]->dato);
        }
    }
    free(hash->tabla);
    free(hash);
}

/* ******************************************************************
 *                PRIMITIVAS DEL ITERADOR HASH
 * *****************************************************************/

hash_iter_t *hash_iter_crear(const hash_t *hash){
    hash_iter_t* hash_iter = malloc(sizeof(hash_iter_t));
    if (hash_iter) return NULL;

    //COMPLETAR

    return hash_iter;
}

bool hash_iter_avanzar(hash_iter_t *iter);

const char *hash_iter_ver_actual(const hash_iter_t *iter);

bool hash_iter_al_final(const hash_iter_t *iter);

void hash_iter_destruir(hash_iter_t* iter);
