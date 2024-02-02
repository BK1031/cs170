/*
   James Calloway
   Matthew Allen
   CS 170
   February 24, 2000

   Program that writes to the screen a certain number
   of times.

*/



main()
{
    int i;
    char buf[30];

    for (i=0;i<5;i++){
        sprintf(buf,"#%d: cs170 is a great class!!\n",i);
        write(1,buf,strlen(buf));
    }

}
