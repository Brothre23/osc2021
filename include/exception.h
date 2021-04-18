#ifndef EXCEPTION_H
#define EXCEPTION_H

struct trapframe {
    unsigned long x[31];    // general register from x0 ~ x30
    unsigned long sp_el0;
    unsigned long elr_el1;
    unsigned long spsr_el1;
};

void enable_irq();
void disable_irq();

#endif