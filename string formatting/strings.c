                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                    #include "strings.h"

void *memset(void *s, int c, size_t n)
{ //writes n bytes of c (which is converted to char) to the memory pointed to by s.
    char* ptr = s;
    for(int i =0; i < n; i++){
        ptr[i] = (char)c;
    }
    return s;
}

void *memcpy(void *dst, const void *src, size_t n)
{
    volatile char* str1 = dst; //copies n bytes of memry to dst from src, returns dst.
    char* str2 = (char*)src;
    for(volatile int i =0; i < n; i++){ //iterates through to copy character by character
        str1[i] = str2[i];
    }
    return dst;
}

int strlen(const char *s)
{
    /* Implementation a gift to you from lab3 */
    int i;
    for (i = 0; s[i] != '\0'; i++) ;
    return i;
}

int strcmp(const char *s1, const char *s2)
{
    int condition = strlen(s2);
    if(strlen(s1) > strlen(s2)) return 1; //if two strings are not same length, return 1 or -1 depending on which is bigger.
    if(strlen(s2) > strlen(s1)) return -1;
    for(int i =0; i < condition; i++){
        int num1 =  s1[i] - '0'; //conversion to character
        int num2 = s2[i] - '0';
        if(num1 > num2) return 1; //if any character is different, return 1 or -1 depending on which character is greater.
        if(num2> num1) return -1;
    }
    
    return 0;
}

int strlcat(char *dst, const char *src, int maxsize) //string concatentation bounded by size
{
    int length = strlen(dst) + strlen(src); //length of two strings together
    int orig_dst_length = strlen(dst);
    int total = maxsize- orig_dst_length -1; //total number of characters you can concatenate, taking into account max size, original length of dst, and the null terminator to be added later.
    if(total > strlen(src)) total = strlen(src); //if there is extra room, total should be the length of src so you add all of its characters and don't do extra iterations
    int i;
    for(i =0; i < total; i++){
        dst[orig_dst_length+i] = src[i]; //adds a character from src to next available spot in dst.
    }
    dst[orig_dst_length +i] = '\0'; //null termination
    return  length;
}

unsigned int strtou(const char *str, char **endptr, int base) //this method converts a string to an unsigned int in accordance with the base the string is in.
{
    int num = 0;
    int n;
    int i;
    for(i = 0; i < strlen(str); i++){
        
        if(base == 16 && (str[i] >= 65 && str[i] <= 70)){
            n = str[i] - '7';
        }
           if(base == 16 && (str[i] >= 97 && str[i] <= 102)){
            n = str[i] - 'W';
        }
        else{
            if(!(str[i] >= '0' && str[i] <= '9')){ //check if reaches an invalid digit
                if(endptr != NULL) *endptr = (char*) str+i; //sets endptr to keep track of invalid digit
                int fix = strlen(str) - i; //fixes conversion
                for(int i =0; i < fix; i++){
                    num = num/base;
                }
                return num; //stops conversion by returning
            }
            n = str[i] - '0'; //converts to int
        }
        for(int j = strlen(str)-i-1; j >0; j--){ //essentially a power function
            n*= base;
        }
        num +=n;
    }
    *endptr = (char*)str+i;
    return num;
}


