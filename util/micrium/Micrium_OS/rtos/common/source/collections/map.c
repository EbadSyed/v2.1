/*
*********************************************************************************************************
*                                              Micrium OS
*                                                Common
*
*                             (c) Copyright 2017; Silicon Laboratories Inc.
*                                        https://www.micrium.com
*
*********************************************************************************************************
* Licensing:
*           YOUR USE OF THIS SOFTWARE IS SUBJECT TO THE TERMS OF A MICRIUM SOFTWARE LICENSE.
*   If you are not willing to accept the terms of an appropriate Micrium Software License, you must not
*   download or use this software for any reason.
*   Information about Micrium software licensing is available at https://www.micrium.com/licensing/
*   It is your obligation to select an appropriate license based on your intended use of the Micrium OS.
*   Unless you have executed a Micrium Commercial License, your use of the Micrium OS is limited to
*   evaluation, educational or personal non-commercial uses. The Micrium OS may not be redistributed or
*   disclosed to any third party without the written consent of Silicon Laboratories Inc.
*********************************************************************************************************
* Documentation:
*   You can find user manuals, API references, release notes and more at: https://doc.micrium.com
*********************************************************************************************************
* Technical Support:
*   Support is available for commercially licensed users of Micrium's software. For additional
*   information on support, you can contact info@micrium.com.
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*
*                                            MAP INTERFACE
*
* File : map.c
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*********************************************************************************************************
*                                            INCLUDE FILES
*********************************************************************************************************
*********************************************************************************************************
*/

#include  <rtos/common/source/collections/map_priv.h>

#include  <rtos/cpu/include/cpu.h>
#include  <rtos/common/include/lib_def.h>

#include  <rtos/common/source/rtos/rtos_utils_priv.h>


/*
*********************************************************************************************************
*********************************************************************************************************
*                                             LOCAL DEFINES
*********************************************************************************************************
*********************************************************************************************************
*/

#define  LOG_DFLT_CH                       (COMMON, COLLECTIONS, MAP)
#define  RTOS_MODULE_CUR                    RTOS_CFG_MODULE_COMMON


/*
*********************************************************************************************************
*********************************************************************************************************
*                                      LOCAL FUNCTION PROTOTYPES
*********************************************************************************************************
*********************************************************************************************************
*/

static  CPU_BOOLEAN  MapItemExists (MAP_INSTANCE  *p_map_instance,
                                    MAP_ITEM      *p_map_item);

static  CPU_BOOLEAN  MapKeyExists  (MAP_INSTANCE  *p_map_instance,
                                    CPU_CHAR      *key);


/*
*********************************************************************************************************
*********************************************************************************************************
*                                           GLOBAL FUNCTIONS
*********************************************************************************************************
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                               MapInit()
*
* Description : Initializes map instance object.
*
* Argument(s) : p_map_instance  Pointer to map instance object.
*
* Return(s)   : none.
*
* Note(s)     : none.
*********************************************************************************************************
*/

void  MapInit (MAP_INSTANCE  *p_map_instance)
{
    RTOS_ASSERT_DBG((p_map_instance != DEF_NULL), RTOS_ERR_NULL_PTR, ;);


    SList_Init((SLIST_MEMBER **)p_map_instance);
}


/*
*********************************************************************************************************
*                                             MapItemAdd()
*
* Description : Add item to map instance.
*
* Argument(s) : p_map_instance  Pointer to map instance object.
*
*               p_map_item      Pointer to map item to add to map instance.
*
*               p_err           Pointer to the variable that will receive one of the following error
*                               code(s) from this function:
*
*                                   RTOS_ERR_NONE
*                                   RTOS_ERR_ALREADY_EXISTS
*
* Return(s)   : none.
*
* Note(s)     : none.
*********************************************************************************************************
*/

void  MapItemAdd (MAP_INSTANCE  *p_map_instance,
                  MAP_ITEM      *p_map_item,
                  RTOS_ERR      *p_err)
{
    RTOS_ASSERT_DBG_ERR_PTR_VALIDATE(p_err, ;);

    RTOS_ASSERT_DBG_ERR_SET((p_map_instance != DEF_NULL), *p_err, RTOS_ERR_NULL_PTR, ;);
    RTOS_ASSERT_DBG_ERR_SET((p_map_item != DEF_NULL), *p_err, RTOS_ERR_NULL_PTR, ;);


    if (MapItemExists(p_map_instance, p_map_item)) {
        RTOS_ERR_SET(*p_err, RTOS_ERR_ALREADY_EXISTS);
        return;
    }

    if (MapKeyExists(p_map_instance, p_map_item->Key)) {
        RTOS_ERR_SET(*p_err, RTOS_ERR_ALREADY_EXISTS);
        return;
    }

    SList_Push((SLIST_MEMBER **)p_map_instance,
              &p_map_item->ListNode);

    RTOS_ERR_SET(*p_err, RTOS_ERR_NONE);

    return;
}


/*
*********************************************************************************************************
*                                            MapItemRemove()
*
* Description : Remove map item from map instance.
*
* Argument(s) : p_map_instance  Pointer to map instance object.
*
*               p_map_item      Pointer to map item to remove from map instance.
*
*               p_err           Pointer to the variable that will receive one of the following error
*                               code(s) from this function:
*
*                                   RTOS_ERR_NONE
*                                   RTOS_ERR_NOT_FOUND
*
* Return(s)   : none.
*
* Note(s)     : none.
*********************************************************************************************************
*/

void  MapItemRemove (MAP_INSTANCE  *p_map_instance,
                     MAP_ITEM      *p_map_item,
                     RTOS_ERR      *p_err)
{
    RTOS_ASSERT_DBG_ERR_PTR_VALIDATE(p_err, ;);

    RTOS_ASSERT_DBG_ERR_SET((p_map_instance != DEF_NULL), *p_err, RTOS_ERR_NULL_PTR, ;);
    RTOS_ASSERT_DBG_ERR_SET((p_map_item != DEF_NULL), *p_err, RTOS_ERR_NULL_PTR, ;);


    if (MapItemExists(p_map_instance, p_map_item)) {
        SList_Rem((SLIST_MEMBER **)p_map_instance, &(p_map_item->ListNode));
        RTOS_ERR_SET(*p_err, RTOS_ERR_NONE);
    } else {
        RTOS_ERR_SET(*p_err, RTOS_ERR_NOT_FOUND);
    }

    return;
}


/*
*********************************************************************************************************
*                                            MapKeyRemove()
*
* Description : Remove map item associated to 'key'.
*
* Argument(s) : p_map_instance  Pointer to map instance object.
*
*               key             String containing key of map item to remove.
*
*               p_err           Pointer to the variable that will receive one of the following error
*                               code(s) from this function:
*
*                                   RTOS_ERR_NONE
*                                   RTOS_ERR_NOT_FOUND
*
* Return(s)   : none.
*
* Note(s)     : none.
*********************************************************************************************************
*/

void  MapKeyRemove (MAP_INSTANCE  *p_map_instance,
                    CPU_CHAR      *key,
                    RTOS_ERR      *p_err)
{
    MAP_ITEM  *p_remove_item;


    RTOS_ASSERT_DBG_ERR_PTR_VALIDATE(p_err, ;);

    RTOS_ASSERT_DBG_ERR_SET((p_map_instance != DEF_NULL), *p_err, RTOS_ERR_NULL_PTR, ;);
    RTOS_ASSERT_DBG_ERR_SET((key != DEF_NULL), *p_err, RTOS_ERR_NULL_PTR, ;);


    p_remove_item = MapKeyItemGet(p_map_instance, key, p_err);
    if (RTOS_ERR_CODE_GET(*p_err) == RTOS_ERR_NONE) {
        SList_Rem((SLIST_MEMBER **)p_map_instance, &(p_remove_item->ListNode));
    }

    return;
}


/*
*********************************************************************************************************
*                                            MapKeyItemGet()
*
* Description : Obtain pointer to map item with corresponding 'key'.
*
* Argument(s) : p_map_instance  Pointer to map instance object.
*
*               key             String containing key of map item to obtain.
*
*               p_err           Pointer to the variable that will receive one of the following error
*                               code(s) from this function:
*
*                                   RTOS_ERR_NONE
*                                   RTOS_ERR_NOT_FOUND
*
* Return(s)   : Pointer to map item, if item exists,
*               DEF_NULL, otherwise.
*
* Note(s)     : (1) Item is not removed from list at this point. This function merely returns a pointer
*                   to the requested map item.
*********************************************************************************************************
*/

MAP_ITEM  *MapKeyItemGet (MAP_INSTANCE  *p_map_instance,
                          CPU_CHAR      *key,
                          RTOS_ERR      *p_err)
{
    MAP_ITEM  *p_map_item_ret  = DEF_NULL;
    MAP_ITEM  *p_map_item_iter;


    RTOS_ASSERT_DBG_ERR_PTR_VALIDATE(p_err, DEF_NULL);

    RTOS_ASSERT_DBG_ERR_SET((p_map_instance != DEF_NULL), *p_err, RTOS_ERR_NULL_PTR, DEF_NULL);
    RTOS_ASSERT_DBG_ERR_SET((key != DEF_NULL), *p_err, RTOS_ERR_NULL_PTR, DEF_NULL);


    RTOS_ERR_SET(*p_err, RTOS_ERR_NOT_FOUND);
    if (*p_map_instance != DEF_NULL) {
        SLIST_FOR_EACH_ENTRY(*p_map_instance, p_map_item_iter, MAP_ITEM, ListNode) {
            if (Str_Cmp_N(p_map_item_iter->Key, key, DEF_INT_08U_MAX_VAL) == 0) {
                RTOS_ERR_SET(*p_err, RTOS_ERR_NONE);
                p_map_item_ret = p_map_item_iter;
                break;
            }
        }
    }

    return (p_map_item_ret);
}


/*
*********************************************************************************************************
*                                           MapKeyValueGet()
*
* Description : Obtain value associated to the map item corresponding to the 'key' passed.
*
* Argument(s) : p_map_instance  Pointer to map instance object.
*
*               key             String containing key of map item to obtain value from.
*
*               p_err           Pointer to the variable that will receive one of the following error
*                               code(s) from this function:
*
*                                   RTOS_ERR_NONE
*                                   RTOS_ERR_NOT_FOUND
*
* Return(s)   : Value to map item, if item exists,
*               DEF_NULL, otherwise.
*
* Note(s)     : none.
*********************************************************************************************************
*/

void  *MapKeyValueGet (MAP_INSTANCE  *p_map_instance,
                       CPU_CHAR      *key,
                       RTOS_ERR      *p_err)
{
    MAP_ITEM  *p_ret_item;


    RTOS_ASSERT_DBG_ERR_PTR_VALIDATE(p_err, DEF_NULL);

    RTOS_ASSERT_DBG_ERR_SET((p_map_instance != DEF_NULL), *p_err, RTOS_ERR_NULL_PTR, DEF_NULL);
    RTOS_ASSERT_DBG_ERR_SET((key != DEF_NULL), *p_err, RTOS_ERR_NULL_PTR, DEF_NULL);


    p_ret_item = MapKeyItemGet(p_map_instance, key, p_err);
    if (RTOS_ERR_CODE_GET(*p_err) != RTOS_ERR_NONE) {
        return (DEF_NULL);
    }

    return (p_ret_item->Value);
}


/*
*********************************************************************************************************
*********************************************************************************************************
*                                           LOCAL FUNCTIONS
*********************************************************************************************************
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                            MapItemExists()
*
* Description : Checks if a map item exists in map instance.
*
* Argument(s) : p_map_instance  Pointer to map instance object.
*
*               p_map_item      Pointer to map item to search for.
*
* Return(s)   : DEF_YES, if 'p_map_item' points to an existing item in map instance,
*               DEF_NO, otherwise.
*
* Note(s)     : none.
*********************************************************************************************************
*/

static  CPU_BOOLEAN  MapItemExists (MAP_INSTANCE  *p_map_instance,
                                    MAP_ITEM      *p_map_item)
{
    MAP_ITEM     *p_map_item_iter;
    CPU_BOOLEAN   found           = DEF_NO;


    if (*p_map_instance != DEF_NULL) {
        SLIST_FOR_EACH_ENTRY(*p_map_instance, p_map_item_iter, MAP_ITEM, ListNode) {
            if (p_map_item_iter == p_map_item) {
                found = DEF_YES;
            }
        }
    }

    return (found);
}


/*
*********************************************************************************************************
*                                            MapKeyExists()
*
* Description : Checks if a map item's key exists in map instance.
*
* Argument(s) : p_map_instance  Pointer to map instance object.
*
*               key             String containing key to search for in map instance.
*
* Return(s)   : DEF_YES, if 'key' exists in map instance,
*               DEF_NO, otherwise.
*
* Note(s)     : none.
*********************************************************************************************************
*/

static  CPU_BOOLEAN  MapKeyExists (MAP_INSTANCE  *p_map_instance,
                                   CPU_CHAR      *key)
{
    MAP_ITEM     *p_map_item_iter;
    CPU_BOOLEAN   found           = DEF_NO;


    if (*p_map_instance != DEF_NULL) {
        SLIST_FOR_EACH_ENTRY(*p_map_instance, p_map_item_iter, MAP_ITEM, ListNode) {
            if (Str_Cmp_N(p_map_item_iter->Key, key, DEF_INT_08U_MAX_VAL) == 0) {
                found = DEF_YES;
            }
        }
    }

    return (found);
}
