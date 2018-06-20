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
	int S;
	int E;
	int b;
	int B;
}cache_parameters;

typedef struct{
	int S;
	int B;
}cache_size;

typedef struct{
	address tag_val;
	int valid_flag;
	int dirty_bit;
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
			input_param->S=1<<(input_param->s);
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
			input_param->B=1<<(input_param->b);
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
	printf("s=%d  S=%d  E=%d  b=%d  B=%d  filename=%s \n",input_param->s,input_param->S,input_param->E,input_param->b,input_param->B,file);
	cache C_new;
	C_new.all_sets=malloc(sizeof(each_set)*input_param->S);
	for(i=0;i<input_param->S;i++)
		C_new.all_sets[i].all_lines=malloc(sizeof(each_line)*input_param->E);
	
	char instruct;
	address addr;
	int size;
	long total_hits=0,total_miss=0,total_evict=0,total_dirty_evict=0,total_store_hits=0;
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
						if(instruct=='S' && set_ptr.all_lines[i].dirty_bit==0)
						{
							set_ptr.all_lines[i].dirty_bit=1;
							total_store_hits++;
						}
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
					if(instruct=='S')
					{
						set_ptr.all_lines[empty_block_pos].dirty_bit=1;
						total_store_hits++;
					}
					else set_ptr.all_lines[empty_block_pos].dirty_bit=0;	
				}
				else if(empty_block_pos==-1)
				{
					if(set_ptr.all_lines[evict_block_pos].dirty_bit==1){
						total_dirty_evict++;
						total_store_hits--;
					}
					if(instruct=='S')
					{
						total_store_hits++;
						set_ptr.all_lines[evict_block_pos].dirty_bit=1;
					}
					else set_ptr.all_lines[evict_block_pos].dirty_bit=0;
					total_evict++;
					evict_flag=1;
					set_ptr.all_lines[evict_block_pos].tag_val=tag_val_file;
					set_ptr.all_lines[evict_block_pos].lru_time_value=lru++;
					set_ptr.all_lines[evict_block_pos].valid_flag=1;
				}
			}
			printf("OP: %c  Address: %llx  Size: %d  ",instruct,addr,size);
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
	printSummary(total_hits,total_miss,total_evict,(total_store_hits*input_param->B),(total_dirty_evict*input_param->B));	
	//for(i=0;i<input_param->S;i++)
	//	free(C_new.all_sets[i].all_lines);
	//free(C_new.all_sets);
	//free(file);
	//free(input_param);
	return 0;
}
