#include <stdio.h>
#include <stdlib.h>
#include "hash.h"
#include "testing.h"

static void prueba_hash_volumen(size_t largo)
{
    hash_t* hash = hash_crear(free);

    const size_t largo_clave = 10;
    char (*claves)[largo_clave] = malloc(largo * largo_clave);

    unsigned* valores[largo];

    /* Inserta 'largo' parejas en el hash */
    bool ok = true;
    for (unsigned i = 0; i < largo; i++) {
        valores[i] = malloc(sizeof(int));
        sprintf(claves[i], "%08d", i);
        *valores[i] = i;
        ok = hash_guardar(hash, claves[i], valores[i]);
        if (!ok) break;
    }

    print_test("Prueba hash almacenar muchos elementos", ok);
    print_test("Prueba hash la cantidad de elementos es correcta", hash_cantidad(hash) == largo);
    printf("\nLas colisiones al ingresar %lu elementos fueron: %lu\n", largo, ver_colisiones());
    hash_destruir(hash);
    free(claves);
}





int main(int argc, char* argv[]){

    prueba_hash_volumen(atoi(argv[1]));
    return 0;
}
