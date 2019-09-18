

#include<stdio.h>
#include<iostream>
#include<fstream>
#include<vector>
#include<cmath>
#include<random>
#include<algorithm>

#define RANGE 1000000


template<typename T>
void create_random_sorted_list(T* data, int size) {
  for(int i=0; i<size; i++) {
    data[i] = {.key = rand()%RANGE, .val= rand()%RANGE};
  }
  std::sort(data, data+size);
}


template<typename T>
void create_random_list(T* data, int size, int min) {
  srand(time(NULL));
  long temp;
  for(int i=0; i<size; i++) {

    data[i] = (rand()%RANGE) + min + 1;
    temp = rand()%RANGE + min + 1;
    data[i] += (temp<<32);
  }
}
