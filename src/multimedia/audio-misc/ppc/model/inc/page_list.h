//MTK : PPC Tool
//(DO NOT EDIT: machine generated header)
#ifndef PAGE_LIST_H
#define PAGE_LIST_H
#include "id.h"
#define PROJ_CODE 0x5a33401b
/*
 * The following sections define condition/ element/ link
 * for default page
 */
int CUR_PLAT = 8516;
/* page 0 */
// condition :
#define page0_cond_num 0


// element :
#define page0_ele_num 3
#define page0_ele0_para_num 3
int page0_ele0_data0[1] = {112};
int page0_ele0_data1[1] = {7};
int page0_ele0_data2[11] = {4,3,7,27,39,31,223,199,63,231,32967};
PARA page0_ele0_para[page0_ele0_para_num] = {
  {0x00008000,1,page0_ele0_data0},
  {0x00008001,1,page0_ele0_data1},
  {0x00008002,11,page0_ele0_data2},
};
#define page0_ele1_para_num 2
int page0_ele1_data0[1] = {47509436};
int page0_ele1_data1[1] = {512};
PARA page0_ele1_para[page0_ele1_para_num] = {
  {0x00010000,1,page0_ele1_data0},
  {0x00010001,1,page0_ele1_data1},
};
#define page0_ele2_para_num 1
int page0_ele2_data0[1] = {1};
PARA page0_ele2_para[page0_ele2_para_num] = {
  {0x7fff8000,1,page0_ele2_data0},
};
ELEMENT page0_ele[page0_ele_num] = {
  {"ELE_INPUT_0",ELE_INPUT_0,0x00000000,0x00000003,PPC_FALSE,PPC_FALSE,3,page0_ele0_para,NULL},
  {"ELE_MASTER_VOLUME_0",ELE_MASTER_VOLUME_0,0x00000003,0x00000003,PPC_FALSE,PPC_FALSE,2,page0_ele1_para,NULL},
  {"ELE_OUTPU_0",ELE_OUTPU_0,0x00000003,0x00000000,PPC_FALSE,PPC_FALSE,1,page0_ele2_para,NULL},
};


 // link :
#define page0_link_num 4
LINK page0_link[page0_link_num] = {
  {0x00000000,0x00000001,0x00010000,0x00000001},
  {0x00000000,0x00000002,0x00010000,0x00000002},
  {0x00010000,0x00000001,0x7fff0000,0x00000001},
  {0x00010000,0x00000002,0x7fff0000,0x00000002},
};
PAGE default_page_list[] = {
  {"NewPage1",0x00000001,1024,PPC_TRUE,0,3,4,NULL,page0_ele,page0_link},
};
const int default_page_num=sizeof(default_page_list)/ sizeof(PAGE);
#endif
