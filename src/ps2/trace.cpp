#include <cstdio>
#include <cstdarg>
#include <tamtypes.h>

#if defined(__mips__)
void Kputc(char c)
{
    while (_lw(0x1000f130) & 0x8000) {
        __asm__("nop\nnop\nnop\n");
    }

    *((char*)0x1000f180) = c;
}

#else
void Kputc(char c)
{
    putchar(c);
}

#endif

void Kputs(char* s)
{
    while (*s != 0) {
        Kputc(*s++);
    }
}

void KprintHexnum(unsigned int n)
{
    char chars[17] = "0123456789abcdef";
    int shift = 28;
    while (shift >= 0) {
        Kputc(chars[(n >> shift) & 0x0F]);
        shift -= 4;
    }
}

void KprintDecnum(unsigned int n)
{
    char chars[17] = "0123456789abcdef";
    int divisor = 1000000;
    int val = n;
    if (val < 0){
        val = -val;
        Kputc('-');
    }
    bool printing = false;
    while (divisor > 0) {
        int digit = val / divisor;
        if (digit != 0 || divisor == 1){
            printing = true;
        }
        if (printing){
            Kputc(chars[digit & 0x0F]);
        }
        val = val % divisor;
        divisor /= 10;
    }
}

int Kprintf(const char* fmt, va_list args)
{
    while (*fmt) {
        if (*fmt != '%') {
            Kputc(*fmt++);
        } else {
            if (*++fmt == '%') {
                // %% is printed as %
                Kputc(*fmt++);
            } else {
                int c = *fmt++;
                switch (c) {
                case 'x':
                case 'p':
                {
                    void *pval = va_arg(args, void *);
                    KprintHexnum((size_t)(pval));
                } break;
                case 'd':
                {
                    unsigned int val = va_arg(args, unsigned int);
                    KprintDecnum(val);
                } break;
                case 's':
                {
                    char* val = va_arg(args, char*);
                    Kputs(val);
                } break;
                }
            }
        }
    }
    return 0;
}

#if 0
void trace(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    Kprintf(fmt, args);
    va_end(args);
}

void traceln(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    Kprintf(fmt, args);
    va_end(args);
    Kputc('\n');
}
#else
void trace(const char* fmt, ...) {}
void traceln(const char* fmt, ...) {}
#endif
