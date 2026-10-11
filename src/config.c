#include "config.h"

#define MAX_LINE_LENGTH 256

#define TIME_INTERVAL_MIN_DEFAULT 600
#define TIME_INTERVAL_MAX_DEFAULT 1800

int add_application(AppList* app_list, const char* app_name) 
{
	if (app_list->size >= app_list->allocated_size) 
	{
		int new_allocated_size = (app_list->allocated_size == 0) ? 1 : app_list->allocated_size * 2;

		char** new_buffer = realloc(app_list->buffer, new_allocated_size * sizeof(char*));
		if (!new_buffer) 
		{
			return ERROR_CONFIG_APP_ALLOCATION;
		}
		app_list->buffer = new_buffer;
		app_list->allocated_size = new_allocated_size;
	}

	app_list->buffer[app_list->size] = strdup(app_name);
	if (!app_list->buffer[app_list->size]) 
	{
		return ERROR_CONFIG_APP_ALLOCATION;
	}
	app_list->size++;

	return 0;
}

int interpret_config_line(const char* line, int line_length, Config* config) 
{
	if (line_length <= 0 || line[0] == '#' || line[0] == '\n') return 0;

	if (sscanf(line, "set_time_interval_seconds %lu %lu", 
		&config->time_interval_seconds_min, 
		&config->time_interval_seconds_max) == 2) return 0;

	char app_buffer[MAX_LINE_LENGTH];
	if (sscanf(line, "add_application %255[^\n]", app_buffer) == 1) 
	{
		int result = add_application(&config->app_list, app_buffer);

		if (result != 0) return result;
	}

	return 0;
}

int read_config_file(const char* filename, Config* config) 
{
	FILE* file = fopen(filename, "r");
	if (!file) return ERROR_CONFIG_FILE_NOT_FOUND;

	config->time_interval_seconds_min = 0;
	config->time_interval_seconds_max = 0;
	config->app_list.size = 0;
	config->app_list.allocated_size = 0;
	config->app_list.buffer = NULL;

	char line[MAX_LINE_LENGTH];
	while (fgets(line, sizeof(line), file)) 
	{
        int result = interpret_config_line(line, strlen(line), config);
        if (result != 0) 
		{
            fclose(file);
            return result;
        }
    }

	// Check if the config time interval is valid
	if (config->time_interval_seconds_max == 0 
		|| config->time_interval_seconds_min >= config->time_interval_seconds_max) 
	{
		fprintf(stderr, "Time interval not set or invalid. Default values will be used: %d, %d.\n", 
			TIME_INTERVAL_MIN_DEFAULT, TIME_INTERVAL_MAX_DEFAULT);
		config->time_interval_seconds_min = TIME_INTERVAL_MIN_DEFAULT;
		config->time_interval_seconds_max = TIME_INTERVAL_MAX_DEFAULT;
	}

	fclose(file);
	return 0;
}