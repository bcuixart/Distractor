#ifndef CONFIG_H
#define CONFIG_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ERROR_CONFIG_FILE_NOT_FOUND -1
#define ERROR_CONFIG_APP_ALLOCATION -2

typedef struct
{
	int size;
	int allocated_size;
	
	char** buffer;

} AppList;

typedef struct 
{
	unsigned long int time_interval_seconds_min;
	unsigned long int time_interval_seconds_max;

	AppList app_list;

} Config;

int read_config_file(const char* filename, Config* config);

#endif