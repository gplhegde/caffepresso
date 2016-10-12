#ifndef PREFETCH_HPP
#define PREFETCH_HPP
#include "Vector.hpp"
namespace VBX{
	template<typename T>
	class Prefetcher{
		Vector<T>* vecs;
		int num_vecs;
		int current_vec;
		bool full;
	public:
		Prefetcher(int num,size_t vec_size)
			:num_vecs(num+1)
		{
			VBX::default_vec_size.set(vec_size);
			vecs = new Vector<T>[num_vecs];
			full=0;
			current_vec=0;
		}
		void fetch( T* from)
		{
			vecs[current_vec].dma_read(from);
			if (++current_vec >=num_vecs){
				full=1;
				current_vec=0;
			}
		}
		//rotate the fifo without a dma, useful for last
		//buffer
		void rotate()
		{
			if (++current_vec >=num_vecs){
				full=1;
				current_vec=0;
			}
		}
		Vector<T>& operator[](int nth_vector)
		{
			int actual_vector=nth_vector;
			if( full){
				actual_vector+= current_vec  ;
			}
			/*assume this loop is faster than a modulus operation*/
			/*only true if nth_vector is not too much larger than num_vecs*/
			while(actual_vector >= num_vecs){
				actual_vector -= num_vecs;
			}
			return vecs[actual_vector];
		}
		~Prefetcher()
		{
			delete[] vecs;
		}
	};
}
#endif //PREFETCH_HPP
