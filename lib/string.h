#ifndef STRING_H
#define STRING_H

int strcmp(const char *s1, const char *s2);
void strset(char *s1, int c, int size);
int strlen(const char *s);
void itoa(int x, char str[], int d);
void ftoa(float n, char *res, int afterpoint);
int atoi(char *s);
void reverse(char *s);
void strcpy(const char *source, char *target);

#endif