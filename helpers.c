#include "helpers.h"

void initialise_product(struct product* product, char* product_name) {
    asprintf(&product->prod_name, "%s", product_name);
    product->buy = malloc(sizeof(struct orders));
    product->sell = malloc(sizeof(struct orders));
    product->buy->price_ls = dyn_array_init();
    product->buy->orders_ls = dyn_array_init();
    product->sell->price_ls = dyn_array_init();
    product->sell->orders_ls = dyn_array_init();
}

void initialise_order(struct order* order, int order_id, long qty, struct trader* trader) {
    order->order_id = order_id;
    order->qty = qty;
    order->trader = trader;
}

void process_commands(struct exchange* exchange, struct trader* trader) {
    char command[COMMAND_SIZE] = { 0 };

    if (strcmp(read_command(trader, command), "") == 0)
        invalid(trader);
    else
        printf("%s [T%d] Parsing command: <%s>\n", LOG_PREFIX, trader->id, command);

    if ((strncmp(command, "BUY ", strlen("BUY ")) == 0) && (count_spaces(command) == 4)) {
        process_buy(exchange, "BUY", command, trader);
    } else if ((strncmp(command, "SELL ", strlen("SELL ")) == 0) && (count_spaces(command) == 4)) {
        process_sell(exchange, "SELL", command, trader);
    } else if ((strncmp(command, "AMEND ", strlen("AMEND ")) == 0) && (count_spaces(command) == 3)) {
        process_amend(exchange, command, trader);
    } else if ((strncmp(command, "CANCEL ", strlen("CANCEL ")) == 0) && (count_spaces(command) == 1)) {
        process_cancel(exchange, command, trader);
    } else {
        invalid(trader);
    }
}

void process_buy(struct exchange* exchange, char* first_word, char* command, struct trader* trader) {
    int order_id;
    char product_name[PRODUCT_SIZE] = { 0 };
    long qty;
    long price;
    
    if (sscanf(command, "%*s %d %s %ld %ld;", &order_id, product_name, &qty, &price) != 4) {
        invalid(trader);
    } else {
        // Find the product and store in the respective BUY struct
        int product_found = 0;
        int is_invalid = 1;
        for (int i = 0; i < exchange->product_ls->size; i++) {
            struct product* product = exchange->product_ls->array[i];
            // Check if product is found in the trade (in product_ls)
            if (strcmp(product->prod_name, product_name) == 0) {
                product_found = 1;
                // Error handling for quantity, price, and order id
                if (qty > 0 && qty <= 999999 && price > 0 && price <= 999999 && order_id == trader->order_id && order_id <= 999999) {
                    is_invalid = 0;
                    trader->order_id++;
                    accepted(exchange->traders_ls, trader, order_id, "BUY", product_name, qty, price);
                    matching(exchange, trader, first_word, order_id, product_name, &qty, price);
                    // If after matching quantity is bigger than 0, store the product
                    if (qty > 0) {
                        int price_found = 0;
                        struct order* new_order = malloc(sizeof(struct order));
                        initialise_order(new_order, order_id, qty, trader);
                        // Check if the price is in the product's price_ls
                        for (int j = 0; j < product->buy->price_ls->size; j++) {
                            long* product_price = (long*) dyn_array_get(product->buy->price_ls, j);
                            // Add to orders_ls[j] if price is found in price_ls (like a hashmap; key: price, value: dyn_array of struct order*)
                            if (*product_price == price) {
                                price_found = 1;
                                dyn_array_add(product->buy->orders_ls->array[j], new_order);
                            }
                        }
                        // If price is not found, add new "key pair value"
                        if (! price_found) {
                            long* price_calloc = (long*) calloc(1, sizeof(long*));
                            *price_calloc = price;
                            // Add a new dyn_array
                            struct dyn_array* price_order = dyn_array_init();
                            dyn_array_add_order(product->buy->price_ls, product->buy->orders_ls, price_calloc, price_order, new_order);
                        }
                    }
                }
            }
        }
        // If product is not found in the trade, it is invalid
        if (product_found && ! is_invalid)
            reporting(exchange);
        else
            invalid(trader);
    }
}

void process_sell(struct exchange* exchange, char* first_word, char* command, struct trader* trader) {
    int order_id;
    char product_name[PRODUCT_SIZE] = { 0 };
    long qty;
    long price;
    
    if (sscanf(command, "%*s %d %s %ld %ld;", &order_id, product_name, &qty, &price) != 4) {
        invalid(trader);
    } else {
        // Find the product and store in the respective SELL struct
        int product_found = 0;
        int is_invalid = 1;
        for (int i = 0; i < exchange->product_ls->size; i++) {
            struct product* product = exchange->product_ls->array[i];
            // Check if product is found in the trade (in product_ls)
            if (strcmp(product->prod_name, product_name) == 0) {
                product_found = 1;
                // Error handling for quantity, price, and order id
                if (qty > 0 && qty <= 999999 && price > 0 && price <= 999999 && order_id == trader->order_id && order_id <= 999999) {
                    is_invalid = 0;
                    trader->order_id++;
                    accepted(exchange->traders_ls, trader, order_id, "SELL", product_name, qty, price);
                    matching(exchange, trader, first_word, order_id, product_name, &qty, price);
                    if (qty > 0) {
                        int price_found = 0;
                        struct order* new_order = malloc(sizeof(struct order));
                        initialise_order(new_order, order_id, qty, trader);
                        // Check if the price is in the product's price_ls
                        for (int j = 0; j < product->sell->price_ls->size; j++) {
                            long* product_price = (long*) dyn_array_get(product->sell->price_ls, j);
                            // Add to orders_ls[j] if price is found in price_ls (like a hashmap; key: price, value: dyn_array of struct order*)
                            if (*product_price == price) {
                                price_found = 1;
                                dyn_array_add(product->sell->orders_ls->array[j], new_order);
                            }
                        }
                        // If price is not found, add new "key pair value"
                        if (! price_found) {
                            long* price_calloc = (long*) calloc(1, sizeof(long*));
                            *price_calloc = price;
                            // Add a new dyn_array
                            struct dyn_array* price_order = dyn_array_init();
                            dyn_array_add_order(product->sell->price_ls, product->sell->orders_ls, price_calloc, price_order, new_order);
                        }
                    }
                }
            }
        }
        // If product is not found in the trade, it is invalid
        if (product_found && ! is_invalid)
            reporting(exchange);
        else
            invalid(trader);
    } 
}

void process_amend(struct exchange* exchange, char* command, struct trader* trader) {
    int order_id;
    long qty;
    long price;
    
    if (sscanf(command, "%*s %d %ld %ld;", &order_id, &qty, &price) != 3) {
        invalid(trader);
    } else {
        int is_invalid = 1;
        // Error handling for quantity and price
        if (qty > 0 && qty <= 999999 && price > 0 && price <= 999999) {
            // Loop through each product's buy and sell to find the order id
            for (int i = 0; i < exchange->product_ls->size; i++) {
                struct product* product = exchange->product_ls->array[i];
                struct orders* product_buy = product->buy;
                struct orders* product_sell = product->sell;
                for (int j = 0; j < product_buy->orders_ls->size; j++) {
                    struct dyn_array* buy_orders = product_buy->orders_ls->array[j];
                    for (int k = 0; k < buy_orders->size; k++) {
                        struct order* buy_order = (struct order*) dyn_array_get(buy_orders, k);
                        if (buy_order->order_id == order_id && trader->id == buy_order->trader->id && order_id <= 999999) {
                            is_invalid = 0;
                            free(buy_order);
                            dyn_array_delete(buy_orders, k); // Delete order from the order_ls
                            amended(exchange->traders_ls, trader, order_id, "BUY", product->prod_name, qty, price);
                            matching(exchange, trader, "BUY", order_id, product->prod_name, &qty, price);
                            // Find the new price in price_ls
                            if (qty > 0) {
                                int price_found = 0;
                                struct order* new_order = malloc(sizeof(struct order));
                                initialise_order(new_order, order_id, qty, trader);
                                // Check if the price is in the product's price_ls
                                for (int l = 0; l < product_buy->price_ls->size; l++) {
                                    long* product_price = (long*) dyn_array_get(product_buy->price_ls, l);
                                    // Add to orders_ls[j] if price is found in price_ls (like a hashmap; key: price, value: dyn_array of struct order*)
                                    if (*product_price == price) {
                                        price_found = 1;
                                        dyn_array_add(product_buy->orders_ls->array[j], new_order);
                                    }
                                }
                                // If price is not found, add new "key pair value"
                                if (! price_found) {
                                    long* price_calloc = (long*) calloc(1, sizeof(long*));
                                    *price_calloc = price;
                                    // Add a new dyn_array
                                    struct dyn_array* price_order = dyn_array_init();
                                    dyn_array_add_order(product_buy->price_ls, product_buy->orders_ls, price_calloc, price_order, new_order);
                                }
                            }      
                        }
                    }                    
                }
                for (int j = 0; j < product_sell->orders_ls->size; j++) {
                    struct dyn_array* sell_orders = product_sell->orders_ls->array[j];
                    for (int k = 0; k < sell_orders->size; k++) {
                        struct order* sell_order = (struct order*) dyn_array_get(sell_orders, k);
                        if (sell_order->order_id == order_id && trader->id == sell_order->trader->id && order_id <= 999999) {
                            is_invalid = 0;
                            free(sell_order);
                            dyn_array_delete(sell_orders, k); // Delete order from the order_ls
                            amended(exchange->traders_ls, trader, order_id, "SELL", product->prod_name, qty, price);
                            matching(exchange, trader, "SELL", order_id, product->prod_name, &qty, price);
                            // Find the new price in price_ls
                            if (qty > 0) {
                                int price_found = 0;
                                struct order* new_order = malloc(sizeof(struct order));
                                initialise_order(new_order, order_id, qty, trader);
                                // Check if the price is in the product's price_ls
                                for (int l = 0; l < product_sell->price_ls->size; l++) {
                                    long* product_price = (long*) dyn_array_get(product_sell->price_ls, l);
                                    // Add to orders_ls[j] if price is found in price_ls (like a hashmap; key: price, value: dyn_array of struct order*)
                                    if (*product_price == price) {
                                        price_found = 1;
                                        dyn_array_add(product_sell->orders_ls->array[j], new_order);
                                    }
                                }
                                // If price is not found, add new "key pair value"
                                if (! price_found) {
                                    long* price_calloc = (long*) calloc(1, sizeof(long*));
                                    *price_calloc = price;
                                    // Add a new dyn_array
                                    struct dyn_array* price_order = dyn_array_init();
                                    dyn_array_add_order(product_sell->price_ls, product_sell->orders_ls, price_calloc, price_order, new_order);
                                }
                            }      
                        }
                    }                    
                }
                // Delete the dyn_array if empty, and delete the respective hashed price in price_ls
                int j = 0;
                while (j < product_buy->orders_ls->size) {
                    struct dyn_array* buy_orders = product_buy->orders_ls->array[j];
                    if (buy_orders->size == 0) {
                        dyn_array_free_array_only(buy_orders);
                        dyn_array_delete(product_buy->orders_ls, j);
                        free(product_buy->price_ls->array[j]);
                        dyn_array_delete(product_buy->price_ls, j);
                        j--; // Decrement the counter because the new index will now have a different element
                    }
                    j++;
                }
                // Delete the dyn_array if empty, and delete the respective hashed price in price_ls
                int k = 0;
                while (k < product_sell->orders_ls->size) {
                    struct dyn_array* matched_orders = product_sell->orders_ls->array[k];
                    if (matched_orders->size == 0) {
                        dyn_array_free_array_only(matched_orders);
                        dyn_array_delete(product_sell->orders_ls, k);
                        free(product_sell->price_ls->array[k]);
                        dyn_array_delete(product_sell->price_ls, k);
                        k--; // Decrement the counter because the new index will now have a different element
                    }
                    k++;
                }
            }
        }
        if (is_invalid)
            invalid(trader);
        else
            reporting(exchange);
    }
}

void process_cancel(struct exchange* exchange, char* command, struct trader* trader) {
    int order_id;
    
    if (sscanf(command, "%*s %d;", &order_id) != 1) {
        invalid(trader);
    } else {
        int is_invalid = 1;
        // Loop through each product's buy and sell to find the order id
        for (int i = 0; i < exchange->product_ls->size; i++) {
            struct product* product = exchange->product_ls->array[i];
            struct orders* product_buy = product->buy;
            struct orders* product_sell = product->sell;
            for (int j = 0; j < product_buy->orders_ls->size; j++) {
                struct dyn_array* buy_orders = product_buy->orders_ls->array[j];
                for (int k = 0; k < buy_orders->size; k++) {
                    struct order* buy_order = (struct order*) dyn_array_get(buy_orders, k);
                    if (buy_order->order_id == order_id && trader->id == buy_order->trader->id && order_id <= 999999) {
                        is_invalid = 0;
                        free(buy_order);
                        dyn_array_delete(buy_orders, k); // Delete order from the order_ls  
                        cancelled(exchange->traders_ls, trader, order_id, "BUY", product->prod_name, 0, 0);
                    }
                }                    
            }
            for (int j = 0; j < product_sell->orders_ls->size; j++) {
                struct dyn_array* sell_orders = product_sell->orders_ls->array[j];
                for (int k = 0; k < sell_orders->size; k++) {
                    struct order* sell_order = (struct order*) dyn_array_get(sell_orders, k);
                    if (sell_order->order_id == order_id && trader->id == sell_order->trader->id && order_id <= 999999) {
                        is_invalid = 0;
                        free(sell_order);
                        dyn_array_delete(sell_orders, k); // Delete order from the order_ls
                        cancelled(exchange->traders_ls, trader, order_id, "SELL", product->prod_name, 0, 0);     
                    }
                }                    
            }
            // Delete the dyn_array if empty, and delete the respective hashed price in price_ls
            int j = 0;
            while (j < product_buy->orders_ls->size) {
                struct dyn_array* buy_orders = product_buy->orders_ls->array[j];
                if (buy_orders->size == 0) {
                    dyn_array_free_array_only(buy_orders);
                    dyn_array_delete(product_buy->orders_ls, j);
                    free(product_buy->price_ls->array[j]);
                    dyn_array_delete(product_buy->price_ls, j);
                    j--; // Decrement the counter because the new index will now have a different element
                }
                j++;
            }
            // Delete the dyn_array if empty, and delete the respective hashed price in price_ls
            int k = 0;
            while (k < product_sell->orders_ls->size) {
                struct dyn_array* matched_orders = product_sell->orders_ls->array[k];
                if (matched_orders->size == 0) {
                    dyn_array_free_array_only(matched_orders);
                    dyn_array_delete(product_sell->orders_ls, k);
                    free(product_sell->price_ls->array[k]);
                    dyn_array_delete(product_sell->price_ls, k);
                    k--; // Decrement the counter because the new index will now have a different element
                }
                k++;
            }
        } 
        if (is_invalid)
            invalid(trader);
        else
            reporting(exchange);
    }
}

// Store products provided from the text file to an array, print output to stdout
void store_products(struct exchange* exchange, char* filename) {
	printf("%s Starting\n", LOG_PREFIX);
	FILE* fptr;
	if ((fptr = fopen(filename, "r")) == NULL) {
		printf("Error opening product file\n");
		exit(1);
	}
	fscanf(fptr, "%d\n", &exchange->n_items);
	printf("%s Trading %d products:", LOG_PREFIX, exchange->n_items);
	
    char product_name[PRODUCT_SIZE] = { 0 };
	while (fgets(product_name, PRODUCT_SIZE, fptr)) {
        struct product* product = malloc(sizeof(struct product));
		product_name[strlen(product_name) - 1] = '\0';
        initialise_product(product, product_name);
        dyn_array_add(exchange->product_ls, product); // Add a product struct to product_ls
        printf(" %s", product_name);
	}
	printf("\n");
    fclose(fptr);
}

// Exchange start up: store products to be traded & launch the trader binaries
void start_up(struct exchange* exchange, int argc, char **argv) {
    store_products(exchange, argv[1]);
    // Create 2 pipes for each trader
    for (int i = 2; i < argc; i++) {
        struct trader* new_trader = malloc(sizeof(struct trader));
        new_trader->id = i - 2;
        asprintf(&new_trader->binary, "%s", argv[i]);
        asprintf(&new_trader->exchange_pipe, FIFO_EXCHANGE, new_trader->id);
        asprintf(&new_trader->trader_pipe, FIFO_TRADER, new_trader->id);
        // Create FIFOs
        if (mkfifo(new_trader->exchange_pipe, 0666) == -1)
            perror("mkfifo exchange");
        printf("%s Created FIFO %s\n", LOG_PREFIX, new_trader->exchange_pipe);
        if (mkfifo(new_trader->trader_pipe, 0666) == -1)
            perror("mkfifo trader");
        printf("%s Created FIFO %s\n", LOG_PREFIX, new_trader->trader_pipe);
        // Launch the trader program
        new_trader->pid = fork();
        if (new_trader->pid == 0) {
            char* id;
            asprintf(&id, "%d", new_trader->id);
            printf("%s Starting trader %d (%s)\n", LOG_PREFIX, new_trader->id, new_trader->binary);
            execl(new_trader->binary, strrchr(new_trader->binary, '/'), id, (char*) NULL);
            // If exec fails, free all dynamic memory and exit
            perror("execl");
            free(id);
            dyn_array_free_traders(exchange->traders_ls);
            dyn_array_free(exchange->traders_ls);
            exit(1);
        }
        // Open pipes
        new_trader->exchange_fd = open(new_trader->exchange_pipe, O_WRONLY);
        new_trader->trader_fd = open(new_trader->trader_pipe, O_RDONLY);
        if (new_trader->exchange_fd == -1)
            perror("exchangefd");
        printf("%s Connected to %s\n", LOG_PREFIX, new_trader->exchange_pipe);
        if (new_trader->trader_fd == -1)
            perror("traderfd");
        printf("%s Connected to %s\n", LOG_PREFIX, new_trader->trader_pipe);
        // Store net quantity in qty_ls - same size as product_ls
        new_trader->qty_ls = dyn_array_init();
        for (int i = 0; i < exchange->product_ls->size; i++) {
            long* qty_calloc = (long*) calloc(1, sizeof(int*));
            dyn_array_add(new_trader->qty_ls, qty_calloc);
        }
        // Store values in value_ls - same size as qty_ls
        new_trader->value_ls = dyn_array_init();
        for (int i = 0; i < new_trader->qty_ls->size; i++) {
            long* price_calloc = (long*) calloc(1, sizeof(long*));
            dyn_array_add(new_trader->value_ls, price_calloc);
        }
        new_trader->disconnected = 0; // Set the disconnected boolean to FALSE
        new_trader->order_id = 0; // Set the order id to 0
        // Store the trader in a list
        dyn_array_add(exchange->traders_ls, new_trader);
    }
    // Send MARKET OPEN message
    for (int i = 0; i < exchange->traders_ls->size; i++) {
        char* market_open = "MARKET OPEN;";
        struct trader* trader = (struct trader*) exchange->traders_ls->array[i];
        write(trader->exchange_fd, market_open, 12);
        kill(trader->pid, SIGUSR1);
    }
}

void reporting(struct exchange* exchange) {
    printf("%s\t--ORDERBOOK--\n", LOG_PREFIX);
    for (int i = 0; i < exchange->product_ls->size; i++) {
        // Check if each product has any BUY or SELL
        struct product* product = exchange->product_ls->array[i];
        int buy_level = product->buy->price_ls->size;
        int sell_level = product->sell->price_ls->size;
        printf("%s\tProduct: %s; Buy levels: %d; Sell levels: %d\n", LOG_PREFIX, product->prod_name, buy_level, sell_level);
        // Print out all the sells
        for (int j = sell_level - 1; j >= 0; j--) {
            struct dyn_array* orders_by_price = product->sell->orders_ls->array[j];
            long price = *((long*) product->sell->price_ls->array[j]);
            long total_qty = 0;
            int num_orders = 0;
            // Calculate total quantity and number of orders for each price
            for (int k = 0; k < orders_by_price->size; k++) {
                total_qty += ((struct order*) orders_by_price->array[k])->qty;
                num_orders++;
            }
            if (num_orders == 1) {
                printf("%s\t\tSELL %ld @ $%ld (%d order)\n", LOG_PREFIX, total_qty, price, num_orders);
            } else {
                printf("%s\t\tSELL %ld @ $%ld (%d orders)\n", LOG_PREFIX, total_qty, price, num_orders);
            }
        }
        // Print out all the buys
        for (int j = buy_level - 1; j >= 0; j--) {
            struct dyn_array* orders_by_price = product->buy->orders_ls->array[j];
            long price = *((long*) product->buy->price_ls->array[j]);
            long total_qty = 0;
            int num_orders = 0;
            // Calculate total quantity and number of orders for each price
            for (int k = 0; k < orders_by_price->size; k++) {
                total_qty += ((struct order*) orders_by_price->array[k])->qty;
                num_orders++;
            }
            if (num_orders == 1) {
                printf("%s\t\tBUY %ld @ $%ld (%d order)\n", LOG_PREFIX, total_qty, price, num_orders);
            } else {
                printf("%s\t\tBUY %ld @ $%ld (%d orders)\n", LOG_PREFIX, total_qty, price, num_orders);
            }
        }
    }
    printf("%s\t--POSITIONS--\n", LOG_PREFIX);
    for (int i = 0; i < exchange->traders_ls->size; i++) {
        struct trader* trader = exchange->traders_ls->array[i];
        printf("%s\tTrader %d:", LOG_PREFIX, trader->id);
        // Print the key: products from product_ls and value: value from value_ls
        for (int j = 0; j < exchange->product_ls->size; j++) {
            struct product* product = exchange->product_ls->array[j];
            long net_qty = *((long*) trader->qty_ls->array[j]);
            long value = *((long*) trader->value_ls->array[j]);
            if (j == exchange->product_ls->size - 1) {
                printf(" %s %ld ($%ld)\n", product->prod_name, net_qty, value);
            } else {
                printf(" %s %ld ($%ld),", product->prod_name, net_qty, value);
            }
        }
    }
}

void matching(struct exchange* exchange, struct trader* trader, char* command, int order_id, char* product_name, long* qty, long price) {
    for (int i = 0; i < exchange->product_ls->size; i++) {
        struct product* product = exchange->product_ls->array[i];
        if (strcmp(product->prod_name, product_name) == 0) {
            struct orders* product_buy = product->buy;
            struct orders* product_sell = product->sell;

            if (strcmp(command, "SELL") == 0) {
                // Get the highest buy
                for (int j = product_buy->price_ls->size - 1; j >= 0; j--) {
                    long* highest_price = (long*) product_buy->price_ls->array[j];
                    // Find the quantities for each price
                    struct dyn_array* matched_orders = product_buy->orders_ls->array[j];
                    for (int k = 0; k < matched_orders->size; k++) {
                        struct order* matched_order = matched_orders->array[k];
                        struct trader* matched_trader = matched_order->trader;
                        long order_qty = matched_order->qty;
                        if (price <= *highest_price && *qty >= order_qty) {
                            long value = *highest_price * order_qty;
                            long fee = round(0.01 * value);
                            *qty -= order_qty;
                            matched_order->qty = 0;
                            // Store the quantity and value for the respective trader
                            *((long*) matched_trader->qty_ls->array[i]) += order_qty;
                            *((long*) trader->qty_ls->array[i]) -= order_qty;
                            *((long*) matched_trader->value_ls->array[i]) -= value;
                            *((long*) trader->value_ls->array[i]) += value;
                            *((long*) trader->value_ls->array[i]) -= fee;
                            exchange->exchange_fees += fee;
                            printf("%s Match: Order %d [T%d], New Order %d [T%d], value: $%ld, fee: $%ld.\n", LOG_PREFIX, matched_order->order_id, matched_trader->id, order_id, trader->id, value, fee);
                            fill(matched_trader, matched_order->order_id, order_qty);
                            fill(trader, order_id, order_qty);
                        } else if (price <= *highest_price && *qty < order_qty) {
                            long value = *highest_price * *qty;
                            long fee = round(0.01 * value);
                            matched_order->qty -= *qty;
                            // Store the quantity and value for the respective trader
                            *((long*) matched_trader->qty_ls->array[i]) += *qty;
                            *((long*) trader->qty_ls->array[i]) -= *qty;
                            *((long*) matched_trader->value_ls->array[i]) -= value;
                            *((long*) trader->value_ls->array[i]) += value;
                            *((long*) trader->value_ls->array[i]) -= fee;
                            exchange->exchange_fees += fee;
                            printf("%s Match: Order %d [T%d], New Order %d [T%d], value: $%ld, fee: $%ld.\n", LOG_PREFIX, matched_order->order_id, matched_trader->id, order_id, trader->id, value, fee);
                            fill(matched_trader, matched_order->order_id, *qty);
                            fill(trader, order_id, *qty);
                            *qty = 0;
                        }
                    }
                    // If order quantity is 0, delete it from matched_orders
                    int k = 0;
                    while (k < matched_orders->size) {
                        struct order* matched_order = matched_orders->array[k];
                        long order_qty = matched_order->qty;
                        if (order_qty == 0) {
                            free(matched_order);
                            dyn_array_delete(matched_orders, k);
                            k--; // Decrement the counter because the new index will now have a different element
                        }
                        k++;
                    }
                }
                // Delete the dyn_array if empty, and delete the respective hashed price in price_ls
                int j = 0;
                while (j < product_buy->orders_ls->size) {
                    struct dyn_array* matched_orders = product_buy->orders_ls->array[j];
                    if (matched_orders->size == 0) {
                        dyn_array_free_array_only(matched_orders);
                        dyn_array_delete(product_buy->orders_ls, j);
                        free(product_buy->price_ls->array[j]);
                        dyn_array_delete(product_buy->price_ls, j);
                        j--; // Decrement the counter because the new index will now have a different element
                    }
                    j++;
                }
            } else if (strcmp(command, "BUY") == 0) {
                // Get the lowest sell
                for (int j = 0; j < product_sell->price_ls->size; j++) {
                    long* lowest_price = (long*) product_sell->price_ls->array[j];
                    // Find the quantities for each price
                    struct dyn_array* matched_orders = product_sell->orders_ls->array[j];
                    for (int k = 0; k < matched_orders->size; k++) {
                        struct order* matched_order = matched_orders->array[k];
                        struct trader* matched_trader = matched_order->trader;
                        long order_qty = matched_order->qty;
                        if (price >= *lowest_price && *qty >= order_qty) {
                            long value = *lowest_price * order_qty;
                            long fee = round(0.01 * value);
                            *qty -= order_qty;
                            matched_order->qty = 0;
                            // Store the quantity and value for the respective trader
                            *((long*) trader->qty_ls->array[i]) += order_qty;
                            *((long*) matched_trader->qty_ls->array[i]) -= order_qty;
                            *((long*) trader->value_ls->array[i]) -= value;
                            *((long*) matched_trader->value_ls->array[i]) += value;
                            *((long*) trader->value_ls->array[i]) -= fee;
                            exchange->exchange_fees += fee;
                            printf("%s Match: Order %d [T%d], New Order %d [T%d], value: $%ld, fee: $%ld.\n", LOG_PREFIX, matched_order->order_id, matched_trader->id, order_id, trader->id, value, fee);
                            fill(matched_trader, matched_order->order_id, order_qty);
                            fill(trader, order_id, order_qty);
                        } else if (price >= *lowest_price && *qty < order_qty) {
                            long value = *lowest_price * *qty;
                            long fee = round(0.01 * value);
                            matched_order->qty -= *qty;
                            // Store the quantity and value for the respective trader
                            *((long*) trader->qty_ls->array[i]) += *qty;
                            *((long*) matched_trader->qty_ls->array[i]) -= *qty;
                            *((long*) trader->value_ls->array[i]) -= value;
                            *((long*) matched_trader->value_ls->array[i]) += value;
                            *((long*) trader->value_ls->array[i]) -= fee;
                            exchange->exchange_fees += fee;
                            printf("%s Match: Order %d [T%d], New Order %d [T%d], value: $%ld, fee: $%ld.\n", LOG_PREFIX, matched_order->order_id, matched_trader->id, order_id, trader->id, value, fee);
                            fill(matched_trader, matched_order->order_id, *qty);
                            fill(trader, order_id, *qty);
                            *qty = 0;
                        }
                    }
                    // If order quantity is 0, delete it from matched_orders
                    int k = 0;
                    while (k < matched_orders->size) {
                        struct order* matched_order = matched_orders->array[k];
                        long order_qty = matched_order->qty;
                        if (order_qty == 0) {
                            free(matched_order);
                            dyn_array_delete(matched_orders, k);
                            k--; // Decrement the counter because the new index will now have a different element
                        }
                        k++;
                    }
                }
                // Delete the dyn_array if empty, and delete the respective hashed price in price_ls
                int j = 0;
                while (j < product_sell->orders_ls->size) {
                    struct dyn_array* matched_orders = product_sell->orders_ls->array[j];
                    if (matched_orders->size == 0) {
                        dyn_array_free_array_only(matched_orders);
                        dyn_array_delete(product_sell->orders_ls, j);
                        free(product_sell->price_ls->array[j]);
                        dyn_array_delete(product_sell->price_ls, j);
                        j--; // Decrement the counter because the new index will now have a different element
                    }
                    j++;
                }
            }
        }
    }
}

// Clean up any remaining child processes, close and delete FIFOs, and free memory
void teardown(struct exchange* exchange) {
    // Close pipes and unlink
    for (int i = 0; i < exchange->traders_ls->size; i++) {
        struct trader* trader = (struct trader*) exchange->traders_ls->array[i];
        close(trader->exchange_fd);
        close(trader->trader_fd);
        unlink(trader->exchange_pipe);
        unlink(trader->trader_pipe);
    }
    // Freeing all malloc 
    dyn_array_free_products(exchange->product_ls);
    dyn_array_free(exchange->product_ls);
    dyn_array_free_traders(exchange->traders_ls);
    dyn_array_free(exchange->traders_ls);
    printf("%s Trading completed\n", LOG_PREFIX);
    printf("%s Exchange fees collected: $%ld\n", LOG_PREFIX, (long) exchange->exchange_fees);
}

void accepted(struct dyn_array* traders_ls, struct trader* trader, int order_id, char* command, char* product_name, long qty, long price) {
    char message[COMMAND_SIZE] = { 0 };
    snprintf(message, COMMAND_SIZE, "ACCEPTED %d;", order_id);
    if (!trader->disconnected) {
        if (write(trader->exchange_fd, message, strlen(message)) == -1)
            perror("write accepted");
        if (kill(trader->pid, SIGUSR1) == -1)
            perror("accepted kill");
    }
    // Send message to all other exchange pipes
    market_msg(traders_ls, trader, order_id, command, product_name, qty, price);
}  

void fill(struct trader* trader, int order_id, long qty) {
    char message[COMMAND_SIZE] = { 0 };
    snprintf(message, COMMAND_SIZE, "FILL %d %ld;", order_id, qty);
    if (!trader->disconnected) {
        if (write(trader->exchange_fd, message, strlen(message)) == -1)
            perror("write accepted");
        if (kill(trader->pid, SIGUSR1) == -1)
            perror("accepted kill");
    }
}

void amended(struct dyn_array* traders_ls, struct trader* trader, int order_id, char* command, char* product_name, long qty, long price) {
    char message[COMMAND_SIZE] = { 0 };
    snprintf(message, COMMAND_SIZE, "AMENDED %d;", order_id);
    if (!trader->disconnected) {
        if (write(trader->exchange_fd, message, strlen(message)) == -1)
            perror("write amended");
        if (kill(trader->pid, SIGUSR1) == -1)
            perror("amended kill");
    }
    // Send message to all other exchange pipes
    market_msg(traders_ls, trader, order_id, command, product_name, qty, price);
}

void cancelled(struct dyn_array* traders_ls, struct trader* trader, int order_id, char* command, char* product_name, long qty, long price) {
    char message[COMMAND_SIZE] = { 0 };
    snprintf(message, COMMAND_SIZE, "CANCELLED %d;", order_id);
    if (!trader->disconnected) {
        if (write(trader->exchange_fd, message, strlen(message)) == -1)
            perror("write cancelled");
        if (kill(trader->pid, SIGUSR1) == -1)
            perror("cancelled kill");
    }
    // Send message to all other exchange pipes
    market_msg(traders_ls, trader, order_id, command, product_name, qty, price);
}

void market_msg(struct dyn_array* traders_ls, struct trader* trader, int order_id, char* command, char* product_name, long qty, long price) {
    char message[COMMAND_SIZE] = { 0 };
    snprintf(message, COMMAND_SIZE, "MARKET %s %s %ld %ld;", command, product_name, qty, price);
    for (int i = 0; i < traders_ls->size; i++) {
        struct trader* cur_trader = (struct trader*) traders_ls->array[i];
        if ((cur_trader->pid != trader->pid) && !cur_trader->disconnected) {
            if (write(cur_trader->exchange_fd, message, strlen(message)) == -1)
                perror("write accepted");
            if (kill(cur_trader->pid, SIGUSR1) == -1)
                perror("accepted kill");
        }
    }
}

void invalid(struct trader* trader) {
    char* message = "INVALID;";
    if (write(trader->exchange_fd, message, strlen(message)) == -1)
        perror("write invalid");
    if (kill(trader->pid, SIGUSR1) == -1)
        perror("invalid kill");
}

// Read command pet character
char* read_command(struct trader* trader, char* command) {
    char chr;
    for (int i = 0; i < COMMAND_SIZE; i++) {
        read(trader->trader_fd, &chr, 1);
        if (chr != ';') {
            strncat(command, &chr, 1);
        } else {
            return command;
        }
    }
    return "";
}

int count_spaces(char* command) {
    int count = 0;
    for (int i = 0; i < COMMAND_SIZE; i++) {
        if (command[i] == ' ') {
            count++;
        }
    }
    return count;
}