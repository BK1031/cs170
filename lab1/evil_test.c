/*
   James Calloway
   Matthew Allen
   CS 170
   February 24, 2000

   Program that writes to the screen a certain number
   of times. This program wont work because of the call to
   sbrk(), which you'll implement in lab2.

*/

#include <stdio.h>

main()
{
    int i;
    char *buf;

    buf=(char *)malloc(30*sizeof(char));

    for (i=0;i<5;i++){

        sprintf(buf,"#%d: cs170 is a great class!!\n",i);
        write(1,buf,strlen(buf));
    }

}