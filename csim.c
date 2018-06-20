//Name: Sarthak Jain
//Andrew ID: sarthak3

//This is the C file to immitate the Cache memories. Concepts like Hit, Miss, and Evict are being counted. A list of instructions along with address are provided to
//cache from the trace file provided to us. Additionally, the instructions are of Load(for read) and Store(for write). We are using write-back strategy for write
//operation deferring to write into memory until a dirty block is evicted from cache. For eviction, we are using LRU policy. To immitate this, each line has a 
//timer variable associated which gets updated whenever a read/write happens to that line. Whenever we need to evice a line from a set, we are looking for a line
//with least timer value. This eviction happens only if there are no empty lines present in a set.

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
	char *file=(char*)(malloc(sizeof(char)*50));//Largest file name with the path was about ~20 characters
	//check if malloc din't assign memory
	if(input_param==NULL || file==NULL)
		return 0;
	int i;
	int verbose_flag=0;
	//Assign values to our cache parameters via command line
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
		else if(strcmp(argv[i],"-v")==0)
			verbose_flag=1;
	}
	//check if there is no trace file been provided in command line input
	if(file==NULL)
	{
		printf("No trace file provided \n");
		return 0;
	}
	//Initialize our cache structure using the S,E and B parameters
	cache *C_new=(cache *)(malloc(sizeof(cache)));
	C_new->all_sets=malloc(sizeof(each_set)*input_param->S);
	for(i=0;i<input_param->S;i++)
		C_new->all_sets[i].all_lines=malloc(sizeof(each_line)*input_param->E);
	
	char instruct;
	address addr;
	int size;
	long total_hits=0,total_miss=0,total_evict=0,total_dirty_evict=0,total_store_hits=0;//assuming all store hits are for total_dirty_bytes_in_cache
	//if any dirty line is evicted, we decrement total_store_hits and increment total_dirty_evict
	int hit_flag=0,evict_flag=0;
	int empty_block_pos=-1;
	int lru=0,evict_block_pos=0;
	FILE* ptr=fopen(file,"r");
	
	if(ptr==NULL)
	{
		printf("Unable to open the trace file \n");
		return 0;
	}

	//Begin reading lines from the file using fscanf. Fscanf takes pointers to the variables as input and return 0 on reaching EOF.
	//To immitate a cache memory there are certain steps to be kept in mind.
	//
	//1. From the address, extract the TAG bits, SET bits and OFFSET bits.
	//2. Our addresses are 64 bit, so tag bit count are 64-(sum of SET and OFFSET bit counts)
	//3. Based on the SET bits value, get the SET number whose Lines we need to search
	//4. Iterate over all the lines in the SET and look for any empty lines.
	//5. If we have some empty lines present then L and S can be done on that empty line
	//6. If not then either it should be a HIT or an EVICT.
	//7. If it's an HIT, update the timer value of the line accordingly since it is accessed at a new time now.
	//8. If it's an EVICT, we find a victim based on our LRU policy. We make use of timer values of each line to find the least timer value, which should signify
	//that it wasn't accessed as frequently as compared to other lines.
	//9. Then we modify the TAG, lru and valid bit of the victim block with the address we are presently reading from file.
	//10. In all these, we maintain 5 counters for HIT,MISS,EVICT,DIRTY_IN_CACHE & DIRTY_EVICT
	
	while(fscanf(ptr,"%c %llx %d",&instruct,&addr,&size)>0)
	{
		if(instruct=='L' || instruct=='S')
		{
			int lru_block_val=INT_MAX;
			address tag_val_file=addr>>(input_param->s + input_param->b);
			int numb_tag_bits_file=64-(input_param->s + input_param->b);
			int set_val=(addr<<numb_tag_bits_file)>>(numb_tag_bits_file+input_param->b);
			each_set set_ptr=C_new->all_sets[set_val];//checking which SET number does the address can lie in
			for(i=0;i<input_param->E;i++)//scanning over all lines of that SET
			{
				if(set_ptr.all_lines[i].valid_flag==1)//If a line is currently occupied
				{
					if(set_ptr.all_lines[i].tag_val==tag_val_file)//if occupied check for a HIT
					{
						if(instruct=='S' && set_ptr.all_lines[i].dirty_bit==0)//we don't increment total_store_hits if it is a same instruction repeated
						{
							set_ptr.all_lines[i].dirty_bit=1;
							total_store_hits++;
						}
						total_hits+=1;
						hit_flag=1;
						set_ptr.all_lines[i].lru_time_value=lru;
						lru++;
					}
					else if(set_ptr.all_lines[i].lru_time_value<lru_block_val)//If not a HIT, check for possible victim for EVICTION
					{
						lru_block_val=set_ptr.all_lines[i].lru_time_value;
						evict_block_pos=i;
						
					}
				}
				else if(empty_block_pos==-1)//Check if a SET has an empty LINE, so that rather than EVICT we would assign in this empty LINE
					empty_block_pos=i;
			}
			if(hit_flag!=1)//if it wasn't a HIT, then case for EMPTY LINE or EVICT
			{
				total_miss++;
				if(empty_block_pos!=-1)//Check if there was an empty LINE in the SET first before eviction
				{
					set_ptr.all_lines[empty_block_pos].tag_val=tag_val_file;
					set_ptr.all_lines[empty_block_pos].lru_time_value=lru++;
					set_ptr.all_lines[empty_block_pos].valid_flag=1;
					if(instruct=='S')//If it is a store(write), then dirty bit of that LINE is set to 1 and dirty_count++
					{
						set_ptr.all_lines[empty_block_pos].dirty_bit=1;
						total_store_hits++;
					}
					else set_ptr.all_lines[empty_block_pos].dirty_bit=0;	
				}
				else if(empty_block_pos==-1)//If all LINES are filled then EVICT victim
				{//if victim line has dirty bit set to 1 then need to decrement dirty bit count in cache by 1 and dirty_evict++
					if(set_ptr.all_lines[evict_block_pos].dirty_bit==1){
						total_dirty_evict++;
						total_store_hits--;
					}
					if(instruct=='S')//if new instruction is S then dirty bit of new line is set to 1 and dirty count++
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
			if(verbose_flag==1)//only display each instruction's outcome if verbose flag was set by giving -v in command line input
			{
				printf("OP: %c  Address: %llx  Size: %d  ",instruct,addr,size);
				if(hit_flag==1)
					printf("HIT \n");
				else if(evict_flag==1)
					printf("EVICT \n");
				else printf("MISS \n");
			}
			empty_block_pos=-1;
			hit_flag=0;
			evict_flag=0;
		}	
	}
	fclose(ptr);
	printSummary(total_hits,total_miss,total_evict,(total_store_hits*input_param->B),(total_dirty_evict*input_param->B));	
	for(i=0;i<input_param->S;i++)
		free(C_new->all_sets[i].all_lines);
	free(C_new);
	//free(file);
	free(input_param);
	return 0;
}
