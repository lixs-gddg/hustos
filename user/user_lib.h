/*
 * header file to be used by applications.
 */

int printu(const char *s, ...);
int exit(int code);
void* naive_malloc();
void naive_free(void* va);
int fork();
void yield();
int sem_new(long init_value);
int sem_P(long semnum);
int sem_V(long semnum);