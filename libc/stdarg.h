#pragma once

#ifdef __CWCC__
typedef struct __va_list_struct {
    char gpr;
    char fpr;
    char reserved[2];
    char *input_arg_area;
    char *reg_save_area;
} __va_list[1];
typedef __va_list va_list;

void *__va_arg(va_list v_list, unsigned char type);

#define va_start(ap, fmt) ((void)fmt, __builtin_va_info(&ap))
#define va_copy(a, b) ((a)[0] = (b)[0])
#define va_arg(ap, t) (*((t *)__va_arg(ap, _var_arg_typeof(t))))
#define va_end(ap) (void)0
#else
typedef __builtin_va_list va_list;

#define va_start __builtin_va_start
#define va_copy __builtin_va_copy
#define va_arg __builtin_va_arg
#define va_end __builtin_va_end
#endif
