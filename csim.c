#include "cachelab.h"
#include<stdlib.h>
#include<unistd.h>
#include<getopt.h>
#include "contracts.h"
#include<stdio.h>
#include<math.h>
#include<strings.h>
#include<limits.h>

typedef unsigned long long int address;

typedef struct{
	int s;
	int E;
	int b;
}cache_parameters;

typedef struct{
	int S;
	int B;
}cache_size;

typedef struct{
	address tag_val;
	int valid_flag;
	int lru_time_value;
}each_line;

typedef struct{
	each_line *all_lines;
}each_set;

typedef struct{
	each_set *all_sets;
}cache;

int main(int argc, char **argv)
{
	cache_parameters *input_param=malloc(sizeof(cache_parameters));
	char *file=(char*)(malloc(sizeof(char)*50));
	if(input_param==NULL || file==NULL)
		return 0;
	int i;
	for(i=1;i<argc;i++)
	{	
		if(strcmp(argv[i],"-s")==0)
		{
			input_param->s=atoi(argv[i+1]);
			i++;
		}
		else if(strcmp(argv[i],"-E")==0)
		{
			input_param->E=atoi(argv[i+1]);
			i++;
		}
		else if(strcmp(argv[i],"-b")==0)
		{
			input_param->b=atoi(argv[i+1]);
			i++;
		}
		else if(strcmp(argv[i],"-t")==0)
		{
			file=argv[i+1];
			i++;
		}
	}
	if(file==NULL)
	{
		printf("No trace file provided \n");
		return 0;
	}
	printf("s=%d  E=%d  b=%d  filename=%s \n",input_param->s,input_param->E,input_param->b,file);
	cache_size *cache_dimension=(cache_size*)(malloc(sizeof(cache_size)));
	cache_dimension->S=1<<input_param->s;
	cache_dimension->B=1<<input_param->b;
	printf("S=%d  B=%d \n",cache_dimension->S,cache_dimension->B);
	cache C_new;
	C_new.all_sets=malloc(sizeof(each_set)*cache_dimension->S);
	for(i=0;i<cache_dimension->S;i++)
		C_new.all_sets[i].all_lines=malloc(sizeof(each_line)*input_param->E);
	
	char instruct;
	address addr;
	int size;
	long total_hits=0,total_miss=0,total_evict=0;
	int hit_flag=0,evict_flag=0;
	int empty_block_pos=-1;
	int lru=0,evict_block_pos=0;
	FILE* ptr=fopen(file,"r");
	
	if(ptr==NULL)
	{
		printf("Unable to open the trace file \n");
		return 0;
	}
	while(fscanf(ptr,"%c %llx %d",&instruct,&addr,&size)>0)
	{
		if(instruct=='L' || instruct=='S')
		{
			int lru_block_val=INT_MAX;
			address tag_val_file=addr>>(input_param->s + input_param->b);
			int numb_tag_bits_file=64-(input_param->s + input_param->b);
			int set_val=(addr<<numb_tag_bits_file)>>(numb_tag_bits_file+input_param->b);
			each_set set_ptr=C_new.all_sets[set_val];
			for(i=0;i<input_param->E;i++)
			{
				if(set_ptr.all_lines[i].valid_flag==1)
				{
					if(set_ptr.all_lines[i].tag_val==tag_val_file)
					{
						total_hits+=1;
						hit_flag=1;
						set_ptr.all_lines[i].lru_time_value=lru;
						lru++;
					}
					else if(set_ptr.all_lines[i].lru_time_value<lru_block_val)
					{
						lru_block_val=set_ptr.all_lines[i].lru_time_value;
						evict_block_pos=i;
						
					}
				}
				else if(empty_block_pos==-1)
					empty_block_pos=i;
			}
			if(hit_flag!=1)
			{
				total_miss++;
				if(empty_block_pos!=-1)
				{
					set_ptr.all_lines[empty_block_pos].tag_val=tag_val_file;
					set_ptr.all_lines[empty_block_pos].lru_time_value=lru++;
					set_ptr.all_lines[empty_block_pos].valid_flag=1;	
				}
				else if(empty_block_pos==-1)
				{
					total_evict++;
					evict_flag=1;
					set_ptr.all_lines[evict_block_pos].tag_val=tag_val_file;
					set_ptr.all_lines[evict_block_pos].lru_time_value=lru++;
					set_ptr.all_lines[evict_block_pos].valid_flag=1;
				}
			}
			printf("OP: %c  Address: %lld  Size: %d  ",instruct,addr,size);
			if(hit_flag==1)
				printf("HIT \n");
			else if(evict_flag==1)
				printf("EVICT \n");
			else printf("MISS \n");
			empty_block_pos=-1;
			hit_flag=0;
			evict_flag=0;
		}	
	}
	fclose(ptr);
	printf("Hits=%lu Miss=%lu Evict=%lu \n",total_hits,total_miss,total_evict);
	//printSumamry(total_hits,total_miss,total_evict);
	return 0;
}
