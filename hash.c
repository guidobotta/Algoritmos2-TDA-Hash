#include "hash.h"
#include <stdbool.h>
#include <stddef.h>

/* ******************************************************************
 *                DEFINICION DE LOS TIPOS DE DATOS
 * *****************************************************************/

struct hash;
struct hash_iter;
//No se por que hay dos strucs hash pero esta asi en el .h
typedef struct hash hash_t;
typedef struct hash_iter hash_iter_t;

typedef void (*hash_destruir_dato_t)(void *);

/* ******************************************************************
 *                      PRIMITIVAS DEL HASH
 * *****************************************************************/

hash_t *hash_crear(hash_destruir_dato_t destruir_dato){
    hast_t* hash = malloc(sizeof(hash_t)*TAM); //#define TAM 33
    if (!hash) return NULL;

    hash_campo_t* tabla = malloc(sizeof(hash_campo_t));
    if (!tabla){
        free(hash);
        return NULL;
    }
    tabla->clave = NULL;
    tabla->valor = NULL;
    tabla->estado = false;

    hash->cantidad = 0; 
    hash->largo = 0; //MODIFICAR
    hash->carga = 0;
    hash->tabla = tabla;
    hash->destruir_dato = destruir_dato;

    return hash;
}

bool hash_guardar(hash_t *hash, const char *clave, void *dato);

void *hash_borrar(hash_t *hash, const char *clave);

void *hash_obtener(const hash_t *hash, const char *clave);

bool hash_pertenece(const hash_t *hash, const char *clave);

size_t hash_cantidad(const hash_t *hash){
    return hash->cantidad;
}

void hash_destruir(hash_t *hash);

/* ******************************************************************
 *                PRIMITIVAS DEL ITERADOR HASH
 * *****************************************************************/

hash_iter_t *hash_iter_crear(const hash_t *hash);

bool hash_iter_avanzar(hash_iter_t *iter);

const char *hash_iter_ver_actual(const hash_iter_t *iter);

bool hash_iter_al_final(const hash_iter_t *iter);

void hash_iter_destruir(hash_iter_t* iter);
