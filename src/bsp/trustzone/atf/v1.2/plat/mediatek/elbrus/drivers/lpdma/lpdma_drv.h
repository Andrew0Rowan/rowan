#ifndef _LPDMA_DRV_H_
#define _LPDMA_DRV_H_

#define LPDMA_CONA          (LPDMA_BASE+0x0)		/* Tigger */
#define LPDMA_CONB          (LPDMA_BASE+0x8)		/* Mode */
#define LPDMA_CONC          (LPDMA_BASE+0x10)		/* Cascad region */
#define LPDMA_COND          (LPDMA_BASE+0x18)		/* Interleave region */
#define LPDMA_CONE          (LPDMA_BASE+0x20)		/* All channels region */
#define LPDMA_CONF          (LPDMA_BASE+0x28)		/* DMA region */
#define LPDMA_CONG          (LPDMA_BASE+0x30)		/* DMA destination address */
#define LPDMA_CONH          (LPDMA_BASE+0x38)		/* Channel one start address for swap mode */
#define LPDMA_CONI          (LPDMA_BASE+0x40)		/* Channel two start address for swap mode */
#define LPDMA_CONJ          (LPDMA_BASE+0x48)		/* Channel three start address for swap mode */
#define LPDMA_CONK          (LPDMA_BASE+0x50)		/* Address range for issue ultra DMA command */
#define LPDMA_CONL          (LPDMA_BASE+0x58)		/* Address range for issue preultra DMA command */
#define LPDMA_CONM          (LPDMA_BASE+0x60)		/* DMA migration status interrupt */
#define LPDMA_CONN          (LPDMA_BASE+0x68)		/* Memory protection region one in copy mode */
#define LPDMA_CONO          (LPDMA_BASE+0x70)		/* Memory protection region two in copy mode */
#define LPDMA_CONP          (LPDMA_BASE+0x78)		/* MMU coherence check */
#define LPDMA_CONQ          (LPDMA_BASE+0x80)		/* MISC */
#define LPDMA_CONR          (LPDMA_BASE+0x88)		/* Dummy value */

/* memory remap registers */
#define LPDMA_MMU1          (LPDMA_BASE+0x400)
#define LPDMA_MMU2          (LPDMA_BASE+0x408)
#define LPDMA_MMU3          (LPDMA_BASE+0x410)
#define LPDMA_MMU4          (LPDMA_BASE+0x418)
#define LPDMA_MMU5          (LPDMA_BASE+0x420)
#define LPDMA_MMU6          (LPDMA_BASE+0x428)
#define LPDMA_MMU7          (LPDMA_BASE+0x430)
#define LPDMA_MMU8          (LPDMA_BASE+0x438)
#define LPDMA_MMU9          (LPDMA_BASE+0x440)
#define LPDMA_MMU10         (LPDMA_BASE+0x448)
#define LPDMA_MMU11         (LPDMA_BASE+0x450)
#define LPDMA_MMU12         (LPDMA_BASE+0x458)
#define LPDMA_MMU13         (LPDMA_BASE+0x460)
#define LPDMA_MMU14         (LPDMA_BASE+0x468)
#define LPDMA_MMU15         (LPDMA_BASE+0x470)
#define LPDMA_MMU16         (LPDMA_BASE+0x478)
#define LPDMA_MMU17         (LPDMA_BASE+0x480)
#define LPDMA_MMU18         (LPDMA_BASE+0x488)
#define LPDMA_MMU19         (LPDMA_BASE+0x490)
#define LPDMA_MMU20         (LPDMA_BASE+0x498)
#define LPDMA_MMU21         (LPDMA_BASE+0x4A0)
#define LPDMA_MMU22         (LPDMA_BASE+0x4A8)
#define LPDMA_MMU23         (LPDMA_BASE+0x4B0)
#define LPDMA_MMU24         (LPDMA_BASE+0x4B8)
#define LPDMA_MMU25         (LPDMA_BASE+0x4C0)
#define LPDMA_MMU26         (LPDMA_BASE+0x4C8)
#define LPDMA_MMU27         (LPDMA_BASE+0x4D0)
#define LPDMA_MMU28         (LPDMA_BASE+0x4D8)
#define LPDMA_MMU29         (LPDMA_BASE+0x4E0)
#define LPDMA_MMU30         (LPDMA_BASE+0x4E8)
#define LPDMA_MMU31         (LPDMA_BASE+0x4F0)
#define LPDMA_MMU32         (LPDMA_BASE+0x4F8)

/* status */
#define LPDMA_INTLV         (LPDMA_BASE+0x808)
#define LPDMA_CASD          (LPDMA_BASE+0x810)
#define LPDMA_MASK          (LPDMA_BASE+0x818)
#define LPDMA_STA           (LPDMA_BASE+0x820)
#define LPDMA_DBG1ST        (LPDMA_BASE+0x830)
#define LPDMA_DBG2ND        (LPDMA_BASE+0x838)
#define LPDMA_DBG3RD        (LPDMA_BASE+0x840)
#define LPDMA_DBG4TH        (LPDMA_BASE+0x848)
#define LPDMA_DBG5TH        (LPDMA_BASE+0x850)
#define LPDMA_DBG6TH        (LPDMA_BASE+0x858)
#define LPDMA_DBG7TH        (LPDMA_BASE+0x860)
#define LPDMA_DBG8TH        (LPDMA_BASE+0x868)
#define LPDMA_DBG9TH        (LPDMA_BASE+0x870)
#define LPDMA_DBG10TH       (LPDMA_BASE+0x878)
#define LPDMA_DBG11TH       (LPDMA_BASE+0x880)
#define LPDMA_DBG12TH       (LPDMA_BASE+0x888)
#define LPDMA_DBG13TH       (LPDMA_BASE+0x890)
#define LPDMA_DBG14TH       (LPDMA_BASE+0x898)
#define LPDMA_DBG15TH       (LPDMA_BASE+0x8A0)
#define LPDMA_DBG16TH       (LPDMA_BASE+0x8A8)
#define LPDMA_DBG17TH       (LPDMA_BASE+0x8B0)
#define LPDMA_DBG18TH       (LPDMA_BASE+0x8B8)
#define LPDMA_DBG19TH       (LPDMA_BASE+0x8C0)
#define LPDMA_DBG20TH       (LPDMA_BASE+0x8C8)

#define LPDMA_START         (0x0)
#define LPDMA_END           (0x8C8)

extern uint32_t sip_lpdma_get_mode(void);
extern uint64_t sip_lpdma_write(unsigned int offset, unsigned int reg_value);
extern uint32_t sip_lpdma_read(unsigned int offset);

#endif
