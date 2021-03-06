#include "assert.h"
#include "backtrace.h"
#include "malloc.h"
#include "nameless.h"
#include "rand.h"
#include <stdint.h>
#include "strings.h"
#include "uart.h"

void main(void);

static void test_backtrace_simple()
{
    struct frame f[2];
    int frames_filled = backtrace(f, 2);

    printf("%d\n", frames_filled);

    assert(frames_filled == 2);
    printf(f[0].name);
   assert(strcmp(f[0].name, "test_backtrace_simple") == 0);
    printf("%p\n", f[0].resume_addr);
    printf("%d\n", f[0].resume_offset);
    assert(f[0].resume_addr == (char *)test_backtrace_simple + f[0].resume_offset);
  assert(strcmp(f[1].name, "main") == 0);
    assert(f[1].resume_addr == (char *)main + f[1].resume_offset);
    print_frames(f, frames_filled);
}

//static int recursion_fun(int n)
//{
//    if (n == 0)
//        return mystery();   // look in nameless.c
//    else
//        return 1 + recursion_fun(n-1);
//}
//
//static int test_backtrace_complex(int n)
//{
//    return recursion_fun(n);
//}

static void test_heap_simple(void)
{

    char *s = malloc(6);
    memcpy(s, "hello", 6);
    char* t = malloc(6);
    memcpy(t, "World", 6);
    assert(strcmp(s, "hello") == 0);
    printf("%s", s);
    free(s);

    s = malloc(6);
    memcpy(s, "hello", 6);
    s = realloc(s, 12);
    strlcat(s, " world", 12);
    assert(strcmp(s, "hello world") == 0);
    printf("%s\n", s);
    free(s);
}


// array of dynamically-allocated strings, each
// string filled with repeated char, e.g. "A" , "BB" , "CCC"
// Examine each string, verify expected contents intact.
static void test_heap_multiple(void)
{
    int n = 26;
    char *arr[n];

    for (int i = 0; i < n; i++) {
        int num_repeats = i + 1;
        char *ptr = malloc(num_repeats + 1);
        printf("This is the ptr: %p\n", ptr);
        assert(ptr != NULL);
        memset(ptr, 'A' - 1 + num_repeats, num_repeats);
        ptr[num_repeats] = '\0';
        arr[i] = ptr;
    }
    for (int i = n-1; i >= 0; i--) {
        int len = strlen(arr[i]);
        char first = arr[i][0], last = arr[i][len -1];
        assert(first == 'A' - 1 + len);
        assert(first == last);
        free(arr[i]);
    }
}

#define max(x, y) ((x) > (y) ? (x) : (y))

static void test_heap_recycle(int max_iterations)
{
    extern int __bss_start__;
    void *heap_low = &__bss_start__;
    void *heap_high = NULL;

    int i;
    void *p = malloc(1);

    for (i = 0; i < max_iterations; i++) {
        int size = rand() % 1024;
        void *q = malloc(size);
        p = realloc(p, size);
        heap_high = max(heap_high, max(p, q));
        free(q);
    }
    free(p);

    printf("\nCompleted %d iterations. Heap grew to peak size of %d bytes.\n", i, (char *)heap_high - (char *)heap_low);
}


static void test_heap_redzones(void)
{
    // DO NOT ATTEMPT THIS TEST unless your heap has red zone protection!
    char *ptr;

    ptr = malloc(9);
    memset(ptr, 'a', 9);
    free(ptr); // ptr is OK

    ptr = malloc(5);
    ptr[-1] = 0x45; // write before payload
    free(ptr);      // ptr is NOT ok

    ptr = malloc(12);
    ptr[13] = 0x45; // write after payload
    free(ptr);      // ptr is NOT ok
}
#include "uart.h"
#include "printf.h"
#include "backtrace.h"
#include "timer.h"
#include "nameless.h"

static int recursion_fun(int n)
{
    if (n == 0)
        return mystery();   // look in nameless.c
    else
        return 1 + recursion_fun(n-1);
}

static int test_backtrace_complex(int n)
{
    return recursion_fun(n);
}

void main()
{
    uart_init();
    timer_delay(1);
    
    
    printf("--------------------Starting test_backtrace_complex--------------------\n");
    test_backtrace_complex(7);
    printf("--------------------Finished test_backtrace_complex--------------------\n");
    printf("%c", 4); // End of transmission.
}


//void main(void)
//{
////    uart_init();
////
////    test_backtrace_simple();
////    printf("\n");
////    test_backtrace_simple(); // Again so you can see the main offset change!
////    printf("\n");
////        test_backtrace_complex(7);  // Slightly tricky backtrace.
////
////       test_heap_simple();
////    test_heap_multiple();
//      test_heap_recycle(5); // increase the number for stress test
//   // test_heap_redzones();
//}

