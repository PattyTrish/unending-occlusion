#ifndef RUNTIME_GECKO_SETJMP_H
#define RUNTIME_GECKO_SETJMP_H

/* Same layout as src/msl/setjmp.h */
typedef struct jmp_buf {
    unsigned long pc;       /*   0: saved PC		*/
    unsigned long cr;       /*   4: saved CR		*/
    unsigned long sp;       /*   8: saved SP		*/
    unsigned long rtoc;     /*  12: saved RTOC		*/
    unsigned long reserved; /*  16: not used		*/
    unsigned long gprs[19]; /*  20: saved R13-R31	*/
    struct {
        double fp;          /* saved FPRn as double	*/
        float ps[2];        /* saved FPRn ps0/ps1	*/
    } fprs[18];             /*  96: saved FP14-FP31	*/
    double fpscr;           /* 384: saved FPSCR		*/
    unsigned long pad[2];   /* 392: pad to 100 words	*/
} jmp_buf;

void __longjmp(register jmp_buf* env, int val);
int __setjmp(register jmp_buf* env);

#define setjmp(env) __setjmp(&(env))
#define longjmp(env, val) __longjmp(&(env), val)

#endif
