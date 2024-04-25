#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <sched.h>

pthread_barrier_t barrier;
double time_wait = 0.0;

typedef struct {
    pthread_t thread_id;
    int thread_num;
    int sched_policy;
    int sched_priority;
} thread_info_t;

void busy_wait(double seconds) {
    clock_t start_time = clock();

    while ((clock() - start_time) / CLOCKS_PER_SEC < seconds) {
        double result = 1.0;
        for (int i = 1; i <= 1000000; i++) {
            result += 1.0 / i;
        }
    }
}

void *thread_func(void *arg) {
    thread_info_t *thread_info = (thread_info_t *)arg;

    pthread_barrier_wait(&barrier);

    for (int i = 0; i < 3; i++) {
        printf("Thread %d is running\n", thread_info->thread_num);
        busy_wait(time_wait);
    }
    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    int opt;
    int num_threads = 0;
    
    char *scheduling_policies = NULL;
    char *priorities = NULL;

    // 使用 getopt 解析命令
    while((opt = getopt(argc, argv, "n:t:s:p:")) != -1){
        switch(opt){
           case 'n':
                num_threads = atoi(optarg);
                break;
           case 't':
                time_wait = atof(optarg);
                break;
           case 's':
                scheduling_policies = optarg;
                break;
           case 'p':
                priorities = optarg;
                break;
           default:
                fprintf(stderr, "Usage: %s -n <num_threads> -t <time_wait> -s <policies> -p <priorities>\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }
    pthread_t *threads = (pthread_t *)malloc(num_threads * sizeof(pthread_t));
    thread_info_t thread_info[num_threads];

    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(0, &cpuset);
    
    pthread_barrier_init(&barrier, NULL, num_threads);

    /*if (pthread_setaffinity_np(thread_info->thread_id, sizeof(cpu_set_t), &cpuset) != 0) {
        perror("pthread_setaffinity_np");
    }*/
    
    char *sched_policy = strtok_r(scheduling_policies, ",", &scheduling_policies);
    char *priority = strtok_r(priorities, ",", &priorities);
    int pri = 0;
    
    
    for (int i = 0; i < num_threads; i++) {
    	if(sched_policy != NULL && priority != NULL){
		pthread_attr_t attr;
		//pthread_barrier_init(&thread_info[i].barrier, NULL, num_threads);
		
		thread_info[i].thread_num = i;
		
		sched_setaffinity(threads[i], sizeof(cpu_set_t), &cpuset);
		
		struct sched_param param;
		param.sched_priority = atoi(priority);
		pthread_attr_init(&attr);
		pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
		if(strcmp(sched_policy,"FIFO")==0){
		   pri = 1;
		}
		else if(strcmp(sched_policy,"NORMAL")==0){
		   pri = 0;
		}
		pthread_attr_setschedpolicy(&attr, pri);
		pthread_attr_setschedparam(&attr, &param);
		
		pthread_create(&threads[i], &attr, thread_func, (void *)&thread_info[i]);
		
		sched_policy = strtok_r(NULL, ",", &scheduling_policies);
		priority = strtok_r(NULL, ",", &priorities);
	}
	else{
		break;
	}
    }
    // 同時啟動所有執行緒
    //pthread_barrier_wait(&thread_info[0].barrier);

    // 等待所有執行緒完成
    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }
    
    pthread_barrier_destroy(&barrier);
    
    return 0;
}
