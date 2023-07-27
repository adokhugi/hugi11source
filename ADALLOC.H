//adok's alloc-hÅllfunktionen

#define __ADALLOC__

#include <string.h>
#include <stdlib.h>

void mymalloc(void **pvoid,long size,signed char fill)
{
  char **pchar=(char **)pvoid;

  *pchar=(char *)malloc(size);
  if(NULL==*pchar)
  {
    printf("error: NULL pointer returned after trying to allocate memory\n");
    exit(1);
  }
  else if(fill) memset(*pchar,0,size);
}