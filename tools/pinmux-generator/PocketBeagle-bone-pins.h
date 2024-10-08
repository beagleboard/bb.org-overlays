

#define gpio_P1_02 &gpio2 23
#define gpio_P1_04 &gpio2 25
#define gpio_P1_06 &gpio0 5
#define gpio_P1_08 &gpio0 2
#define gpio_P1_10 &gpio0 3
#define gpio_P1_12 &gpio0 4
#define gpio_P1_20 &gpio0 20
#define gpio_P1_26 &gpio0 12
#define gpio_P1_28 &gpio0 13
#define gpio_P1_29 &gpio3 21
#define gpio_P1_30 &gpio1 11
#define gpio_P1_31 &gpio3 18
#define gpio_P1_32 &gpio1 10
#define gpio_P1_33 &gpio3 15
#define gpio_P1_34 &gpio0 26
#define gpio_P1_35 &gpio2 24
#define gpio_P1_36 &gpio3 14
#define gpio_P2_01 &gpio1 18
#define gpio_P2_02 &gpio1 27
#define gpio_P2_03 &gpio0 23
#define gpio_P2_04 &gpio1 26
#define gpio_P2_05 &gpio0 30
#define gpio_P2_06 &gpio1 25
#define gpio_P2_07 &gpio0 31
#define gpio_P2_08 &gpio1 28
#define gpio_P2_09 &gpio0 15
#define gpio_P2_10 &gpio1 20
#define gpio_P2_11 &gpio0 14
#define gpio_P2_17 &gpio2 1
#define gpio_P2_18 &gpio1 15
#define gpio_P2_19 &gpio0 27
#define gpio_P2_20 &gpio2 0
#define gpio_P2_22 &gpio1 14
#define gpio_P2_24 &gpio1 12
#define gpio_P2_25 &gpio1 9
#define gpio_P2_27 &gpio1 8
#define gpio_P2_28 &gpio3 20
#define gpio_P2_29 &gpio0 7
#define gpio_P2_30 &gpio3 17
#define gpio_P2_31 &gpio0 19
#define gpio_P2_32 &gpio3 16
#define gpio_P2_33 &gpio1 13
#define gpio_P2_34 &gpio3 19
#define gpio_P2_35 &gpio2 22

#define P1_02(mode) AM33XX_IOPAD(0x08e4, mode)  /* R5: lcd_hsync */
#define P1_04(mode) AM33XX_IOPAD(0x08ec, mode)  /* R6: lcd_ac_bias_en */
#define P1_06(mode) AM33XX_IOPAD(0x095c, mode)  /* A16: spi0_cs0 */
#define P1_08(mode) AM33XX_IOPAD(0x0950, mode)  /* A17: spi0_sclk */
#define P1_10(mode) AM33XX_IOPAD(0x0954, mode)  /* B17: spi0_d0 */
#define P1_12(mode) AM33XX_IOPAD(0x0958, mode)  /* B16: spi0_d1 */
#define P1_20(mode) AM33XX_IOPAD(0x09b4, mode)  /* D14: xdma_event_intr1 */
#define P1_26(mode) AM33XX_IOPAD(0x0978, mode)  /* D18: uart1_ctsn */
#define P1_28(mode) AM33XX_IOPAD(0x097c, mode)  /* D17: uart1_rtsn */
#define P1_29(mode) AM33XX_IOPAD(0x09ac, mode)  /* A14: mcasp0_ahclkx */
#define P1_30(mode) AM33XX_IOPAD(0x0974, mode)  /* E16: uart0_txd */
#define P1_31(mode) AM33XX_IOPAD(0x09a0, mode)  /* B12: mcasp0_aclkr */
#define P1_32(mode) AM33XX_IOPAD(0x0970, mode)  /* E15: uart0_rxd */
#define P1_33(mode) AM33XX_IOPAD(0x0994, mode)  /* B13: mcasp0_fsx */
#define P1_34(mode) AM33XX_IOPAD(0x0828, mode)  /* T11: gpmc_ad10 */
#define P1_35(mode) AM33XX_IOPAD(0x08e8, mode)  /* V5: lcd_pclk */
#define P1_36(mode) AM33XX_IOPAD(0x0990, mode)  /* A13: mcasp0_aclkx */
#define P2_01(mode) AM33XX_IOPAD(0x0848, mode)  /* U14: gpmc_a2 */
#define P2_02(mode) AM33XX_IOPAD(0x086c, mode)  /* V17: gpmc_a11 */
#define P2_03(mode) AM33XX_IOPAD(0x0824, mode)  /* T10: gpmc_ad9 */
#define P2_04(mode) AM33XX_IOPAD(0x0868, mode)  /* T16: gpmc_a10 */
#define P2_05(mode) AM33XX_IOPAD(0x0870, mode)  /* T17: gpmc_wait0 */
#define P2_06(mode) AM33XX_IOPAD(0x0864, mode)  /* U16: gpmc_a9 */
#define P2_07(mode) AM33XX_IOPAD(0x0874, mode)  /* U17: gpmc_wpn */
#define P2_08(mode) AM33XX_IOPAD(0x0878, mode)  /* U18: gpmc_be1n */
#define P2_09(mode) AM33XX_IOPAD(0x0984, mode)  /* D15: uart1_txd */
#define P2_10(mode) AM33XX_IOPAD(0x0850, mode)  /* R14: gpmc_a4 */
#define P2_11(mode) AM33XX_IOPAD(0x0980, mode)  /* D16: uart1_rxd */
#define P2_17(mode) AM33XX_IOPAD(0x088c, mode)  /* V12: gpmc_clk */
#define P2_18(mode) AM33XX_IOPAD(0x083c, mode)  /* U13: gpmc_ad15 */
#define P2_19(mode) AM33XX_IOPAD(0x082c, mode)  /* U12: gpmc_ad11 */
#define P2_20(mode) AM33XX_IOPAD(0x0888, mode)  /* T13: gpmc_csn3 */
#define P2_22(mode) AM33XX_IOPAD(0x0838, mode)  /* V13: gpmc_ad14 */
#define P2_24(mode) AM33XX_IOPAD(0x0830, mode)  /* T12: gpmc_ad12 */
#define P2_25(mode) AM33XX_IOPAD(0x096c, mode)  /* E17: uart0_rtsn */
#define P2_27(mode) AM33XX_IOPAD(0x0968, mode)  /* E18: uart0_ctsn */
#define P2_28(mode) AM33XX_IOPAD(0x09a8, mode)  /* D13: mcasp0_axr1 */
#define P2_29(mode) AM33XX_IOPAD(0x0964, mode)  /* C18: eCAP0_in_PWM0_out */
#define P2_30(mode) AM33XX_IOPAD(0x099c, mode)  /* C12: mcasp0_ahclkr */
#define P2_31(mode) AM33XX_IOPAD(0x09b0, mode)  /* A15: xdma_event_intr0 */
#define P2_32(mode) AM33XX_IOPAD(0x0998, mode)  /* D12: mcasp0_axr0 */
#define P2_33(mode) AM33XX_IOPAD(0x0834, mode)  /* R12: gpmc_ad13 */
#define P2_34(mode) AM33XX_IOPAD(0x09a4, mode)  /* C13: mcasp0_fsr */
#define P2_35(mode) AM33XX_IOPAD(0x08e0, mode)  /* U5: lcd_vsync */
