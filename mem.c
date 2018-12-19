#include <stdlib.h>
#include <stdio.h>
#include <sys/mman.h>
#include <fcntl.h>
#include "mem.h"
#include <unistd.h>

//record size and next of free space of heap
typedef struct __node_t {
int  size;
struct __node_t *next;
} node_t;

//to record size and magic number of allocated
typedef struct __header_t {
int size;
int magic;
} header_t;

node_t* head = NULL;


static int coalesce(node_t* a,node_t* b){
	//printf("a: %p   a->size:%d  sizeofnode:%ld  sizeofheader:%ld\n",a,a->size,sizeof(node_t),sizeof(header_t));
	//printf("%p  %p \n",a + a->size + sizeof(node_t),b);
	if(a == NULL || b == NULL)
		return -1;

	else if((void*)a + a->size + sizeof(node_t) == (void*)b){
		a->size = a->size + sizeof(node_t) + b->size;
		a->next = b->next;
		return 1;
	}
	return 0;
}


//return pre_finded, so finded is pre_finded -> next or head
static node_t* FindFit(int style,int size,node_t* finded,node_t* pre_finded){
	size -= sizeof(node_t);
	node_t* p = NULL;
	node_t* q = head;
	while(q != NULL){
		if(q->size >= size){// make sure q is useful
			if(finded->size >= size && (style == M_BESTFIT && q->size > finded->size)
				              || (style == M_WORSTFIT && q->size < finded->size)){
				//these are situations not choose q
				p = q;
				q = q -> next;
				continue;
			}
			pre_finded = p;
			finded = q;
			if(style  == M_FIRSTFIT)
				break;
		}
		p = q;
		q = q -> next;
		//printf("finded in Find is %p\n",finded);
		//printf("pre_finded in find is %p\n",pre_finded);
	}
	return pre_finded;
	

}

int mem_init(int size_of_region){
	
	//make size_of_region is times of pagesize
	int pagesize = getpagesize();
	int temp = size_of_region % pagesize ? 1 : 0;
	size_of_region = (temp + size_of_region/pagesize) * pagesize;
	printf("size_of_region : %d\n",size_of_region);

	int fd = open("/dev/zero",O_RDWR);
    head = mmap(NULL,size_of_region,PROT_READ | PROT_WRITE,MAP_PRIVATE,fd,0);
    head -> size = size_of_region - sizeof(node_t);
    head -> next = NULL;
    //printf("%p\n",head);
    if(head == MAP_FAILED){
    	perror("mmap");
    	printf("mem_init failed\n");
    	m_error = E_BAD_ARGS;
    	exit(1);
    }
    close(fd);
	return 0;
}


void* mem_alloc(int size,int style){
	if(head == NULL){
		printf("head is null\n");
		exit(1); 
	}

	//update size
	int flag = size%8 ? 1 : 0;
	size = (size/8 + flag) * 8 + sizeof(header_t);
	//printf("allocsize: %d\n",size);

	node_t* finded = head;
	node_t* pre_finded = NULL;
	printf("before address of finded is %p\n", finded);
	pre_finded = FindFit(style,size,finded,pre_finded);
	if(pre_finded != NULL)
		finded = pre_finded -> next;
	printf("after address of finded is %p\n", finded);
	//printf("pre_finded is %p\n",pre_finded);

	if(pre_finded == NULL){
		if(head -> size < size - sizeof(node_t)){
			printf("no enough space\n");
			m_error = E_NO_SPACE;
			return NULL;
		}
		int temp = head -> size - size;
		if(temp <= 0){
			head = head->next;
		}
		else{
			node_t* nt = head -> next;
			head = (node_t*)((void*)head + size);
			head -> size = temp;
			head -> next = nt;
		}
		
	}
	else{
		int temp = finded -> size - size;
		if(temp <= 0){
			pre_finded -> next = finded -> next;
		}
		else{
			node_t* nt = finded -> next;
			node_t* hfinded = (node_t*)((void*)finded + size);
			hfinded -> size = temp;
			hfinded -> next = nt;
			pre_finded -> next = hfinded;
		}
		
	}

	void* result = (void*)finded + sizeof(header_t);
	header_t* p = (header_t*)((void*)finded);
	p -> size = size - sizeof(header_t);
	p -> magic = 12345678;
	//printf("allocated size: %d\n",p->size);
    return result;
}



int mem_free(void *ptr){
	if(ptr != NULL){
		header_t *hptr = (void *)ptr - sizeof(header_t);
		if(hptr -> magic != 12345678){
			printf("magic number wrong\n");
			m_error = E_BAD_POINTER;
			return -1;
		}
		int sizeToFree = hptr->size + sizeof(header_t) - sizeof(node_t);
		node_t* free = (node_t*)((void*)hptr);
		free -> size = sizeToFree;

		if(free < head){
			free->next = head;
			head = free;
			coalesce(head,head->next);
		}
		else{
			node_t* p = head;
			node_t* q = head -> next;
			while(free > q){
				p = q;
				q = q -> next;
			}
			p -> next = free;
			free -> next = q;
			if(coalesce(p,free) == 1)
				coalesce(p,q);
			else
				coalesce(free,q);
		}
	}
	return 0;
}

void mem_dump(){
	node_t* p = head;
	printf("free space of heap:\n");
	while(p != NULL){
		printf("address: %p    size: %d\n",p,p->size);
		p = p->next;
	}
	printf("\n\n");
}



	// if(style == M_BESTFIT){
	// 	node_t* p = NULL;
	// 	node_t* q = head;
	// 	while(q != NULL){
	// 		if(q->size > size && q->size < finded->size){
	// 			pre_finded = p;
	// 			finded = q;
	// 		}
	// 		p = q;
	// 		q = q -> next;
	// 	}
	// }
	// else if(style == M_WORSTFIT){
	// 	node_t* p = NULL;
	// 	node_t* q = head;
	// 	while(q != NULL){
	// 		if(q->size > size && q->size > finded->size){
	// 			pre_finded = p;
	// 			finded = q;
	// 		}
	// 		p = q;
	// 		q = q -> next;
	// 	}
	// }
	// else{
	// 	while(finded -> size < size){
	// 		pre_finded = finded;
	// 		finded = finded-> next;
	// 	}		
	// }