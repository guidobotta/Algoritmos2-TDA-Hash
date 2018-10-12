#include "hash.h"
#include <stdbool.h>
#include <stddef.h>

/* ******************************************************************
 *                DEFINICION DE LOS TIPOS DE DATOS
 * *****************************************************************/

 typedef struct hash_campo{
     char *clave;
     void *valor;
     estado_t estado;
 }hash_campo_t;

 struct hash {
     size_t cantidad;
     size_t largo;
     size_t carga;
     hash_campo_t *tabla;
     hash_destruir_dato_t destruir_dato;
 };

 /* ******************************************************************
  *                      PRIMITIVAS PRIVADAS
  * *****************************************************************/
//https://stackoverflow.com/questions/32795453/use-of-murmurhash-in-c
uint32_t murmur3_32(const char *key, uint32_t len, uint32_t seed) {
      static const uint32_t c1 = 0xcc9e2d51;
      static const uint32_t c2 = 0x1b873593;
      static const uint32_t r1 = 15;
      static const uint32_t r2 = 13;
      static const uint32_t m = 5;
      static const uint32_t n = 0xe6546b64;

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

size_t hash_cantidad(const hash_t *hash);

void hash_destruir(hash_t *hash);

/* ******************************************************************
 *                PRIMITIVAS DEL ITERADOR HASH
 * *****************************************************************/

hash_iter_t *hash_iter_crear(const hash_t *hash);

bool hash_iter_avanzar(hash_iter_t *iter);

const char *hash_iter_ver_actual(const hash_iter_t *iter);

bool hash_iter_al_final(const hash_iter_t *iter);

void hash_iter_destruir(hash_iter_t* iter);
