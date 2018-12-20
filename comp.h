#ifndef COMP_H
#define COMP_H
void comp_config(void);

// feedback C
#define PA0_SELECT COMP_CSR_COMP1INSEL_1 | COMP_CSR_COMP1INSEL_2 | COMP_CSR_COMP1EN
// feedback a
#define PA4_SELECT COMP_CSR_COMP1INSEL_2 | COMP_CSR_COMP1EN
// feedback b
#define PA5_SELECT COMP_CSR_COMP1INSEL_0 | COMP_CSR_COMP1INSEL_2 | COMP_CSR_COMP1EN
#define comp1_change_input(x) COMP->CSR=(x)

#endif
