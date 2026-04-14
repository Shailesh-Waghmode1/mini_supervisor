#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>

int main(int argc, char *argv[]) {
    int mode = 0;
    
    if (argc > 1) {
        mode = atoi(argv[1]);
    }
    
    printf("Crasher started (PID: %d, mode: %d)\n", getpid(), mode);
    fflush(stdout);
    
    srand(time(NULL) + getpid());
    
    switch (mode) {
        case 0:
            // Random crash mode - run for 3-8 seconds then crash
            {
                int runtime = 3 + (rand() % 6);
                printf("Will crash in %d seconds...\n", runtime);
                fflush(stdout);
                
                for (int i = runtime; i > 0; i--) {
                    printf("Time remaining: %d seconds\n", i);
                    fflush(stdout);
                    sleep(1);
                }
                
                printf("CRASHING NOW!\n");
                fflush(stdout);
                
                // Cause segmentation fault
                int *p = NULL;
                *p = 42;
            }
            break;
            
        case 1:
            // Exit with non-zero code after some time
            {
                printf("Will exit with error code in 4 seconds...\n");
                fflush(stdout);
                
                for (int i = 4; i > 0; i--) {
                    printf("Countdown: %d\n", i);
                    fflush(stdout);
                    sleep(1);
                }
                
                printf("Exiting with error code 1\n");
                fflush(stdout);
                exit(1);
            }
            break;
            
        case 2:
            // Send SIGTERM to self
            {
                printf("Will terminate self with SIGTERM in 3 seconds...\n");
                fflush(stdout);
                sleep(3);
                
                printf("Sending SIGTERM to self\n");
                fflush(stdout);
                kill(getpid(), SIGTERM);
                
                // This shouldn't be reached
                sleep(1);
                printf("Still alive somehow...\n");
            }
            break;
            
        case 3:
            // Infinite loop that prints status
            {
                int counter = 0;
                printf("Starting infinite loop (use this to test normal operation)\n");
                fflush(stdout);
                
                while (1) {
                    printf("Working... iteration %d\n", ++counter);
                    fflush(stdout);
                    sleep(2);
                    
                    // Rarely crash (1 in 20 chance every iteration)
                    if (rand() % 20 == 0) {
                        printf("Random crash!\n");
                        fflush(stdout);
                        int *p = NULL;
                        *p = 42;
                    }
                }
            }
            break;
            
        default:
            printf("Unknown mode %d. Available modes:\n", mode);
            printf("  0: Random crash (3-8 seconds)\n");
            printf("  1: Exit with error code\n");
            printf("  2: Self-terminate with SIGTERM\n");
            printf("  3: Infinite loop with rare random crashes\n");
            exit(1);
    }
    
    return 0;
}