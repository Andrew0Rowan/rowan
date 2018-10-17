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
 * MediaTek Inc. (C) 2016-2017. All rights reserved.
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


/* FILE NAME:
 * AUTHOR:
 * PURPOSE:
 *
 * NOTES:
 */

#include <assert.h>
#include <string.h>

#include "dev_list.h"

#define UNUSED_ATTR __attribute__((unused))

#ifndef BDADDR_LEN
#define  BDADDR_LEN  ((int)      18)
#endif

struct list_node_t {
  struct list_node_t *next;
  char addr[BDADDR_LEN];
  void *data;
};

typedef struct list_t {
  list_node_t *head;
  list_node_t *tail;
  size_t length;
  list_free_cb free_cb;
} list_t;

static list_node_t *list_free_node_(list_t *list, list_node_t *node);

// Hidden constructor, only to be used by the hash map for the allocation tracker.
// Behaves the same as |list_new|, except you get to specify the allocator.
list_t *list_new_internal(list_free_cb callback) {
  list_t *list = (list_t *)calloc(1, sizeof(list_t));
  if (!list)
    return NULL;

  list->free_cb = callback;
  return list;
}

list_t *list_new(list_free_cb callback) {
  return list_new_internal(callback);
}

void list_free(list_t *list) {
  if (!list)
    return;

  list_clear(list);
  free(list);
}

bool list_is_empty(const list_t *list) {
  assert(list != NULL);
  return (list->length == 0);
}

list_node_t * list_contains(const list_t *list, const char *addr) {
  assert(list != NULL);
  assert(addr != NULL);

  for (const list_node_t *node = list_begin(list); node != list_end(list); node = list_next(node)) {
    if(0 == strncmp(node->addr, addr, BDADDR_LEN - 1))
      return node;
  }

  return NULL;
}

size_t list_length(const list_t *list) {
  assert(list != NULL);
  return list->length;
}

void *list_front(const list_t *list) {
  assert(list != NULL);
  assert(!list_is_empty(list));

  return list->head->data;
}

void *list_back(const list_t *list) {
  assert(list != NULL);
  assert(!list_is_empty(list));

  return list->tail->data;
}

list_node_t *list_back_node(const list_t *list) {
  assert(list != NULL);
  assert(!list_is_empty(list));

  return list->tail;
}

bool list_insert_after(list_t *list, list_node_t *prev_node, void *data) {
  assert(list != NULL);
  assert(prev_node != NULL);
  assert(data != NULL);

  list_node_t *node = (list_node_t *)calloc(1, sizeof(list_node_t));
  if (!node)
    return false;

  node->next = prev_node->next;
  node->data = data;
  prev_node->next = node;
  if (list->tail == prev_node)
    list->tail = node;
  ++list->length;
  return true;
}

bool list_prepend(list_t *list, void *data) {
  assert(list != NULL);
  assert(data != NULL);

  list_node_t *node = (list_node_t *)calloc(1, sizeof(list_node_t));
  if (!node)
    return false;
  node->next = list->head;
  node->data = data;
  list->head = node;
  if (list->tail == NULL)
    list->tail = list->head;
  ++list->length;
  return true;
}

bool list_append(list_t *list, char *addr, void *data) {
  assert(list != NULL);
  assert(data != NULL);

  list_node_t *node = (list_node_t *)calloc(1, sizeof(list_node_t));
  if (!node)
    return false;
  node->next = NULL;
  node->data = data;
  strncpy(node->addr, addr, BDADDR_LEN);
  if (list->tail == NULL) {
    list->head = node;
    list->tail = node;
  } else {
    list->tail->next = node;
    list->tail = node;
  }
  ++list->length;
  return true;
}

bool list_remove(list_t *list, char *addr) {
  assert(list != NULL);
  assert(addr != NULL);

  if (list_is_empty(list))
    return false;

  if(strncmp(list->head->addr, addr, BDADDR_LEN) == 0) {
  //if (list->head->data == data) {
    list_node_t *next = list_free_node_(list, list->head);
    if (list->tail == list->head)
      list->tail = next;
    list->head = next;
    return true;
  }

  for (list_node_t *prev = list->head, *node = list->head->next; node; prev = node, node = node->next)
    //if (node->data == data) {
    if(strncmp(node->addr, addr, BDADDR_LEN) == 0) {
      prev->next = list_free_node_(list, node);
      if (list->tail == node)
        list->tail = prev;
      return true;
    }

  return false;
}

void list_clear(list_t *list) {
  assert(list != NULL);
  for (list_node_t *node = list->head; node; )
    node = list_free_node_(list, node);
  list->head = NULL;
  list->tail = NULL;
  list->length = 0;
}

list_node_t *list_foreach(const list_t *list, list_iter_cb callback, void *context) {
  assert(list != NULL);
  assert(callback != NULL);

  for (list_node_t *node = list->head; node; ) {
    list_node_t *next = node->next;
    if (!callback(node->data, context))
      return node;
    node = next;
  }
  return NULL;
}

list_node_t *list_begin(const list_t *list) {
  assert(list != NULL);
  return list->head;
}

list_node_t *list_end(UNUSED_ATTR const list_t *list) {
  assert(list != NULL);
  return NULL;
}

list_node_t *list_next(const list_node_t *node) {
  assert(node != NULL);
  return node->next;
}

void *list_node(const list_node_t *node) {
  assert(node != NULL);
  return node->data;
}

void list_move_end(list_t *list, char *addr)
{
  list_node_t *mv_node = NULL;
  assert(list != NULL);
  assert(addr != NULL);

  if (list_is_empty(list))
    return;

  if(strncmp(list->tail->addr, addr, BDADDR_LEN) == 0) {
    return;
  }

  if(strncmp(list->head->addr, addr, BDADDR_LEN) == 0) {
    mv_node = list->head;
    if (list->tail == list->head)
      list->tail = mv_node->next;
    list->head = mv_node->next;

    if (list->tail == NULL) {
      list->head = mv_node;
      list->tail = mv_node;
    } else {
      list->tail->next = mv_node;
      list->tail = mv_node;
    }
    return;
  }

  for (list_node_t *prev = list->head, *node = list->head->next; node; prev = node, node = node->next)
    if(strncmp(node->addr, addr, BDADDR_LEN) == 0) {
      prev->next = node->next;
      list->tail->next = node;
      list->tail = node;
      return;
    }

  return;
}

static list_node_t *list_free_node_(list_t *list, list_node_t *node) {
  assert(list != NULL);
  assert(node != NULL);

  list_node_t *next = node->next;

  if (list->free_cb)
    list->free_cb(node->data);
  if(!node)
  {
     return NULL;
  }
  free(node);
  --list->length;

  return next;
}
