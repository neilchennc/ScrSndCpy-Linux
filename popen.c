#include "popen.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int popen_run(const char *command, popen_output_t output)
{
    char *redirect_command;
    char buffer[256];
    FILE *fp;
    int error_code;

    redirect_command = malloc(strlen(command) + 8);
    sprintf(redirect_command, "%s 2>&1", command);

    fp = popen(redirect_command, "r");
    while (fgets(buffer, sizeof(buffer) - 1, fp))
    {
        // printf("popen_run: %s\n", buffer);
        if (output)
        {
            output(buffer);
        }
    }
    error_code = pclose(fp);

    free(redirect_command);

    return error_code;
}