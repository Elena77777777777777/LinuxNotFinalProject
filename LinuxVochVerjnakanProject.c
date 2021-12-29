#include <stdio.h>
#include <queue>
#include <unistd.h>
#include <pthread.h>
#include <malloc.h>
#include <stdlib.h>


class Task {
public:
    Task() {}
    virtual ~Task() {
        
    }
    virtual void run()=0;
    virtual void showTask()=0;
};


class WorkQueue {
public:
WorkQueue() {
   
    pthread_mutex_init(&qmtx,0);

    
    pthread_cond_init(&wcond, 0);
}

~WorkQueue() {
   
    pthread_mutex_destroy(&qmtx);
    pthread_cond_destroy(&wcond);
}

Task *nextTask() {
    
    Task *nxt = 0;

    pthread_mutex_lock(&qmtx);
   
    if (finished && tasks.size() == 0) {
        
        nxt = 0;
    } else {
        
        if (tasks.size()==0) {
            pthread_cond_wait(&wcond, &qmtx);
        }
       
        nxt = tasks.front();
        if(nxt){
        tasks.pop();
    }

       
        if (nxt) nxt->showTask();
    }
   
    pthread_mutex_unlock(&qmtx);
    return nxt;
}

void addTask(Task *nxt) {
   
    if (!finished) {
        
        pthread_mutex_lock(&qmtx);
       
        tasks.push(nxt);
       
        pthread_cond_signal(&wcond);
       
        pthread_mutex_unlock(&qmtx);
    }
}

void finish() {
    pthread_mutex_lock(&qmtx);
    finished = true;
   
    pthread_cond_signal(&wcond);
    pthread_mutex_unlock(&qmtx);
}


bool hasWork() {

    return (tasks.size()>0);
}

private:
std::queue<Task*> tasks;
bool finished;
pthread_mutex_t qmtx;
pthread_cond_t wcond;
};


void *getWork(void* param) {
Task *mw = 0;
WorkQueue *wq = (WorkQueue*)param;
while (mw = wq->nextTask()) {
    mw->run();
    delete mw;
}
pthread_exit(NULL);
}

class ThreadPool {
public:

ThreadPool(int n) : _numThreads(n) {
int rc;
    printf("Creating a thread pool with %d threads\n", n);
    threads = new pthread_t[n];
    for (int i=0; i< n; ++i) {
        rc = pthread_create(&(threads[i]), 0, getWork, &workQueue);
    if (rc){
     printf("ERROR; return code from pthread_create() is %d\n", rc);
     exit(-1);
        }
    }
}

~ThreadPool() {
    workQueue.finish();

    for (int i=0; i<_numThreads; ++i) {
        pthread_join(threads[i], 0);
    }
    delete [] threads;
}


void addTask(Task *nxt) {
    workQueue.addTask(nxt);
}

void finish() {
    workQueue.finish();
}


bool hasWork() {
    return workQueue.hasWork();
}

private:
pthread_t * threads;
int _numThreads;
WorkQueue workQueue;
};


static pthread_mutex_t console_mutex = PTHREAD_MUTEX_INITIALIZER;


void showTask(int n) {
pthread_mutex_lock(&console_mutex);
pthread_mutex_unlock(&console_mutex);
}


class FibTask : public Task {
public:
FibTask(int n) : Task(), _n(n) {}
~FibTask() {
    // Debug prints
    pthread_mutex_lock(&console_mutex);
    printf("tid(%d) - fibd(%d) being deleted\n", pthread_self(), _n);
    pthread_mutex_unlock(&console_mutex);        
}
virtual void run() {
   
    long long val = innerFib(_n);
   
    pthread_mutex_lock(&console_mutex);
    printf("Fibd %d = %lld\n",_n, val);
    pthread_mutex_unlock(&console_mutex);


}
virtual void showTask() {
   
    pthread_mutex_lock(&console_mutex);
    printf("thread %d computing fibonacci %d\n", pthread_self(), _n);
    pthread_mutex_unlock(&console_mutex);        
}
private:

long long innerFib(long long n) {
    if (n<=1) { return 1; }
    return innerFib(n-1) + innerFib(n-2);
}
long long _n;
};

int main(int argc, char *argv[]) 
{

ThreadPool *tp = new ThreadPool(10);


delete tp;

printf("\n\n\n\n\nDone with all work!\n");
}