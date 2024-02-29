#ifndef HELPERS_H
#define HELPERS_H

#include "pe_common.h"
#include "pe_exchange.h"

void initialise_product(struct product* product, char* product_name);
void initialise_order(struct order* order, int order_id, long qty, struct trader* trader);
void process_commands(struct exchange* exchange, struct trader* trader);
void process_buy(struct exchange* exchange, char* first_word, char* command, struct trader* trader);
void process_sell(struct exchange* exchange, char* first_word, char* command, struct trader* trader);
void process_amend(struct exchange* exchange, char* command, struct trader* trader);
void process_cancel(struct exchange* exchange, char* command, struct trader* trader);
void store_products(struct exchange* exchange, char* filename);
void start_up(struct exchange* exchange, int argc, char **argv);
void reporting(struct exchange* exchange);
void matching(struct exchange* exchange, struct trader* trader, char* command, int order_id, char* product_name, long* qty, long price);
void teardown(struct exchange* exchange);
void accepted(struct dyn_array* traders_ls, struct trader* trader, int order_id, char* command, char* product_name, long qty, long price);
void fill(struct trader* trader, int order_id, long qty);
void amended(struct dyn_array* traders_ls, struct trader* trader, int order_id, char* command, char* product_name, long qty, long price);
void cancelled(struct dyn_array* traders_ls, struct trader* trader, int order_id, char* command, char* product_name, long qty, long price);
void market_msg(struct dyn_array* traders_ls, struct trader* trader, int order_id, char* command, char* product_name, long qty, long price);
void invalid(struct trader* trader);
char* read_command(struct trader* trader, char* command);
int count_spaces(char* command);

#endif