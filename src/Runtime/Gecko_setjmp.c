#include "Gecko_setjmp.h"

#define SAVE_FPR(n)                          \
    stfd fp##n, env->fprs[n - 14].fp;        \
    la r4, env->fprs[n - 14].ps;             \
    psq_stx fp##n, 0, r4, 0, 0

#define RESTORE_FPR(n)                       \
    lfd fp##n, env->fprs[n - 14].fp;         \
    la r7, env->fprs[n - 14].ps;             \
    psq_lx fp##n, 0, r7, 0, 0

asm int __setjmp(register jmp_buf* env) {
#ifdef __MWERKS__ // clang-format off
    nofralloc
    mflr    r5
    mfcr    r6
    stw     r5, env->pc
    stw     r6, env->cr
    stw     SP, env->sp
    stw     RTOC, env->rtoc
    stmw    r13, env->gprs
    mffs    fp0
    SAVE_FPR(14)
    SAVE_FPR(15)
    SAVE_FPR(16)
    SAVE_FPR(17)
    SAVE_FPR(18)
    SAVE_FPR(19)
    SAVE_FPR(20)
    SAVE_FPR(21)
    SAVE_FPR(22)
    SAVE_FPR(23)
    SAVE_FPR(24)
    SAVE_FPR(25)
    SAVE_FPR(26)
    SAVE_FPR(27)
    SAVE_FPR(28)
    SAVE_FPR(29)
    SAVE_FPR(30)
    SAVE_FPR(31)
    stfd    fp0, env->fpscr
    li      r3, 0
    blr
#endif // clang-format on
}

asm void __longjmp(register jmp_buf* env, register int val) {
#ifdef __MWERKS__ // clang-format off
    nofralloc
    lwz     r5, env->pc
    lwz     r6, env->cr
    mtlr    r5
    mtcrf   255, r6
    lwz     SP, env->sp
    lwz     RTOC, env->rtoc
    lmw     r13, env->gprs
    RESTORE_FPR(14)
    RESTORE_FPR(15)
    RESTORE_FPR(16)
    RESTORE_FPR(17)
    RESTORE_FPR(18)
    RESTORE_FPR(19)
    RESTORE_FPR(20)
    RESTORE_FPR(21)
    RESTORE_FPR(22)
    RESTORE_FPR(23)
    RESTORE_FPR(24)
    RESTORE_FPR(25)
    RESTORE_FPR(26)
    RESTORE_FPR(27)
    RESTORE_FPR(28)
    RESTORE_FPR(29)
    RESTORE_FPR(30)
    RESTORE_FPR(31)
    lfd     fp0, env->fpscr
    cmpwi   val, 0
    mr      r3, val
    mtfsf   255, fp0
    bnelr
    li      r3, 1
    blr
#endif // clang-format on
}
