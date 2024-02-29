#ifndef DYN_ARRAY_H
#define DYN_ARRAY_H

/* USYD CODE CITATION ACKNOWLEDGEMENT
* I declare that the following lines of code have been copied from the
* website titled: "Angus's Tutorial Thursday/Tutorial 4/q5.c" and it is not my own work. 
* 
* Original URL
* https://edstem.org/au/courses/10466/workspaces/p22cx2yFA0bfgC52NzNoIoQcNT79cds8
* Last access May, 2023
*/

#include "pe_common.h"

#define INITIAL_CAPACITY (2)

struct dyn_array {
    int size;
    int capacity;
    void** array;
};

extern void dyn_resize(struct dyn_array* dyn);
extern struct dyn_array* dyn_array_init();
extern void dyn_array_add(struct dyn_array* dyn, void* ptr);
extern void dyn_array_delete(struct dyn_array* dyn, int index);
extern void** dyn_array_get(struct dyn_array* dyn, int index);
extern void dyn_array_free(struct dyn_array* dyn);
/* end of copied code */
extern void dyn_array_add_order(struct dyn_array* price_ls, struct dyn_array* orders_ls, long* price, struct dyn_array* price_order, struct order* order);
extern void dyn_array_free_traders(struct dyn_array* dyn);
extern void dyn_array_free_trader(struct trader* trader);
void dyn_array_free_array_only(struct dyn_array* dyn);
extern void dyn_array_free_products(struct dyn_array* product_ls);

#endif
