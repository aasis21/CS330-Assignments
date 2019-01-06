#include "kvstore.h"
#include <pthread.h>
#include <time.h>
#include <signal.h>

// Constants
#define THREADS 16
#define RUNS 10
#define RD_RUNS 50
#define INPUT "16MB.txt"
#define SIZE (500 * 1024l)

static pthread_mutex_t fail_lock;
static pthread_mutex_t read_lock;
static volatile int stop = 0;
static int fail = 0;
static char error[32];
static int log_data[THREADS][2];
static char *buf = NULL;
static long length;

void int_handler(int dummy)
{
    stop = 1;
}

void *run(void *args)
{
    int num = *((int*) args);  // Thread num

    // Choose random slice of SIZE length from INPUT
    char *value = (char*) malloc(SIZE);
    int shift = rand() / (RAND_MAX / (length - SIZE) + 1);
    memcpy(value, buf + shift, SIZE);
    value[SIZE - 1] = '\0';

    char *obtained = (char*) malloc(SIZE);
    char key[32], keyp[32];
    sprintf(key, "testfile-%d", num);
    sprintf(keyp, "newtestfile-%d", num);

    // Testing for RUNS no. of runs
    for (int i = 1; i <= RUNS; i++)
    {
        if (fail || stop)
            break;

        log_data[num][0] = i;
        // Clean earlier runs, testing lookup and delete
        if (lookup_key(key) > 0)
            delete_key(key);
        if (lookup_key(keyp) > 0)
            delete_key(keyp);

        // Testing write
        if (put_key(key, value, SIZE) < 0)
        {
            pthread_mutex_lock(&fail_lock);
            if (!fail)
            {
                fail = num;
                strcpy(error, "write");
            }
            pthread_mutex_unlock(&fail_lock);
            break;
        }

        // Testing rename
        if (rename_key(key, keyp) < 0)
        {
            pthread_mutex_lock(&fail_lock);
            if (!fail)
            {
                fail = num;
                strcpy(error, "rename");
            }
            pthread_mutex_unlock(&fail_lock);
            break;
        }

        // Testing read for RD_RUNS no. of runs
        for (int j = 1; j <= RD_RUNS; j++)
        {
            if (fail || stop)
                break;

            log_data[num][1] = j;
            pthread_mutex_lock(&read_lock);
            int status = get_key(keyp, obtained);
            pthread_mutex_unlock(&read_lock);
            if (status < 0)
            {
                pthread_mutex_lock(&fail_lock);
                if (!fail)
                {
                    fail = num;
                    strcpy(error, "read");
                }
                pthread_mutex_unlock(&fail_lock);
                break;
            }
            if (strcmp(value, obtained))
            {
                pthread_mutex_lock(&fail_lock);
                if (!fail)
                {
                    fail = num;
                    strcpy(error, "mismatch");
                }
                pthread_mutex_unlock(&fail_lock);
                break;
            }
        }
    }

    free(obtained);
    free(value);
    return NULL;
}

void *logging(void *args)
{
    // Setup for 100ms sleep
    struct timespec ts;
    ts.tv_sec = 0;
    ts.tv_nsec = 100 * 1000000;

    // Run till not failed and not complete
    int complete = 0;
    while ((!fail) && (!complete) && (!stop))
    {
        complete = 1;
        for (int i = 0; i < THREADS; i++)
        {
            if ((log_data[i][0] < RUNS) || (log_data[i][1] < RD_RUNS))
                complete = 0;
            printf("Thread %2d:: Runs: %3d, Read Runs: %3d\n", i + 1, log_data[i][0], log_data[i][1]);
        }
        printf("\e[%dA", THREADS);  // Move cursor up THREADS no. of lines
        nanosleep(&ts, NULL);  // Sleep for 100ms
    }
    printf("\e[%dB", THREADS);  // Move cursor down THREADS no. of lines
    return NULL;
}

int main()
{
    // Register Ctrl-C handler
    signal(SIGINT, int_handler);

    // Open INPUT for testing on input text
    FILE *f = fopen(INPUT, "rb");
    if (f)
    {
        fseek(f, 0, SEEK_END);
        length = ftell(f);
        fseek(f, 0, SEEK_SET);
        buf = malloc(length);
        if (buf)
            fread(buf, 1, length, f);
        fclose(f);
    }
    else
    {
        printf("Could not open file for reading\n");
        return -1;
    }

    if (!buf)
    {
        printf("Input file: %s missing\n", INPUT);
        free(buf);
        return -1;
    }

    if (pthread_mutex_init(&fail_lock, NULL) || pthread_mutex_init(&read_lock, NULL))
    {
        printf("Mutex init failed\n");
        return 1;
    }

    pthread_t thread[THREADS];  // Threads for testing
    pthread_t logger;  // Thread for logging
    int num[THREADS];
    for (int i = 0; i < THREADS; i++)
    {
        num[i] = i;
        if (pthread_create(thread + i, NULL, run, (void*)(num + i)))
        {
            printf("Could not create thread %d\n", i);
            return 1;
        }
    }
    if (pthread_create(&logger, NULL, logging, NULL))
    {
        printf("Could not create logger thread\n");
        return 1;
    }

    for (int i = 0; i < THREADS; i++)
        pthread_join(thread[i], NULL);
    pthread_join(logger, NULL);

    pthread_mutex_destroy(&fail_lock);
    pthread_mutex_destroy(&read_lock);
    free(buf);

    if (stop)
    {
        printf("\rReceived SIGINT\n");
        return 1;
    }
    else if (fail)
    {
        if (!(strcmp(error, "mismatch")))
            printf("ERROR: Mismatch in thread %d during read\n", fail + 1);
        else
            printf("ERROR: Error in thread %d during %s\n", fail + 1, error);
        return 1;
    }
    else
    {
        printf("SUCCESS: All okay\n");
        return 0;
    }
}
