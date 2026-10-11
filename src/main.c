#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include "config.h"

void usage(void) 
{
	printf("Usage: .\\distractor.exe [<config_file>, default: .\\config.txt]\n");
}

int app_loop(Config* config)
{
	while(1)
	{
		unsigned long int wait_seconds = config->time_interval_seconds_min + 
			rand() % (config->time_interval_seconds_max - config->time_interval_seconds_min + 1);

		Sleep(wait_seconds * 1000);		

		STARTUPINFOA si = {0};
		si.cb = sizeof(si);
		PROCESS_INFORMATION pi = {0};

		int process_index = rand() % config->app_list.size;
		if (!CreateProcessA(config->app_list.buffer[process_index], NULL, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) 
		{
			DWORD error_code = GetLastError();

			fprintf(stderr, "Error: Could not create process for application: %s\n", config->app_list.buffer[process_index]);
			fprintf(stderr, "%lu\n", error_code);

			continue;
		}

		WaitForSingleObject(pi.hProcess, INFINITE);

		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
	}

	return 0;
}

int main(int argc, char *argv[]) 
{
	srand((unsigned int)GetTickCount());

	if (argc > 2) 
	{
		usage();
		return 1;
	}

	char* config_file = "config.txt";
	if (argc == 2) config_file = argv[1];

	Config config = {0};
	int read_result = read_config_file(config_file, &config);

	if (read_result == ERROR_CONFIG_FILE_NOT_FOUND) 
	{
		fprintf(stderr, "Error: Could not find config file: %s\n", config_file);
		return 1;
	}

	if (config.app_list.size == 0) 
	{
		fprintf(stderr, "Error: No applications specified in config file: %s\n", config_file);
		return 1;
	}

	return app_loop(&config);
}