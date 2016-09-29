#define BUFFER_PTR(X,Y) {INTERMEDIATE_T *filter2D_out##X = (INTERMEDIATE_T *)(scratchpad + ((Y+X)*conv_size*conv_size));}
#define KERNEL_PTR(X,Y) {KERNEL_T *kernel##X = (KERNEL_T *)(kernels + ((Y+X)*KERNEL_SIZE));}
#define KERNEL_PTR_FLOAT(X,Y) {KERNEL_T *oKernel##X = (KERNEL_T *)(kernels + ((Y+X)*KERNEL_SIZE));\
				floatify(oKernel##X,kernel##X,KERNEL_SIZE);}
#define FLOAT_KERNEL_INIT(X,Y) {float *kernel##X = (float *)(scratchpad + ((Y+X)*KERNEL_SIZE));}
#define MAP_PTR(X,Y) {MAP_T *map##X = (MAP_T *)(maps + ((Y+X)*MAP_SIZE));}
#define SCALE_PTR(X,Y) {SCALE_T scale##X = kernel_scales[Y+X];}
#define FILTER2D(X) {filter2D(kernel##X,image,filter2D_out##X,KERNEL_WIDTH,IMAGE_WIDTH,conv_size,scale##X);}
#define MAXPOOL_SUBSAMPLE(X) {maxpool_subsample(filter2D_out##X,map##X,DOWN_FAC,conv_size);}

//Inspired from: http://stackoverflow.com/questions/28231743/self-unrolling-macro-loop-in-c-c
//for repeating macro based on number of arguments.. support up to 5.
#define PTR_UNROLL_1(X,Y) X(0,Y)
#define PTR_UNROLL_2(X,Y) X(0,Y) X(1,Y)
#define PTR_UNROLL_3(X,Y) X(0,Y) X(1,Y) X(2,Y)
#define PTR_UNROLL_4(X,Y) X(0,Y) X(1,Y) X(2,Y) X(3,Y)
#define PTR_UNROLL_5(X,Y) X(4,Y) PTR_UNROLL_4(X,Y)

#define PTR_EXPAND(...) __VA_ARGS__

#define PTR_UNROLL__(N,X,Y) PTR_EXPAND(PTR_UNROLL_##N)(X,Y)
#define PTR_UNROLL_(N,X,Y) PTR_UNROLL__(N,X,Y)
#define PTR_UNROLL(N,X,Y) PTR_UNROLL_(PTR_EXPAND(N),X,Y)

#define FUNC_UNROLL_1(X) X(0)
#define FUNC_UNROLL_2(X) X(0) X(1)
#define FUNC_UNROLL_3(X) X(0) X(1) X(2)
#define FUNC_UNROLL_4(X) X(0) X(1) X(2) X(3)
#define FUNC_UNROLL_5(X) X(4) FUNC_UNROLL_4(X)

#define FUNC_EXPAND(...) __VA_ARGS__

#define FUNC_UNROLL__(N,X) FUNC_EXPAND(FUNC_UNROLL_##N)(X)
#define FUNC_UNROLL_(N,X) FUNC_UNROLL__(N,X)
#define FUNC_UNROLL(N,X) FUNC_UNROLL_(FUNC_EXPAND(N),X)
