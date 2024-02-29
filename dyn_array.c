#include "dyn_array.h"

/* USYD CODE CITATION ACKNOWLEDGEMENT
* I declare that the following lines of code have been copied with changes from the
* website titled: "Angus's Tutorial Thursday/Tutorial 4/q5.c" and it is not my own work. 
* 
* Original URL
* https://edstem.org/au/courses/10466/workspaces/p22cx2yFA0bfgC52NzNoIoQcNT79cds8
* Last access May, 2023
*/
void dyn_resize(struct dyn_array* dyn) {
    // Double the size of the internal array.
    dyn->capacity *= 2;
    dyn->array = realloc(dyn->array, dyn->capacity * sizeof(void*));

    if (dyn->array == NULL) {
        perror("dyn_resize()");
        exit(1);
    }
}

struct dyn_array* dyn_array_init() {
    // Initialise a new dynamic array.
    struct dyn_array* dyn = calloc(1, sizeof(struct dyn_array));
    dyn->capacity = INITIAL_CAPACITY;
    dyn->array = malloc(dyn->capacity * sizeof(void*));
    
    if (dyn->array == NULL) {
        perror("dyn_array_init()");
        exit(1);
    }
    return dyn;
}

void dyn_array_add(struct dyn_array* dyn, void* ptr) {
    // Add a value to the end of the dynamic array.
    if (dyn->size + 1 > dyn->capacity) {
        // We need to resize.
        dyn_resize(dyn);
    }
    dyn->array[dyn->size] = ptr;
    dyn->size++;
}

void dyn_array_delete(struct dyn_array* dyn, int index) {
    // We shift everything to the right of the index left by one.
    int i = index;
    while (i < dyn->size - 1) {
        dyn->array[i] = dyn->array[i + 1];
        i++;
    }
    dyn->size--;
}

void** dyn_array_get(struct dyn_array* dyn, int index) {
    // Check the bound on the index.
    if (!(0 <= index && index < dyn->size)) {
        fprintf(stderr, "dyn_array_get(): Tried to access index out of bounds.");
        exit(1);
    }
    return dyn->array[index];
}

void dyn_array_free(struct dyn_array* dyn) {
    for (int i = 0; i < dyn->size; i++) {
        free(dyn->array[i]);
    }
    free(dyn->array);
    free(dyn);
}
/* end of copied code */
void dyn_array_add_order(struct dyn_array* price_ls, struct dyn_array* orders_ls, long* price, struct dyn_array* price_order, struct order* order) {
    // Add a value to the end of the dynamic array.
    if (price_ls->size + 1 > price_ls->capacity) {
        // We need to resize.
        dyn_resize(price_ls);
        dyn_resize(orders_ls);
    }

    int pos = 0;
    for (int i = 0; i < price_ls->size; i++) {
        if (*((int*) price_ls->array[i]) < *price) {
            pos++;
        } else {
            break;
        }
    }

    dyn_array_add(price_order, order);
    // Shift elements greater than price to the right
    for (int i = price_ls->size - 1; i >= pos; i--) {
        price_ls->array[i + 1] = price_ls->array[i];
        orders_ls->array[i + 1] = orders_ls->array[i];
    }
    // Insert elements at the position
    price_ls->array[pos] = price;
    orders_ls->array[pos] = price_order;
    price_ls->size++;
    orders_ls->size++;
}

void dyn_array_free_traders(struct dyn_array* dyn) {
    for (int i = 0; i < dyn->size; i++) {
        free(((struct trader*)dyn->array[i])->binary);
        free(((struct trader*)dyn->array[i])->exchange_pipe);
        free(((struct trader*)dyn->array[i])->trader_pipe);
        dyn_array_free(((struct trader*)dyn->array[i])->value_ls);
        dyn_array_free(((struct trader*)dyn->array[i])->qty_ls);
    }
}

void dyn_array_free_trader(struct trader* trader) {
    free(trader->binary);
    free(trader->exchange_pipe);
    free(trader->trader_pipe);
    dyn_array_free(trader->qty_ls);
    dyn_array_free(trader->value_ls);
    free(trader);
}

void dyn_array_free_array_only(struct dyn_array* dyn) {
    free(dyn->array);
    free(dyn);
}

void dyn_array_free_products(struct dyn_array* product_ls) {
    for (int i = 0; i < product_ls->size; i++) {
        struct product* product = product_ls->array[i];
        // Free dyn_array of struct order in orders_ls
        for (int j = 0; j < product->buy->orders_ls->size; j++) {
            struct dyn_array* product_order_buy = product->buy->orders_ls->array[j];
            dyn_array_free(product_order_buy);
        }
        for (int j = 0; j < product->sell->orders_ls->size; j++) {
            struct dyn_array* product_order_sell = product->sell->orders_ls->array[j];
            dyn_array_free(product_order_sell);
        }
        free(product->prod_name);
        dyn_array_free(product->buy->price_ls);
        dyn_array_free_array_only(product->buy->orders_ls);
        dyn_array_free(product->sell->price_ls);
        dyn_array_free_array_only(product->sell->orders_ls);
        free(product->buy);
        free(product->sell);
    }
}