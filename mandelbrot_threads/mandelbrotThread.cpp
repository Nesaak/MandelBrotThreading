#include <stdio.h>
#include <pthread.h>
#include <algorithm>

#include "CycleTimer.h"

typedef struct {
    float x0, x1;
    float y0, y1;
    unsigned int width;
    unsigned int height;
    int maxIterations;
    int* output;
    int threadId;
    int numThreads;
} WorkerArgs;


extern void mandelbrotSerial(
    float x0, float y0, float x1, float y1,
    int width, int height,
    int startRow, int numRows,
    int maxIterations,
    int output[]);


/*

**OLD WORKER CODE, PRE-OPTIMIZATION**

void* workerThreadStart(void* threadArgs) {

    WorkerArgs* args = static_cast<WorkerArgs*>(threadArgs);

    double startTime = CycleTimer::currentSeconds();

    // Calculate start and end rows for this thread
    int startRow = args->threadId * (args->height / args->numThreads);
    int endRow = startRow + (args->height / args->numThreads);

    printf("Thread %d starting at %d, ending at %d\n", args->threadId, startRow, endRow);

    // Call mandelbrotSerial to compute section
    mandelbrotSerial(args->x0, args->y0, args->x1, args->y1, 
                     args->width, args->height, startRow, endRow - startRow,
                     args->maxIterations, args->output);

    double endTime = CycleTimer::currentSeconds();

    printf("Thread %d finished in [%.3f] ms\n", args->threadId, (endTime - startTime) * 1000);

    return NULL;
}
*/


// workerThreadStart -- Thread entrypoint.
void* workerThreadStart(void* threadArgs) {
    WorkerArgs* args = static_cast<WorkerArgs*>(threadArgs);

    double startTime = CycleTimer::currentSeconds();

    int adjustedHeight = args->height / 2;
    int numRows = adjustedHeight / args->numThreads;

    int startRow = args->threadId * (adjustedHeight / args->numThreads);

    printf("Thread %d starting at %d, ending at %d\n", args->threadId, startRow, startRow + numRows);
    mandelbrotSerial(args->x0, args->y0, args->x1, args->y1, 
                args->width, args->height, startRow, numRows,
                args->maxIterations, args->output);

    startRow += adjustedHeight;
    printf("Thread %d starting at %d, ending at %d\n", args->threadId, startRow, startRow + numRows);
    mandelbrotSerial(args->x0, args->y0, args->x1, args->y1, 
                args->width, args->height, startRow, numRows,
                args->maxIterations, args->output);

    double endTime = CycleTimer::currentSeconds();

    printf("Thread %d finished in [%.3f] ms\n", args->threadId, (endTime - startTime) * 1000);

    return NULL;
}

//
// MandelbrotThread --
//
// Multi-threaded implementation of mandelbrot set image generation.
// Multi-threading performed via pthreads.
void mandelbrotThread(
    int numThreads,
    float x0, float y0, float x1, float y1,
    int width, int height,
    int maxIterations, int output[])
{
    const static int MAX_THREADS = 32;

    if (numThreads > MAX_THREADS)
    {
        fprintf(stderr, "Error: Max allowed threads is %d\n", MAX_THREADS);
        exit(1);
    }

    pthread_t workers[MAX_THREADS];
    WorkerArgs args[MAX_THREADS];

    for (int i=0; i<numThreads; i++) {
        // TODO: Set thread arguments here.
        args[i].x0 = x0;
        args[i].y0 = y0;
        args[i].x1 = x1;
        args[i].y1 = y1;
        args[i].width = width;
        args[i].height = height;
        args[i].maxIterations = maxIterations;
        args[i].numThreads = numThreads;
        args[i].output = output;
        args[i].threadId = i;
    }

    // Fire up the worker threads.  Note that numThreads-1 pthreads
    // are created and the main app thread is used as a worker as
    // well.

    for (int i=1; i<numThreads; i++)
        pthread_create(&workers[i], NULL, workerThreadStart, &args[i]);

    workerThreadStart(&args[0]);

    // wait for worker threads to complete
    for (int i=1; i<numThreads; i++)
        pthread_join(workers[i], NULL);
}
