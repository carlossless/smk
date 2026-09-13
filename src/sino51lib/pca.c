#include "pca.h"
#include "pca_hw.h"

#if !PCA_HW_PRESENT
#    error "pca.c: this part has no PCA block, see its pca_hw.h"
#endif

#define PCA_CMD_RUN  0x02u
#define PCA_CPM_PWM  0xD8u
#define PCA_CPM_HOLD 0xD0u

#define PCA_UNIT_START(cf, cmd, topl, toph) \
    toph = 0;                               \
    topl = PCA_TOP_8BIT;                    \
    cmd  = PCA_CMD_RUN;

#define PCA_CHANNEL_CLEAR(cpm, cpl, cph) \
    cpl = 0;                             \
    cph = 0;

#define PCA_CHANNEL_HOLD(cpm, cpl, cph)            cpm = PCA_CPM_HOLD;
#define PCA_CHANNEL_PWM(cpm, cpl, cph)             cpm = PCA_CPM_PWM;
#define PCA_UNIT_CONFIG_CLEAR(cf, cmd, topl, toph) cf = 0;

void pca_init(void)
{
    PCACON = 0;

    PCA_FOREACH_UNIT(PCA_UNIT_START)
    PCA_FOREACH_CHANNEL(PCA_CHANNEL_CLEAR)

    pca_release();
}

void pca_hold(void)
{
    PCA_FOREACH_CHANNEL(PCA_CHANNEL_HOLD)
    PCACON = 0;
}

void pca_release(void)
{
    PCA_FOREACH_UNIT(PCA_UNIT_CONFIG_CLEAR)
    PCA_FOREACH_CHANNEL(PCA_CHANNEL_PWM)
    PCACON = PCA_CON_RUN;
}
