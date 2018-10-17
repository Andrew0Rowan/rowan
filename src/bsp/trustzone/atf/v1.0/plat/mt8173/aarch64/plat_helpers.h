#ifndef PLAT_HELPERS_H
#define PLAT_HELPERS_H

/* L2ACTLR definitions */
#define L2ACTLR_ENABLE_UNIQUECLEAN	(1 << 14)
#define L2ACTLR_DISABLE_CLEAN_PUSH	(1 << 3)

/* CPUACTLR definitions */
#define CPUACTLR_DTAH		(1 << 24)

unsigned long read_l2actlr(void);
unsigned long read_l2ectlr(void);

void write_l2actlr(unsigned long);
void write_l2ectlr(unsigned long);

unsigned long read_cpuactlr(void);
void write_cpuactlr(unsigned long);

unsigned long read_cpuectlr(void);
void write_cpuectlr(unsigned long);


#endif
