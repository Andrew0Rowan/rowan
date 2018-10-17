/* Copyright Statement:                                                        
 *                                                                             
 * This software/firmware and related documentation ("MediaTek Software") are  
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without 
 * the prior written permission of MediaTek inc. and/or its licensors, any     
 * reproduction, modification, use or disclosure of MediaTek Software, and     
 * information contained herein, in whole or in part, shall be strictly        
 * prohibited.                                                                 
 *                                                                             
 * MediaTek Inc. (C) 2014. All rights reserved.                                
 *                                                                             
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES 
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")     
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER  
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL          
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED    
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR          
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH 
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,            
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.   
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK       
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE  
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR     
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S 
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE       
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE  
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE  
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.    
 *                                                                             
 * The following software/firmware and/or related documentation ("MediaTek     
 * Software") have been modified by MediaTek Inc. All revisions are subject to 
 * any receiver's applicable license agreements with MediaTek Inc.             
 */


#ifndef _U_GPIO_H_
#define _U_GPIO_H_
/*-----------------------------------------------------------------------------
                    include files
-----------------------------------------------------------------------------*/
#include "u_common.h"


//-----------------------------------------------------------------------------
// Constant definitions
//-----------------------------------------------------------------------------
#define OUTPUT 		1
#define INPUT 		0
#define HIGH        1
#define LOW         0



#define GPIO_BASE_VALUE         387           //only MT8516 is this value, others chip is different

// GPIO name definitions
#define	GPIO0     	            0
#define	GPIO1     	            1
#define	GPIO2     	            2
#define	GPIO3     	            3
#define	GPIO4     	            4
#define	GPIO5     	            5
#define	GPIO6     	            6
#define	GPIO7     	            7
#define	GPIO8     	            8
#define	GPIO9     	            9
#define	GPIO10	                10
#define	GPIO11	                11
#define	GPIO12	                12
#define	GPIO13	                13
#define	GPIO14	                14
#define	GPIO15	                15
#define	GPIO16	                16
#define	GPIO17	                17
#define	GPIO18	                18
#define	GPIO19	                19
#define	GPIO20	                20
#define	GPIO21	                21
#define	GPIO22	                22
#define	GPIO23	                23
#define	GPIO24	                24
#define	GPIO25	                25
#define	GPIO26	                26
#define	GPIO27	                27
#define	GPIO28	                28
#define	GPIO29	                29
#define	GPIO30	                30
#define	GPIO31	                31
#define	GPIO32	                32
#define	GPIO33	                33
#define	GPIO34	                34
#define	GPIO35	                35
#define	GPIO36	                36
#define	GPIO37	                37
#define	GPIO38	                38
#define	GPIO39	                39
#define	GPIO40	                40
#define	GPIO41	                41
#define	GPIO42	                42
#define	GPIO43	                43
#define	GPIO44	                44
#define	GPIO45	                45
#define	GPIO46	                46
#define	GPIO47	                47
#define	GPIO48	                48
#define	GPIO49	                49
#define	GPIO50	                50
#define	GPIO51	                51
#define	GPIO52	                52
#define	GPIO53	                53
#define	GPIO54	                54
#define	GPIO55	                55
#define	GPIO56	                56
#define	GPIO57	                57
#define	GPIO58	                58
#define	GPIO59	                59
#define	GPIO60	                60
#define	GPIO61	                61
#define	GPIO62	                62
#define	GPIO63	                63
#define	GPIO64	                64
#define	GPIO65	                65
#define	GPIO66	                66
#define	GPIO67	                67
#define	GPIO68	                68
#define	GPIO69	                69
#define	GPIO70	                70
#define	GPIO71	                71
#define	GPIO72	                72
#define	GPIO73	                73
#define	GPIO74	                74
#define	GPIO75	                75
#define	GPIO76	                76
#define	GPIO77	                77
#define	GPIO78	                78
#define	GPIO79	                79
#define	GPIO80	                80
#define	GPIO81	                81
#define	GPIO82	                82
#define	GPIO83	                83
#define	GPIO84	                84
#define	GPIO85	                85
#define	GPIO86	                86
#define	GPIO87	                87
#define	GPIO88	                88
#define	GPIO89	                89
#define	GPIO90	                90
#define	GPIO91	                91
#define	GPIO92	                92
#define	GPIO93	                93
#define	GPIO94	                94
#define	GPIO95	                95
#define	GPIO96	                96
#define	GPIO97	                97
#define	GPIO98	                98
#define	GPIO99	                99
#define	GPIO100	                100
#define	GPIO101	                101
#define	GPIO102	                102
#define	GPIO103	                103
#define	GPIO104	                104
#define	GPIO105	                105
#define	GPIO106	                106
#define	GPIO107	                107
#define	GPIO108	                108
#define	GPIO109	                109
#define	GPIO110	                110
#define	GPIO111	                111
#define	GPIO112	                112
#define	GPIO113	                113
#define	GPIO114	                114
#define	GPIO115	                115
#define	GPIO116	                116
#define	GPIO117	                117
#define	GPIO118	                118
#define	GPIO119	                119
#define	GPIO120	                120
#define	GPIO121	                121
#define	GPIO122	                122
#define	GPIO123	                123
#define	GPIO124	                124




extern int gpio_export(unsigned int gpio);
extern int gpio_unexport(unsigned int gpio);
extern int gpio_set_dir(unsigned int gpio, unsigned int out_flag);
extern int gpio_set_value(unsigned int gpio, unsigned int value);
extern int gpio_get_value(unsigned int gpio);
extern int gpio_set_edge(unsigned int gpio, char *edge);
extern int gpio_fd_open(unsigned int gpio);
extern int gpio_fd_close(int fd);  
extern void gpio_config(unsigned int i4GpioNum, unsigned int i4OutIn, unsigned int i4High);


#endif /* _U_GPIO_H_ */
