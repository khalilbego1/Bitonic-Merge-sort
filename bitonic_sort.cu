

#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#define THREADS 512 
#define BLOCKS 32768 
#define NUM_VALS THREADS*BLOCKS

void print_elapsed(clock_t start, clock_t stop)
{
  double elapsed = ((double) (stop - start)) / CLOCKS_PER_SEC;
  printf("Elapsed time: %.3fs\n", elapsed);
}

float random_float()
{
  return (float)rand()/(float)RAND_MAX;
}

void array_print(float *arr, int length) 
{
  int i;
  for (i = 0; i < length; ++i) {
    printf("%1.3f ",  arr[i]);
  }
  printf("\n");
}

void array_fill(float *arr, int length)
{
  srand(time(NULL));
  int i;
  for (i = 0; i < length; ++i) {
    arr[i] = random_float();
  }
}

__global__ void bitonic_sort_step(float *dev_values, int j, int k)
{
  unsigned int i, ixj; 
  i = threadIdx.x + blockDim.x * blockIdx.x;
  ixj = i^j;

  if ((ixj)>i) {
    if ((i&k)==0) {
      if (dev_values[i]>dev_values[ixj]) {
        float temp = dev_values[i];
        dev_values[i] = dev_values[ixj];
        dev_values[ixj] = temp;
      }
    }
    if ((i&k)!=0) {
      if (dev_values[i]<dev_values[ixj]) {
        float temp = dev_values[i];
        dev_values[i] = dev_values[ixj];
        dev_values[ixj] = temp;
      }
    }
  }
}


void bitonic_sort(float *values)
{
  float *dev_values;
  size_t size = NUM_VALS * sizeof(float);

  cudaMalloc((void**) &dev_values, size);
  cudaMemcpy(dev_values, values, size, cudaMemcpyHostToDevice);

  dim3 blocks(BLOCKS,1);    
  dim3 threads(THREADS,1);  
  int j, k;
  for (k = 2; k <= NUM_VALS; k <<= 1) {
    for (j=k>>1; j>0; j=j>>1) {
      bitonic_sort_step<<<blocks, threads>>>(dev_values, j, k);
    }
  }
  cudaMemcpy(values, dev_values, size, cudaMemcpyDeviceToHost);
  cudaFree(dev_values);
}

int main(void)
{
  clock_t start, stop;

  float *values = (float*) malloc( NUM_VALS * sizeof(float));
  array_fill(values, NUM_VALS);

  start = clock();
  bitonic_sort(values);
  stop = clock();

  print_elapsed(start, stop);
}
