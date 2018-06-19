#include "cachelab.h"
#include<stdlib.h>
#include<unistd.h>
#include<getopt.h>
#include "contracts.h"
#include<stdio.h>
#include<math.h>
#include<strings.h>

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
	printf("s=%d  E=%d  b=%d  filename=%s \n",input_param->s,input_param->E,input_param->b,file);
	return 0;
}
