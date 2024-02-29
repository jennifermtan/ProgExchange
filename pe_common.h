#ifndef PE_COMMON_H
#define PE_COMMON_H

#define _DEFAULT_SOURCE
#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <math.h>

#define FIFO_EXCHANGE "/tmp/pe_exchange_%d"
#define FIFO_TRADER "/tmp/pe_trader_%d"
#define FEE_PERCENTAGE (1)
#define FIFO_SIZE (32)
#define COMMAND_SIZE (43)
#define PRODUCT_SIZE (18)

struct trader {
    pid_t pid;
    int id;
    char* binary;
    char* exchange_pipe;
    char* trader_pipe;
    int exchange_fd;
    int trader_fd;
    struct dyn_array* qty_ls; // This is hashed with product_ls
    struct dyn_array* value_ls; // This is hashed with qty_ls
    int disconnected;
    int order_id;
};

struct product {
    char* prod_name;
    struct orders* buy;
    struct orders* sell;
};

struct orders {
    struct dyn_array* price_ls;
    struct dyn_array* orders_ls;
};

struct order {
    int order_id;
    long qty;
    struct trader* trader;
};

#endif
