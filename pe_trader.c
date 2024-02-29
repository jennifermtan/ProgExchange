#include "pe_trader.h"

volatile int sigusr1_no;
void sigusr1_handler(int signo, siginfo_t* info, void*) {
    sigusr1_no = signo;
}

int main(int argc, char ** argv) {
	if (argc < 2) {
        printf("Not enough arguments\n");
        return 1;
    }

    // register signal handler
    struct sigaction sig = {
        .sa_sigaction = sigusr1_handler,
        .sa_flags = SA_SIGINFO
    };

    if (sigaction(SIGUSR1, &sig, NULL) == -1) {
        perror("Sigaction error");
    }

    // connect to named pipes
    char exchange_pipe[FIFO_SIZE];
    char trader_pipe[FIFO_SIZE];
    snprintf(exchange_pipe, FIFO_SIZE, FIFO_EXCHANGE, atoi(argv[1]));
    snprintf(trader_pipe, FIFO_SIZE, FIFO_TRADER, atoi(argv[1]));

    int exchange_fd = open(exchange_pipe, O_RDONLY);
    int trader_fd = open(trader_pipe, O_WRONLY);

    if (exchange_fd == -1)
        perror("Open error Exchange");
    if (trader_fd == -1)
        perror("Open error Trader");
    
    char exchange_msg[COMMAND_SIZE];
    char trader_msg[COMMAND_SIZE];
    char product[PRODUCT_SIZE];
    int qty;
    int price;
    int order_id = 0;
    
    // event loop:
    while(1) {
        // wait for exchange update (MARKET message)
        // Check for a SIGUSR1 signal
        pause(); // Stops execution until signal received
        if (sigusr1_no == SIGUSR1) {
            sigusr1_no = 0;
            read(exchange_fd, exchange_msg, COMMAND_SIZE);
            if (strncmp(exchange_msg, "MARKET SELL ", 12) == 0) {
                // send order:
                if (sscanf(exchange_msg, "%*s %*s %s %d %d;", product, &qty, &price) != 3) {
                    perror("sscanf");
                }
                // Disconnect and shut down if BUY order quantity is >= 1000
                if (qty >= 1000) {
                    break;
                }
                snprintf(trader_msg, COMMAND_SIZE, "BUY %d %s %d %d;", order_id, product, qty, price);
                write(trader_fd, trader_msg, strlen(trader_msg));
                while (! sigusr1_no) {
                    kill(getppid(), SIGUSR1);
                    sleep(2);
                }
                // wait for exchange confirmation (ACCEPTED message)
                read(exchange_fd, exchange_msg, COMMAND_SIZE);
                sigusr1_no = 0;
                order_id++;
            }
        }
    }
    close(exchange_fd);
    close(trader_fd);

    return 0;
}
