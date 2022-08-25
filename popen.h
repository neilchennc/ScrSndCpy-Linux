#ifndef POPEN_H
#define POPEN_H

typedef void (*popen_output_t)(const char *output);

int popen_run(const char *command, popen_output_t output);

#endif // #ifndef POPEN_N