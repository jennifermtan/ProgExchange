#ifndef PE_EXCHANGE_H
#define PE_EXCHANGE_H

#include "pe_common.h"
#include "dyn_array.h"

#define LOG_PREFIX "[PEX]"

struct exchange {
    long exchange_fees;
    int n_items;
    struct dyn_array* product_ls;
    struct dyn_array* traders_ls;
};

void initialise_exchange(struct exchange* exchange);

#endif