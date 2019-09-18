
#include<stdio.h>
#include<iostream>
#include<fstream>
#include<vector>
#include<cmath>
#include<random>
#include<algorithm>
#include"bitonic.hxx"
#include"buildData.h"



template<typename T>
__global__ void bitonicSort(T* data, int N);

int main(int argc, char** argv) {

  if(argc != 4) {
	printf("usage: bitonic <N> <BLOCKS> <THREADS>\n");
	exit(1);
  }

  cudaEvent_t start, stop;
  float time_elapsed=0.0;
  int N = pow(2,atoi(argv[1]));
  int BLOCKS = atoi(argv[2]);
  int THREADS = atoi(argv[3]);

  TYPE* h_data = (TYPE*)malloc(N*sizeof(TYPE));

  TYPE* d_data;
  cudaMalloc(&d_data, N*sizeof(TYPE));
  float total_time=0.0;

  srand(time(NULL));

  create_random_list<TYPE>(h_data, N, 0);

  cudaMemcpy(d_data, h_data, N*sizeof(TYPE), cudaMemcpyHostToDevice);

  cudaEventCreate(&start);
  cudaEventCreate(&stop);
  cudaEventRecord(start, 0);


  bitonicSort<TYPE,cmp>(d_data,N,BLOCKS, THREADS);

  cudaDeviceSynchronize();

  cudaEventRecord(stop,0);
  cudaEventSynchronize(stop);
  cudaEventElapsedTime(&time_elapsed, start, stop);

  printf("%lf\n", time_elapsed);

  cudaMemcpy(h_data, d_data, N*sizeof(TYPE), cudaMemcpyDeviceToHost);

#ifdef DEBUG
  bool error=false;
  for(int i=1; i<N; i++) {
    if(h_data[i-1] > h_data[i]) {
      error=true;
      printf("i:%d, %d > %d\n", i,h_data[i-1], h_data[i]);
    }
  }

  if(error)
    printf("NOT SORTED!\n");
  else
    printf("SORTED!\n");
#endif

  cudaFree(d_data);
  free(h_data);
}

