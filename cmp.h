

#define DATATYPE int // Datatype to sort
#define CASTTYPE int // Datatype to cast key to in order to use __shfl()


// FOR INTEGERS
#define MAXVAL 2147483647
#define MINVAL -2147483647
// FOR LONGS
//#define MAXVAL 0x0000ffff00000001
//#define MINVAL 0x000000010000ffff 

#define W 32
#define ELTS 32
#define M 1024

// Define abstract format of comparison function
typedef int(*fptr_t)(DATATYPE, DATATYPE);

// Comparison function used to sort by.
// Edit this to be whatever comparison function is needed.
template<typename T>
__forceinline__ __device__ int cmp(T a, T b) {
  return a < b; // Basic less than comparison



}


template<typename T>
__forceinline__ int host_cmp(T a, T b) {
  return a < b;
}
+
// Variable used for debugging and performance counting
__device__ int tot_cmp;
