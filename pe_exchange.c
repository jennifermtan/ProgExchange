/**
 * comp2017 - assignment 3
 * Jennifer Tan
 * jtan8485
 */

#include "pe_exchange.h"
#include "helpers.h"

volatile pid_t sigusr1_pid;
volatile pid_t sigchld_pid;
volatile pid_t sigpipe_pid;
volatile int sigusr1_no;
volatile int sigchld_no;
volatile int sigpipe_no;
void sigusr1_handler(int signo, siginfo_t* info, void*) {
    sigusr1_pid = info->si_pid;
    sigusr1_no = signo;
}

void sigchld_handler(int signo, siginfo_t* info, void*) {
    sigchld_pid = info->si_pid;
    sigchld_no = signo;
}

void sigpipe_handler(int signo, siginfo_t* info, void*) {
    sigpipe_pid = info->si_pid;
    sigpipe_no = signo;
}

int main(int argc, char **argv) {
    if (argc < 3) {
        perror("argc");
    }
    // Register signal handler
    struct sigaction sig = {
        .sa_sigaction = sigusr1_handler,
        .sa_flags = SA_SIGINFO
    };

    if (sigaction(SIGUSR1, &sig, NULL) == -1) {
        perror("SIGUSR1");
    }

    sig.sa_sigaction = sigchld_handler;
    if (sigaction(SIGCHLD, &sig, NULL) == -1) {
        perror("SIGCHLD");
    }

    sig.sa_sigaction = sigpipe_handler;
    if (sigaction(SIGPIPE, &sig, NULL) == -1) {
        perror("SIGPIPE");
    }

    struct exchange temp;
    struct exchange* exchange = &temp;
    initialise_exchange(exchange);
    start_up(exchange, argc, argv);

    while (1) {
        if (sigusr1_no == SIGUSR1) {
            sigusr1_no = 0;
            // Read pipe and execute stuff
            for (int i = 0; i < exchange->traders_ls->size; i++) {
                struct trader* trader = (struct trader*) exchange->traders_ls->array[i];
                if (sigusr1_pid == trader->pid) {
                    if (! trader->disconnected)
                        process_commands(exchange, trader);
                }
            }
        }
        if (sigchld_no == SIGCHLD) {
            sigchld_no = 0;
            // If process ends, set disconnected to TRUE
            for (int i = 0; i < exchange->traders_ls->size; i++) {
                struct trader* trader = (struct trader*) exchange->traders_ls->array[i];
                if (sigchld_pid == trader->pid) {
                    trader->disconnected = 1;
                    printf("%s Trader %d disconnected\n", LOG_PREFIX, trader->id);
                }
            }
        }
        if (sigpipe_no == SIGPIPE) {
            sigpipe_no = 0;
            // If pipe closes, set disconnected to TRUE
            for (int i = 0; i < exchange->traders_ls->size; i++) {
                struct trader* trader = (struct trader*) exchange->traders_ls->array[i];
                if (sigpipe_pid == trader->pid) {
                    trader->disconnected = 1;
                    printf("%s Trader %d disconnected\n", LOG_PREFIX, trader->id);
                }
            }
        }
        // Count the number of disconnected traders, and end the program if all traders are disconnected
        int num_disconnected = 0;
        for (int i = 0; i < exchange->traders_ls->size; i++) {
            struct trader* trader = (struct trader*) exchange->traders_ls->array[i];
            if (trader->disconnected == 1) {
                num_disconnected++;
            }
        }
        if (num_disconnected == exchange->traders_ls->size) {
            break;
        }
    }
	teardown(exchange);
	return 0;
}

void initialise_exchange(struct exchange* exchange) {
    exchange->exchange_fees = 0;
	exchange->n_items = 0;
    exchange->product_ls = dyn_array_init();
    exchange->traders_ls = dyn_array_init();
}