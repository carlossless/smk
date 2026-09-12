#include "pca.h"
#include "sh68f89.h"

void pca_init(void)
{
    PCACON = 0;

    P0TOPH = 0;
    P0TOPL = PCA_TOP_8BIT;
    P0CMD  = PCA_CMD_RUN;
    P1TOPH = 0;
    P1TOPL = PCA_TOP_8BIT;
    P1CMD  = PCA_CMD_RUN;
    P2TOPH = 0;
    P2TOPL = PCA_TOP_8BIT;
    P2CMD  = PCA_CMD_RUN;
    P3TOPH = 0;
    P3TOPL = PCA_TOP_8BIT;
    P3CMD  = PCA_CMD_RUN;

    P0CPL0 = 0;
    P0CPH0 = 0;
    P0CPL1 = 0;
    P0CPH1 = 0;
    P1CPL0 = 0;
    P1CPH0 = 0;
    P1CPL1 = 0;
    P1CPH1 = 0;
    P1CPL2 = 0;
    P1CPH2 = 0;
    P2CPL0 = 0;
    P2CPH0 = 0;
    P2CPL1 = 0;
    P2CPH1 = 0;
    P3CPL0 = 0;
    P3CPH0 = 0;
    P3CPL1 = 0;
    P3CPH1 = 0;

    pca_release();
}

void pca_hold(void)
{
    P0CPM0 = PCA_CPM_HOLD;
    P0CPM1 = PCA_CPM_HOLD;
    P1CPM0 = PCA_CPM_HOLD;
    P1CPM1 = PCA_CPM_HOLD;
    P1CPM2 = PCA_CPM_HOLD;
    P2CPM0 = PCA_CPM_HOLD;
    P2CPM1 = PCA_CPM_HOLD;
    P3CPM0 = PCA_CPM_HOLD;
    P3CPM1 = PCA_CPM_HOLD;
    PCACON = 0;
}

void pca_release(void)
{
    P0CF   = 0;
    P1CF   = 0;
    P2CF   = 0;
    P3CF   = 0;
    P0CPM0 = PCA_CPM_PWM;
    P0CPM1 = PCA_CPM_PWM;
    P1CPM0 = PCA_CPM_PWM;
    P1CPM1 = PCA_CPM_PWM;
    P1CPM2 = PCA_CPM_PWM;
    P2CPM0 = PCA_CPM_PWM;
    P2CPM1 = PCA_CPM_PWM;
    P3CPM0 = PCA_CPM_PWM;
    P3CPM1 = PCA_CPM_PWM;
    PCACON = PCA_CON_RUN;
}
