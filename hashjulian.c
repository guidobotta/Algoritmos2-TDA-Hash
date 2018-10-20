#include "hash.h"
#include <stdbool.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>

#define TAM 37
#define FACTOR_AUMENTO 0.7
#define FACTOR_DISMINUCION 0.2
#define VACIO 'V'
#define OCUPADO 'O'
#define BORRADO 'B'
#define AUMENTAR true
#define DISMINUIR false
#define TAM_PRIMOS 26

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
//Declaro la primitiva para evitar conflictos con las demas funciones
bool _hash_guardar_(hash_t *hash, const char *clave, void *dato, bool redimension);

////
//FUNCION DE HASHING
////

//https://stackoverflow.com/questions/32795453/use-of-murmurhash-in-c
unsigned int _hashing_(const char *key) {

    static const unsigned int c1 = 0xcc9e2d51;
    static const unsigned int c2 = 0x1b873593;
    static const unsigned int r1 = 15;
    static const unsigned int r2 = 13;
    static const unsigned int m = 5;
    static const unsigned int n = 0xe6546b64;

    unsigned int len = (unsigned int)strlen(key);
    unsigned int seed = 42; //Un numero aleatorio, pero constante
    unsigned int hash = seed;

    const int nblocks = len / 4;
    const unsigned int *blocks = (const unsigned int *) key;
    int i;
    for (i = 0; i < nblocks; i++) {
        unsigned int k = blocks[i];
        k *= c1;
        k = (k << r1) | (k >> (32 - r1));
        k *= c2;

        hash ^= k;
        hash = ((hash << r2) | (hash >> (32 - r2))) * m + n;
    }

    const unsigned char *tail = (const unsigned char *) (key + nblocks * 4);
    unsigned int k1 = 0;

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

unsigned int hashing(const char *key, const hash_t* hash){
    return _hashing_(key) % (unsigned int)hash->largo;
}


////
//FUNCION DE CALCULADO DE POSICION
////

// Devuelve -1 si no se encuentra la clave o la posicion de la clave.
// Asigna la posicion del primer borrado, si hay, a borrado.
// Asigna la posicion del vacio en caso de no estar el elemento.
int _calcular_posicion_(hash_campo_t* tabla, int fact, int posicion, const char* clave, int* borrado, int* vacio, size_t largo){
    if((*borrado == -1) && (tabla[posicion].estado == BORRADO)){
        *borrado = posicion;if(posicion < 0) posicion *= -1;
    }
    else if(tabla[posicion].estado == VACIO){
        *vacio = posicion;
        return -1;
    }
    else if(tabla[posicion].clave){
        if(!strcmp(tabla[posicion].clave,clave)) return posicion;
    }
    fact++;

    return _calcular_posicion_(tabla, fact, (posicion+(fact*fact))%(int)largo, clave, borrado, vacio, largo);
}

int calcular_posicion(hash_campo_t* tabla, int posicion, const char* clave, int* borrado, int* vacio, size_t largo){
    int fact = 0;
    return _calcular_posicion_(tabla, fact, posicion, clave, borrado, vacio, largo);
}

void inicializar_tabla(hash_campo_t *tabla, size_t tam){
    hash_campo_t campo;
    campo.clave = NULL;
    campo.valor = NULL;
    campo.estado = VACIO;

    for(int i=0; i<tam; i++){
        tabla[i] = campo; //Asigno el mismo campo vacio;
    }
}

////
//FUNCION DE REDIMENSION
////

void modificar_campo(hash_t* hash, void *dato, const char *clave, int posicion, bool redimension){
    if(!redimension){
        char *clave_dinamica = malloc(sizeof(char)*strlen(clave)+1);
        strcpy(clave_dinamica, clave);
        hash->tabla[posicion].clave = clave_dinamica;
    }else{
        hash->tabla[posicion].clave = (char*)clave;
    }
    hash->tabla[posicion].valor = dato;
    hash->tabla[posicion].estado = OCUPADO;
}

size_t nuevo_largo(hash_t *hash, bool aumentar){
    size_t vector[TAM_PRIMOS] = {37, 79, 163, 331, 673, 1361, 2729, 5471, 10949, 21911, 43853, 87719, 175447, 350899, 701819, 1403641, 2807303, 5614657, 11229331, 22458671, 449117381, 89834777, 179669557, 359339171, 718678369, 1437356741};//Primos
    int i=0;
    if(hash->largo >= vector[TAM_PRIMOS-1]){
        if(aumentar) return hash->largo*2;
        return hash->largo/2;
    }
    for(i=0; i<TAM_PRIMOS; i++){
        if(vector[i] == hash->largo){
            break;
        }
    }

    if(aumentar){
        i++;
    }else{
        i -= 2;
    }
    if(i >= TAM_PRIMOS) return hash->largo*2;
    if(i<0) return vector[0];

    return vector[i];
}

bool redimensionar_hash(hash_t *hash, bool aumentar){

    size_t tam_nuevo = nuevo_largo(hash, aumentar); //Le pasamos false para disminuir el largo y true para aumentar
    hash_campo_t* tabla_nueva = malloc(sizeof(hash_campo_t)*tam_nuevo);
    size_t tope = hash->largo;
    if(!tabla_nueva) return false;

    hash_campo_t* tabla_vieja = hash->tabla;
    inicializar_tabla(tabla_nueva, tam_nuevo);
    hash->tabla = tabla_nueva;
    hash->largo = tam_nuevo; //Debe haber una forma mejor de hacerlo, es para que no redimensione nuevamente dentro de hash_guardar
    hash->carga = ((float)hash->cantidad)/(float)hash->largo;
    hash->cantidad = 0;

    for(int i=0; i<tope; i++){
        //Cuidado. Hay que ver como se inicializa cada campo de la tabla
        if(tabla_vieja[i].clave){
            if(!_hash_guardar_(hash, tabla_vieja[i].clave, tabla_vieja[i].valor, true)){
                free(tabla_nueva);
                hash->tabla = tabla_vieja;
                hash->largo = tope;
                hash->carga = ((float)hash->cantidad)/(float)hash->largo;
                return false;
            }
        }
    }

    hash->carga = ((float)hash->cantidad)/(float)hash->largo;
    free(tabla_vieja);

    return true;
}

bool _hash_guardar_(hash_t *hash, const char *clave, void *dato, bool redimension){

    if(hash->carga >= FACTOR_AUMENTO && !redimension){
        if(!redimensionar_hash(hash, AUMENTAR)) return false;//Le pasamos DISMINUIR si hay que achicar
    }

    int indice = hashing(clave, hash);
    int borrado = -1;
    int vacio = -1;
    int posicion = calcular_posicion(hash->tabla, indice, clave, &borrado, &vacio, hash->largo);


    if(posicion == -1){
        if(borrado != -1){ //ASIGNAR CLAVE EN BORRADO
            modificar_campo(hash, dato, clave, borrado, redimension);
        }
        else{ //ASIGNAR CLAVE EN VACIO
            modificar_campo(hash, dato, clave, vacio, redimension);
        }
    }
    else{
        if(hash->destruir_dato){
            hash->destruir_dato(hash->tabla[posicion].valor);
        }
        free(hash->tabla[posicion].clave);
        modificar_campo(hash, dato, clave, posicion, redimension);
        return true;
    } //PISAR CLAVE ANTERIOR

    hash->cantidad++;
    hash->carga = ((float)hash->cantidad)/(float)hash->largo;

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

    hash_campo_t* tabla = malloc(sizeof(hash_campo_t)*TAM);

    if (!tabla){
        free(hash);
        return NULL;
    }

    inicializar_tabla(tabla, TAM);
    hash->cantidad = 0;
    hash->largo = TAM;
    hash->carga = 0;
    hash->tabla = tabla;
    hash->destruir_dato = destruir_dato;

    return hash;
}

bool hash_guardar(hash_t *hash, const char *clave, void *dato){
    return _hash_guardar_(hash, (char*)clave, dato, false);
}

void *hash_borrar(hash_t *hash, const char *clave){
    int posicion = 0;
    int vacio = 0;
    void *dato = NULL;
    int borrado = 0;
    int indice = hashing(clave, hash);
    if((posicion = calcular_posicion(hash->tabla, indice, clave, &borrado, &vacio, hash->largo)) == -1) return NULL;
    free(hash->tabla[posicion].clave);
    hash_campo_t campo;
    campo.clave = NULL;
    campo.valor = NULL;
    campo.estado = BORRADO;

    dato = hash->tabla[posicion].valor;
    hash->tabla[posicion] = campo;

    hash->cantidad--;
    hash->carga = ((float)hash->cantidad)/(float)hash->largo;

    if(hash->carga <= FACTOR_DISMINUCION && hash->largo > TAM){
        if(!redimensionar_hash(hash, DISMINUIR)) return false;
    }
    return dato;

}

void *hash_obtener(const hash_t *hash, const char *clave){
    int indice = hashing(clave, hash);
    int borrado = 0;
    int vacio = -1;
    int posicion = calcular_posicion(hash->tabla, indice, clave, &borrado, &vacio, hash->largo);

    if (posicion == -1) return NULL;
    return hash->tabla[posicion].valor;
}

bool hash_pertenece(const hash_t *hash, const char *clave){
    int indice = hashing(clave, hash);
    int borrado = 0;
    int vacio = -1;
    if(calcular_posicion(hash->tabla, indice, clave, &borrado, &vacio, hash->largo) != -1) return true;
    return false;
}

size_t hash_cantidad(const hash_t *hash){
    return hash->cantidad;
}

void hash_destruir(hash_t *hash){
    if(hash->destruir_dato){
        for(int i=0; i < hash->largo; i++){
            //Si el estado no es ocupado no hago nada
            if(hash->tabla[i].estado == OCUPADO){
                free(hash->tabla[i].clave);
                hash->destruir_dato(hash->tabla[i].valor);
            }
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

     hash_iter->hash = (hash_t*)hash;
     hash_iter->posicion = 0;

     return hash_iter;
 }

 bool hash_iter_al_final(const hash_iter_t *iter){
     return(iter->posicion >= iter->hash->largo); //Chequear
 }

 bool hash_iter_avanzar(hash_iter_t *iter){
     iter->posicion++;
     while(!hash_iter_al_final(iter)){
         if(iter->hash->tabla[iter->posicion].estado == OCUPADO) return true;
         //Devuelve true si encuentra un campo ocupado
         iter->posicion++;
     }
     //Devuelve false si llegó al final del hash
     return false;
 }

 const char *hash_iter_ver_actual(const hash_iter_t *iter){
     if(hash_iter_al_final(iter)) return NULL;
     return iter->hash->tabla[iter->posicion].valor;
 }

 void hash_iter_destruir(hash_iter_t* iter){
     free(iter);
 }
