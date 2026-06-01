#include<stdio.h>
#include<string.h>

const char g_buff[]="Hello World from ubuntu 24.04's gcc 13.";
int main(){
    char buff[1024];
    memcpy(buff,g_buff,sizeof(g_buff));
    printf("%s\n",buff);
    return 0;
}