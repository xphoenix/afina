#include <afina/allocator/Simple.h>
#include "math.h"
#include <afina/allocator/Pointer.h>
#include <afina/allocator/Error.h>
#define MIN_BLOCK_SIZE 4

namespace Afina {
namespace Allocator {


		
//get info about the block from the pointer
int get_size(void *block){
	return *(int*)((char*)block+sizeof(char));
}
	
void* get_link_l(void *block){
	return *(void**)((char*)block+sizeof(char)+sizeof(int));
}

void* get_link_r(void *block){
	return *(void**)((char*)block+sizeof(char)+sizeof(int)+sizeof(void*));
}

void* get_pointer(void *block){
	return *(void**)((char*)block+sizeof(char)+sizeof(int));
}

int back_get_size(void *endofblock){
	return *(int*)((char*)endofblock-sizeof(char)-sizeof(int));
}

void* get_data(void *block){
	return ((char*)block+sizeof(char)+sizeof(int)+sizeof(void*));
}
//set info in block 
void set_size(void *block,int size){
	*(int*)((char*)block+sizeof(char))=size;
}
	
void set_link_l(void *block,void* link){
	*(void**)((char*)block+sizeof(char)+sizeof(int))=link;
}

void set_link_r(void *block,void* link){
	*(void**)((char*)block+sizeof(char)+sizeof(int)+sizeof(void*))=link;
}

void set_pointer(void *block,void* pointer){
	*(void**)((char*)block+sizeof(char)+sizeof(int))=pointer;
}

void back_set_size(void *endofblock,int size){
	*(int*)((char*)endofblock-sizeof(char)-sizeof(int))=size;
}

void* block_from_pointer(Pointer &p) {
	return ((char*)p.get()-sizeof(int)-sizeof(char)-sizeof(void*));
}

void* left_neighbour(void * block){
   return ((char*)block-*(int*)((char*)block-sizeof(char)-sizeof(int)));
}
//how much memory will the block with N bytes of data occupy
int realsize(size_t N){
	return N+2*sizeof(char)+sizeof(int)+sizeof(void*);
}
//to rewrite data from one place to another
void rewrite(void* from,void* to,size_t N){
	for(int i=0;i<N;i++)
		*(char*)((char*)to+i)=*(char*)((char*)from+i);
}
	
//move the empty_slot pointer to an unoccupied position
int Simple::renew_empty_slot(){
	while((*((void**)empty_slot)!=NULL)&&(((char*)empty_slot)<((char*)start))){
		empty_slot=((void**)empty_slot+1);
	}
	if (((char*)empty_slot)>=((char*)start))
		return 0;
	else
		return 1;
}
//find the nearest free block for given block (using the list of free blocks)
void * Simple::nearest_free_block(void* block){
	void* free_block;
	free_block=*(void**)base;
	if (((char*)free_block)>((char*)block))
		return NULL;
	else
		while((((char*)get_link_r(free_block))<((char*)block))&&(get_link_r(free_block)!=NULL))
			free_block=get_link_r(free_block);
	return free_block;
}

//find the appropriate place to fit the block of size N (using the list of free blocks)	
void* Simple::find_to_alloc(size_t N){
	void* next_block;
	if (curr_block==NULL)
		if(*((void**)base)==NULL)
			return NULL;
		else
			curr_block=*((void**)base);
	void* starting_block=curr_block;
	next_block=curr_block;
	do{
		if(get_size(next_block)>=realsize(N)){
			curr_block=get_link_r(next_block);
			return next_block;
		}
	if(get_link_r(next_block)!=NULL)
		next_block=get_link_r(next_block);
	} while(get_link_r(next_block)!=NULL);
	curr_block=*((void**)base);
	next_block=curr_block;
	do{
		if(get_size(next_block)>=realsize(N)){
			curr_block=get_link_r(next_block);
			return next_block;
		}
	if(get_link_r(next_block)!=NULL)	
		next_block=get_link_r(next_block);
	} while(next_block!=starting_block);
	return NULL;
}
//to assign a pointer to a fresh block and store it in the empty slot
void* Simple::give_pointer(void * block){
	*((void**)empty_slot)=get_data(block);
	set_pointer(block,empty_slot);
	return empty_slot;
}
	
	
//to take all the memory of the free block,returning the place the pointer to the occupied block in the storage of pointers
void* Simple::occupy_block(void* block){
	*((char*)block)='+';
	*((char*)block+get_size(block)-1)='+';
	if (get_link_l(block)!=NULL)
		set_link_r(get_link_l(block),get_link_r(block));
	else
		*(void**)base=get_link_r(block);
	if (get_link_r(block)!=NULL)
		set_link_l(get_link_r(block),get_link_l(block));
	return give_pointer(block);
}

//to slice a free block into a occupied block and a smaller free block, returning the place the pointer to the occupied block in the storage of pointers
void* Simple::slice_block(void* block,size_t N){
	void* newblock;
	newblock=(char*)block+realsize(N);
	set_link_l(newblock,get_link_l(block));
	set_link_r(newblock,get_link_r(block));
	
	if (get_link_l(block)!=NULL)
		set_link_r(get_link_l(block),newblock);
	else
		*(void**)base=newblock;
	if (get_link_r(block)!=NULL){
		set_link_l(get_link_r(block),newblock);
	}
	set_size(newblock,get_size(block)-realsize(N));
	back_set_size(((char*)newblock+get_size(newblock)),get_size(newblock));
	*((char*)block+get_size(block)-1)='-';
	set_size(block,realsize(N));
	*((char*)block+realsize(N)-1)='+';
	*((char*)newblock)='-';
	*((char*)block)='+';
	return give_pointer(block);
}
	
//to shrink the occupied block in the process of reallocating memory
void Simple::shrink_block(void* block,size_t N){
	*((char*)block+realsize(N)-1)='+';
	void *newblock;
	newblock=((char*)block+realsize(N));
	set_size(newblock,get_size(block)-realsize(N));
	create_free_block(newblock);
	set_size(block,realsize(N));
}
//to make the block free and possibly merge it with neighbouring free blocks
void Simple::create_free_block(void* block){
	void * llink;
	void * rlink;
	int shift;
	shift=0;
	int totalsize;
	totalsize=get_size(block);
	//if the left neighbour is free
	if(*(char*)((char*)block-1)=='-'){
		llink=get_link_l(left_neighbour(block));
		shift-=get_size(left_neighbour(block));
		totalsize-=shift;
	}
	else
		llink=nearest_free_block(block);
	//if the right one is free
	if((char*)((char*)block+get_size(block))<(char*)((char*)base+size)){
		if(*(char*)((char*)block+get_size(block))=='-'){
			rlink=get_link_r(((char*)block+get_size(block)));
			totalsize+=get_size(((char*)block+get_size(block)));
		}
		else{
			if(llink!=NULL){
				if(((char*)get_link_r(llink))<((char*)block))
					rlink=get_link_r(get_link_r(llink));
				else
					rlink=get_link_r(llink);
			} else{
				rlink=*(void**)base;
			}
		}
	}
	//now we got all the info and can make changes to memory and list of free blocks
	void* newblock;
	newblock=((char*)block+shift);
	*((char*)newblock)='-';
	*(char*)((char*)newblock+totalsize-1)='-';
	set_size(newblock,totalsize);
	back_set_size(((char*)newblock+totalsize),totalsize);
	if (llink==NULL)
		*(void**)base=newblock;
	else{
		set_link_r(llink,newblock);
	}
	set_link_l(newblock,llink);
	set_link_r(newblock,rlink);
	if (rlink!=NULL)
		set_link_l(rlink,newblock);
}	
	

	
//to clean a block and delete the pointer to this block in the storage of pointers
void Simple::clean_block(void *block){
	*(void**)get_pointer(block)=NULL;
	empty_slot=get_pointer(block);
	create_free_block(block);
}

Simple::Simple(void *base, size_t size) {
	this->base=base;
	this->size=size;
	pointers_reserved=size/10;
	space=size-pointers_reserved-sizeof(void*);
	start=((char*)base)+pointers_reserved+sizeof(void*);
	empty_slot=((char*)base+sizeof(void*));
	//we start with one big free block
	*(void**)base=((char*)base)+size-space;
	*(char*)start='-';
	set_size(start,space);
	set_link_l(start,NULL);
	set_link_r(start,NULL);
	*((char*)start+space-1)='-';
	back_set_size(((char*)start+space-1),space);
	curr_block=start;
}




Pointer Simple::alloc(size_t N) {
	if (renew_empty_slot()==0){
		throw AllocError(AllocErrorType::NoMemory,"NO memory\n");//Throw an exception here, no place to store pointers
	}
		Pointer p;
	void *target_block;
	target_block=find_to_alloc(N);
	if (target_block==NULL){
		throw AllocError(AllocErrorType::NoMemory,"NO memory\n");//Throw an exception here, no place to fit the request
	}
	else{//if we can slice the block, we do, if not, we occupy all of its memory.
		if(realsize(N)>get_size(target_block)-realsize(0)-sizeof(void*)-sizeof(int)-MIN_BLOCK_SIZE)
			p.set(occupy_block(target_block));
		else
			p.set(slice_block(target_block,N));
		return p;
	}
}	
			
void Simple::realloc(Pointer &p, size_t N) {
	void* target_block;
	//if the pointer is new to us, simply call alloc
	if (p.get()==NULL){
		p=alloc(N);
	}
	else{
	target_block=block_from_pointer(p);	
	// if it is possible to shrink the block in a way such that a new free block can be created, do so
	if(realsize(N)+realsize(0)+sizeof(void*)+sizeof(int)+MIN_BLOCK_SIZE<get_size(target_block)){
		shrink_block(target_block,N);
	}
	else{
		if (realsize(N)<=get_size(target_block)){}//if it is impossible yet there is no need to alloc more memory, do nothing
		else{
			void * rblock;
			rblock=((char*)target_block+get_size(target_block));
			//if we can realloc using the free block in the right, do so
			if ((*((char*)rblock)=='-')&&(get_size(rblock)+get_size(target_block)>=realsize(N))){
				//checking if we need to create a new free block afterwardst
				if(get_size(rblock)+get_size(target_block)>=realsize(N)+realsize(0)+sizeof(void*)+sizeof(int)+MIN_BLOCK_SIZE){
					void* backup;
					backup=*((void**)empty_slot);
					slice_block(rblock,realsize(N-get_size(target_block)));
					*(void**)empty_slot=backup;
					set_size(target_block,realsize(N));
					
				}
				else{
					void* backup;
					backup=*((void**)empty_slot);
					occupy_block(rblock);
					*(void**)empty_slot=backup;
					set_size(target_block,get_size(target_block)+get_size(rblock));
				}
			}
			else{
				//at this point we are already content with the fact that we need to move the data so we prepare for rewriting
				void* data;
				int datasize;
				Pointer newp;
				data=get_data(target_block);
				datasize=get_size(target_block)-realsize(0);
				//checking if it is possible to find a free block big enough for our purpose
				if(find_to_alloc(N)!=NULL){
					empty_slot=get_pointer(target_block);
					newp=alloc(N);
					void* backup;
					backup=*((void**)empty_slot);
					*(void**)((char*)newp.get()-sizeof(void*))=get_pointer(target_block);
					*(void**)get_pointer(target_block)=newp.get();
					rewrite(data,newp.get(),datasize);
					*((void**)empty_slot)=NULL;
					clean_block(target_block);
					*((void**)empty_slot)=backup;
				}
				//if this fails, we can try to use our left neighbour
				else{
					if(*(char*)((char*)target_block-1)!='-')
						throw AllocError(AllocErrorType::NoMemory,"NO memory\n");
					else{
						void * lblock;
						lblock=left_neighbour(target_block);
						if (get_size(lblock)+datasize<=realsize(N)){
							set_size(lblock,(get_size(lblock)+get_size(target_block)));
							void* backup;
							backup=*((void**)empty_slot);
							occupy_block(rblock);
							set_pointer(rblock,get_pointer(target_block));
							rewrite(data,get_data(rblock),datasize);
							*(void**)get_pointer(target_block)=get_data(rblock);
							*((void**)empty_slot)=backup;
						}
						else{
							throw AllocError(AllocErrorType::NoMemory,"NO memory\n");
						}
					}
				}
			}
		}
	}}		

}

void Simple::free(Pointer &p) {
	void* target_block;
	target_block=block_from_pointer(p);
	//here we check the correctness of p
	if((((char*)target_block>((char*)(base+size)))||((char*)target_block<(char*)start))||(target_block==NULL))
		throw AllocError(AllocErrorType::InvalidFree,"InvalidFree\n");
	if(  (  (  ((char*)(get_pointer(target_block))>(char*)start)
	||(  (char*)(get_pointer(target_block))<(char*)base)  )  )||
	((((char*)(get_pointer(target_block))-(char*)base)%sizeof(void*))!=0))
		throw AllocError(AllocErrorType::InvalidFree,"InvalidFree\n");
	clean_block(target_block);
}

void Simple::defrag() {
	void* free_place;
	void* current_place;
	int shift;
	free_place=*(void**)base;
	current_place=free_place;
	//stack the blocks from the current place to free place (to the left)
	while(((char*)current_place)<((char*)base+size)){
		if(*((char*)current_place)=='-'){
			//simply skip free blocks
			current_place=(void**)((char*)current_place+get_size(current_place));
		}
		else{
			//rewrite the data from occupied blocks, also pay attention to their pointers in pointer storage
			shift=get_size(current_place);
			rewrite(current_place,free_place,shift);
			(*(void**)get_pointer(current_place))=(char*)(*(void**)get_pointer(current_place))-((char*)current_place-(char*)free_place);
			free_place=((char*)free_place+shift);
			current_place=((char*)current_place+shift);
		}
	}
	//organise all free space left as one large free block
	*((char*)free_place)='-';
	set_size(free_place,size-((char*)free_place-(char*)base));
	set_link_r(free_place,NULL);
	set_link_l(free_place,NULL);
	back_set_size(((char*)free_place+get_size(free_place)),get_size(free_place));
	*((char*)free_place+get_size(free_place))='-';
	*(void**)base=free_place;
	curr_block=free_place;
}

