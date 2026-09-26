#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>

struct Test {
    const char* source;
    int expected;
};
static struct Test tests[] = {
    {"int main(){return 3+5;}", 8},
    {"int main(){int x; x=7; return x;}", 7},
    {"int add(int x,int y){return x+y;} int main(){return add(3,4);}", 7},
    {"int main(){int x; x=0; if(1) x=5; else x=8; return x;}", 5},
    {"int main(){int x; x=0; while(x<8){x=x+1; if(x==3) continue; if(x==6) break;} return x;}", 6},
    {"int main(){int sum; int i; sum=0; for(i=0;i<10;i=i+1) sum=sum+i; return sum;}", 45},
    {"int main(){int i; i=0; while(i<4) i=i+1; return i;}", 4},
    {"int main(){int x; x=0; if(0 && (x=5)) return 1; return x;}", 0},
    {"int main(){int x; x=0; if(1 || (x=5)) return x; return 9;}", 0},
    {"int main(){int x; int *p; x=12; p=&x; return *p;}", 12},
    {"int main(){int a[3]; a[0]=4; a[1]=9; a[2]=3; return a[0]+a[1]+a[2];}", 16},
    {"int main(){int a[3]; int *p; a[0]=3; a[1]=11; p=a; return *(p+1);}", 11},
    {"int main(){int a[3]; int *p; p=a; return sizeof(a);}", 12},
    {"int main(){int a[3]; int *p; a[0]=2; a[1]=8; a[2]=14; p=a; return (p+2)-p;}", 2},
    {"int main(){int a[3]; int *p; a[0]=2; a[1]=7; p=a; return *(p+1);}", 7},
    {"int main(){char a[3]; a[0]='A'; a[1]='B'; return a[0]+a[1];}", 131},
    {"int counter; int main(){counter=42; return counter;}", 42},
    {"char global_char; int main(){global_char='Z'; return global_char;}", 90},
    {"int main(){char a[4]; return sizeof(a);}", 4},
    {"int main(){int *p; return sizeof(p);}", 8},
    {"int main(){return sizeof(char);}", 1},
    {"int main(){return sizeof(int);}", 4},
    {"int main(){return 3;} // trailing line comment\n", 3},
    {"/* block comment */ int main(){return 6;}", 6},
    {"int main(){return \"hi\\n\"[2];}", 10},
};
int main(void) {
    int count = sizeof(tests) / sizeof(tests[0]);
    for (int i = 0; i < count; i++) {
        FILE* f = fopen("test_input.c", "w");
        if (!f) {
            perror("test_input.c");
            return 1;
        }
        fputs(tests[i].source, f);
        fclose(f);
        int status = system("./main test_input.c > foo.s && gcc -o foo foo.s && ./foo");
        int actual = status == -1 || !WIFEXITED(status) ? -1 : WEXITSTATUS(status);
        if (actual != tests[i].expected) {
            fprintf(stderr, "FAIL %d: expected %d got %d\n  %s\n", i + 1, tests[i].expected, actual, tests[i].source);
            return 1;
        }
        printf("ok %d: %d\n", i + 1, actual);
    }
    FILE* f = fopen("test_input.c", "w");
    fputs("int main(){break;}", f);
    fclose(f);
    if (system("./main test_input.c > /dev/null 2>&1") == 0) {
        fprintf(stderr, "invalid break should fail\n");
        return 1;
    }
    f = fopen("test_input.c", "w");
    fputs("int main(){return unknown;}", f);
    fclose(f);
    if (system("./main test_input.c > /dev/null 2>&1") == 0) {
        fprintf(stderr, "undeclared identifier should fail\n");
        return 1;
    }
    puts("all tests passed");
    return 0;
}
