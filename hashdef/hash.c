#include "hash.h"
#include <stdbool.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>

#define TAM 37
#define FACTOR_AUMENTO 0.57
#define FACTOR_DISMINUCION 0.1
#define VACIO 'V'
#define OCUPADO 'O'
#define BORRADO 'B'
#define AUMENTAR true
#define DISMINUIR false
#define TAM_PRIMOS 26
//Vector de numeros primos para el tamaño de la tabla.
/*const size_t vector[TAM_PRIMOS] = {37, 79, 163, 331, 673, 1361, 2729, 5471, 10949, 21911, 43853, 87719, 175447, 350899, 701819,
    1403641, 2807303, 5614657, 11229331, 22458671, 449117381, 89834777, 179669557, 359339171, 718678369, 1437356741};
*/

/* ******************************************************************+
 *                DEFINICION DE LOS TIPOS DE DATOS
 * *****************************************************************/

typedef struct hash_campo{
    char *clave;
    void *valor;
    char estado;
}hash_campo_t;

struct hash{
    size_t cantidad;
    size_t largo;
    size_t pos_tam;
    float carga;
    hash_campo_t *tabla;
    hash_destruir_dato_t destruir_dato;
};

 /* ******************************************************************
  *                      FUNCIONES PRIVADAS
  * *****************************************************************/

//Declaro las funciones para evitar conflictos con las demas funciones

unsigned int hashing(const char *key, const hash_t* hash);
int calcular_posicion(hash_campo_t* tabla, int fact, int posicion, const char* clave, int* vacio, size_t largo);
size_t nuevo_largo(hash_t *hash, bool aumentar);
void inicializar_tabla(hash_campo_t *tabla, size_t tam);
bool redimensionar_hash(hash_t *hash, bool aumentar);
void modificar_campo(hash_t* hash, void *dato, const char *clave, int posicion, bool redimension);
bool _hash_guardar_(hash_t *hash, const char *clave, void *dato, bool redimension);

////
//FUNCION DE HASHING
////
//Funcion de hash djb2 obtenida de internet.
size_t funcion_hash(unsigned char *str){
    size_t hash = 5381;
    int c;

    while ((c = *str++))
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

    return hash;
}
//Funcion de hash K&R obtenida de internet.
size_t funcion_hash2(unsigned char *s)
{
    size_t hashval;

    for (hashval = 0; *s != '\0'; s++)
        hashval = *s + 31*hashval;
    return hashval;
}

unsigned int hashing(const char *key, const hash_t* hash){
    return (unsigned int)(funcion_hash2((unsigned char*)key) % hash->largo);
}

////
//FUNCION DE CALCULADO DE POSICION
////

// Devuelve -1 si no se encuentra la clave o la posicion de la clave.
// Asigna la posicion del primer borrado, si hay, a borrado.
// Asigna la posicion del vacio en caso de no estar el elemento.

int calcular_posicion(hash_campo_t* tabla, int fact, int posicion, const char* clave, int* vacio, size_t largo){
    if(tabla[posicion].estado == VACIO){
        *vacio = posicion;
        return -1;
    }
    else if(tabla[posicion].clave){
        if(!strcmp(tabla[posicion].clave,clave)) return posicion;
    }
    fact++;

    return calcular_posicion(tabla, fact, (int)((posicion+(fact*fact)) % largo), clave, vacio, largo);
}

// Inicializar una tabla

void inicializar_tabla(hash_campo_t *tabla, size_t tam){
    hash_campo_t campo;
    campo.clave = NULL;
    campo.valor = NULL;
    campo.estado = VACIO;

    for(int i=0; i<tam; i++){
        tabla[i] = campo; //Asigno el mismo campo vacio;
    }
}

// Redimensionar Hash

bool redimensionar_hash(hash_t *hash, bool aumentar){
    size_t tam_nuevo = 0;
    if(aumentar) tam_nuevo = (size_t)((double)hash->largo*(2.7)); //Duplico el tamaño
    else{
       tam_nuevo = hash->largo/4; //Divido entre 4
    }
    hash_campo_t* tabla_nueva = malloc(sizeof(hash_campo_t)*tam_nuevo);
    size_t tope = hash->largo;
    if(!tabla_nueva) return false;

    hash_campo_t* tabla_vieja = hash->tabla;
    inicializar_tabla(tabla_nueva, tam_nuevo);
    hash->tabla = tabla_nueva;
    hash->largo = tam_nuevo;
    hash->carga = ((float)hash->cantidad)/(float)hash->largo;
    hash->cantidad = 0;

    for(int i=0; i<tope; i++){
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

///
//FUNCION DE GUARDAR
///

// Modificar campo

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

// Hash guardar

bool _hash_guardar_(hash_t *hash, const char *clave, void *dato, bool redimension){

    if(hash->carga >= FACTOR_AUMENTO && !redimension){
        if(!redimensionar_hash(hash, AUMENTAR)) return false;//Le pasamos DISMINUIR si hay que achicar
    }

    int indice = hashing(clave, hash);
    int vacio = -1;
    int posicion = calcular_posicion(hash->tabla, 0, indice, clave, &vacio, hash->largo);

    if(posicion == -1){
        //ASIGNAR CLAVE EN VACIO
        modificar_campo(hash, dato, clave, vacio, redimension);
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
    int indice = hashing(clave, hash);
    if((posicion = calcular_posicion(hash->tabla, 0, indice, clave, &vacio, hash->largo)) == -1) return NULL;
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
    int vacio = -1;
    int posicion = calcular_posicion(hash->tabla, 0, indice, clave, &vacio, hash->largo);

    if (posicion == -1) return NULL;
    return hash->tabla[posicion].valor;
}

bool hash_pertenece(const hash_t *hash, const char *clave){
    int indice = hashing(clave, hash);
    int vacio = -1;
    if(calcular_posicion(hash->tabla, 0, indice, clave, &vacio, hash->largo) != -1) return true;
    return false;
}

size_t hash_cantidad(const hash_t *hash){
    return hash->cantidad;
}

void hash_destruir(hash_t *hash){
    for(int i=0; i < hash->largo; i++){
        //Si el estado no es ocupado no hago nada
        if(hash->tabla[i].estado == OCUPADO){
            free(hash->tabla[i].clave);
            if(hash->destruir_dato) hash->destruir_dato(hash->tabla[i].valor);
        }
    }

    free(hash->tabla);
    free(hash);
}

/* ******************************************************************
 *                DEFINICION DEL STRUCT HASH
 * *****************************************************************/

 struct hash_iter{
    hash_t* hash;
    size_t posicion;
 };

/* ******************************************************************
 *                PRIMITIVAS DEL ITERADOR HASH
 * *****************************************************************/

hash_iter_t *hash_iter_crear(const hash_t *hash){
    hash_iter_t* hash_iter = malloc(sizeof(hash_iter_t));
    if (!hash_iter) return NULL;

    hash_iter->hash = (hash_t*)hash;
    hash_iter->posicion = 0;
    if(!hash_iter_ver_actual(hash_iter)) hash_iter_avanzar(hash_iter);

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
    return iter->hash->tabla[iter->posicion].clave;
}

void hash_iter_destruir(hash_iter_t* iter){
    free(iter);
}
