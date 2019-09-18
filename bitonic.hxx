
#include<stdio.h>
#include<iostream>
#include<fstream>
#include<vector>
#include<cmath>
#include<random>
#include<algorithm>
#include"basecase/squareSort.hxx"

#define TYPE long int

#define ILP 1
#define BUFF 8

template<typename T, fptr_t f>
__global__ void bitonicKernel(T* data, int N) {
	squareSortDevice<T,f>(data, N);
}

template<typename T, fptr_t f>
__forceinline__ __device__ void cmpSwapRev(T* data, int idx1, int idx2) {
  T v1[ILP];
  T v2[ILP];

  T temp[ILP];

#pragma unroll
  for(int i=0; i<ILP; i++) {
    temp[i] = data[idx1+i];
    v2[i] = data[idx2-i];
  }
#pragma unroll
  for(int i=0; i<ILP; i++) {
    v1[i] = temp[i];
    if(f(v2[i],temp[i])) {
      v1[i] = v2[i];
      v2[i] = temp[i];
    }
  }
#pragma unroll
  for(int i=0; i<ILP; i++) {
    data[idx1+i] = v1[i];
    data[idx2-i] = v2[i];
  }
}

template<typename T, fptr_t f>
__forceinline__ __device__ void cmpSwapRegs(T* a, T* b) {
	T temp = *a;
	*a = *b;
	*b = temp;
}

template<typename T, fptr_t f>
__forceinline__ __device__ void cmpSwap(T* data, int idx1, int idx2) {
  T v1[ILP];
  T v2[ILP];

  T temp[ILP];

#pragma unroll
  for(int i=0; i<ILP; i++) {
    temp[i] = data[idx1+i];
    v2[i] = data[idx2+i];
  }
#pragma unroll
  for(int i=0; i<ILP; i++) {
    v1[i] = temp[i];
    if(f(v2[i],temp[i])) {
      v1[i] = v2[i];
      v2[i] = temp[i];
    }
  }
#pragma unroll
  for(int i=0; i<ILP; i++) {
    data[idx1+i] = v1[i];
    data[idx2+i] = v2[i];
  }
}
/*
template<typename T, fptr_t f>
__global__ void swapAllBlock(T* data, int N, int dist) {
  int eltsPerBlock = N/blockDim.x;

  for(int chunk=0; chunk < chunksPerBlock; chunk++) {
    for(int i=threadIdx.x; i<M*dist; i+=blockDim.x) {
      cmpSwap<T,f>(data, (chunkStart+chunk)*M*2*dist + i, (chunkStart+chunk)*M*2*dist +i+(M*dist));
    }
  }
}
*/
template<typename T, fptr_t f>
__global__ void swapAllBlock(T* data, int N, int dist) {
  int totalChunks = N/(M*2*dist);
  int chunksPerBlock = totalChunks/gridDim.x;
  if(threadIdx.x==0 && blockIdx.x==0) {
  printf("chunksPerBlock:%d\n", chunksPerBlock);
  }
  
  if(chunksPerBlock > 0) {
    int chunkStart = blockIdx.x*chunksPerBlock;

    for(int chunk=0; chunk < chunksPerBlock; chunk++) {
      for(int i=threadIdx.x; i<M*dist; i+=blockDim.x) {
        cmpSwap<T,f>(data, (chunkStart+chunk)*M*2*dist + i, ((chunkStart+chunk)*M*2*dist)+i+(M*dist));
      }
    }
  }

}

template<typename T, fptr_t f>
__global__ void swapAll(T* data, int N, int dist) {
  int globalId = threadIdx.x + blockIdx.x*blockDim.x;
//int i=0;
  for(int i=0; i<N; i+=M*dist*2) {
//    for(int j=globalId*ILP; j<M*dist; j+=gridDim.x*blockDim.x*ILP) {
    int j=globalId;
    if(j < M*dist)
      cmpSwap<T,f>(data, i+j, M*dist+i+j);
    }
//  }
}

template<typename T, fptr_t f>
__global__ void swapAllRevRegs(T* data, int N, int dist) {
	int globalId = threadIdx.x + blockIdx.x*blockDim.x;
	int count=0;
	T buff1[BUFF];
	T buff2[BUFF];
	for(int i=0; i<N; i+=M*dist*2) {
		for(int j=globalId*ILP; j<M*dist; j+=gridDim.x*blockDim.x*ILP) {
			buff1[count] = data[i+j];
			buff2[count] = data[i+(M*dist*2)-j-1];
			count++;
			if(count == BUFF-1) {
				for(int k=0; k<count; k++) {
					cmpSwapRegs<T,f>(buff1+i, buff2+i);
				}
				for(int k=0; k<count; k++) {
					data[i+j - (k*gridDim.x*blockDim.x)] = buff1[i];
					data[i+(M*dist*2)-j-1 + (k*gridDim.x*blockDim.x)] = buff2[i];
				}
				count = 0;
			}
		}
	}
}

template<typename T, fptr_t f>
__global__ void swapAllRev(T* data, int N, int dist) {
  int globalId = threadIdx.x + blockIdx.x*blockDim.x;

  for(int i=0; i<N; i+=M*dist*2) {
    for(int j=globalId*ILP; j<M*dist; j+=gridDim.x*blockDim.x*ILP) {
      cmpSwapRev<T,f>(data, i+j, i+(M*dist*2)-j-1);
//      cmpSwap<T,f>(data, i+j, i+(M*dist*2)-j-1);
    }
  }
}

template<typename T,fptr_t f>
void bitonicSort(T* data, int N, int BLOCKS, int THREADS) {

  int baseBlocks=((N/M)/(THREADS/W));
  int roundDist=1;
  int subDist=1;


  squareSort<T,f><<<baseBlocks,32>>>(data, N);
  cudaDeviceSynchronize();

  int levels = (int)log2((float)(N/M)+1)+1;

  for(int i=1; i<levels; i++) {
	  
    swapAllRev<T,f><<<BLOCKS,THREADS>>>(data,N,roundDist);

    cudaDeviceSynchronize();
    subDist = roundDist/2;
    for(int j=i-1; j>0; j--) {	

      swapAll<T,f><<<BLOCKS,THREADS>>>(data,N,subDist);
      cudaDeviceSynchronize();
      subDist /=2;
    }


    roundDist *=2;
  }

  cudaDeviceSynchronize();
}
