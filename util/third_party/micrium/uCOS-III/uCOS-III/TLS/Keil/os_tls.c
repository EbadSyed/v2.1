/*
************************************************************************************************************************
*                                                      uC/OS-III
*                                                 The Real-Time Kernel
*
*                                  (c) Copyright 2009-2016; Micrium, Inc.; Weston, FL
*                           All rights reserved.  Protected by international copyright laws.
*
*                                        THREAD LOCAL STORAGE (TLS) MANAGEMENT
*                                              KEIL MDK IMPLEMENTATION 
*
* File    : OS_TLS.C
* By      : JJL
* Version : V3.06.01
*
* LICENSING TERMS:
* ---------------
*           uC/OS-III is provided in source form for FREE short-term evaluation, for educational use or 
*           for peaceful research.  If you plan or intend to use uC/OS-III in a commercial application/
*           product then, you need to contact Micrium to properly license uC/OS-III for its use in your 
*           application/product.   We provide ALL the source code for your convenience and to help you 
*           experience uC/OS-III.  The fact that the source is provided does NOT mean that you can use 
*           it commercially without paying a licensing fee.
*
*           Knowledge of the source code may NOT be used to develop a similar product.
*
*           Please help us continue to provide the embedded community with the finest software available.
*           Your honesty is greatly appreciated.
*
*           You can find our product's user manual, API reference, release notes and
*           more information at doc.micrium.com.
*           You can contact us at www.micrium.com.
************************************************************************************************************************
*/

#define  MICRIUM_SOURCE
#include "../../Source/os.h"

#ifdef VSC_INCLUDE_SOURCE_FILE_NAMES
const  CPU_CHAR  *os_tls__c = "$Id: $";
#endif

#if defined(OS_CFG_TLS_TBL_SIZE) && (OS_CFG_TLS_TBL_SIZE > 0u)
/*
************************************************************************************************************************
*                                                   LOCAL CONSTANTS
************************************************************************************************************************
*/

#define  OS_TLS_LOCK_MAX     4                                  /* Number of 'mutexes' needed by the library          */
#define  OS_TLS_STO_SIZE   128                                  /* Number of bytes needed to store all TLS variables  */

/*
************************************************************************************************************************
*                                                  LOCAL DATA TYPES
************************************************************************************************************************
*/

typedef  struct  os_tls_lock   OS_TLS_LOCK;


struct  os_tls_lock {                                           
    OS_MUTEX       Mutex;                                                 /* OS Mutex object.                         */
    OS_TLS_LOCK   *NextPtr;                                               /* Pointer to the next object in the pool.  */
};

/*
************************************************************************************************************************
*                                                   LOCAL VARIABLES
************************************************************************************************************************
*/

static  CPU_DATA             OS_TLS_NextAvailID;                          /* Next available TLS ID                    */

static  CPU_DATA             OS_TLS_LibSpaceID;                           /* ID used to store library space pointer   */

static  OS_TLS_LOCK          OS_TLS_LockPoolTbl[OS_TLS_LOCK_MAX];
static  OS_TLS_LOCK         *OS_TLS_LockPoolListPtr = (OS_TLS_LOCK *)0;   /* Pointer to head of 'OS_TLS_LOCK' list    */

extern  void                *__libspace_start;							  /* Single threaded STDLIB storage pointer   */


/*
************************************************************************************************************************
*                                       ALLOCATE THE NEXT AVAILABLE TLS ID
*
* Description: This function is called to obtain the ID of the next free TLS (Task Local Storage) register 'id'
*
* Arguments  : p_err       is a pointer to a variable that will hold an error code related to this call.
*
*                            OS_ERR_NONE               if the call was successful
*                            OS_ERR_TLS_NO_MORE_AVAIL  if you are attempting to assign more TLS than you declared
*                                                           available through OS_CFG_TLS_TBL_SIZE.
*
* Returns    : The next available TLS 'id' or OS_CFG_TLS_TBL_SIZE if an error is detected.
************************************************************************************************************************
*/

OS_TLS_ID  OS_TLS_GetID (OS_ERR  *p_err)
{
    OS_TLS_ID  id;
    CPU_SR_ALLOC();



#ifdef OS_SAFETY_CRITICAL
    if (p_err == (OS_ERR *)0) {
        OS_SAFETY_CRITICAL_EXCEPTION();
        return ((OS_TLS_ID)OS_CFG_TLS_TBL_SIZE);
    }
#endif

    CPU_CRITICAL_ENTER();
    if (OS_TLS_NextAvailID >= OS_CFG_TLS_TBL_SIZE) {        /* See if we exceeded the number of IDs available         */
       *p_err = OS_ERR_TLS_NO_MORE_AVAIL;                   /* Yes, cannot allocate more TLS                          */
        CPU_CRITICAL_EXIT();
        return ((OS_TLS_ID)OS_CFG_TLS_TBL_SIZE);
    }
     
    id    = OS_TLS_NextAvailID;
    OS_TLS_NextAvailID++;
    CPU_CRITICAL_EXIT();
   *p_err = OS_ERR_NONE;
    return (id);
}


/*
************************************************************************************************************************
*                                        GET THE CURRENT VALUE OF A TLS REGISTER
*
* Description: This function is called to obtain the current value of a TLS register
*
* Arguments  : p_tcb     is a pointer to the OS_TCB of the task you want to read the TLS register from.  If 'p_tcb' is 
*                        a NULL pointer then you will get the TLS register of the current task.
*
*              id        is the 'id' of the desired TLS register.  Note that the 'id' must be less than 
*                        'OS_TLS_NextAvailID'
*
*              p_err     is a pointer to a variable that will hold an error code related to this call.
*
*                            OS_ERR_NONE            if the call was successful
*                            OS_ERR_OS_NOT_RUNNING  if the kernel has not started yet
*                            OS_ERR_TLS_ID_INVALID  if the 'id' is greater or equal to OS_TLS_NextAvailID
*                            OS_ERR_TLS_NOT_EN      if the task was created by specifying that TLS support was not
*                                                     needed for the task
*
* Returns    : The current value of the task's TLS register or 0 if an error is detected.
*
* Note(s)    : 1) p_tcb->Opt contains options passed to OSTaskCreate().  One of these options (OS_OPT_TASK_NO_TLS) is
*                 used to specify that the user doesn't want TLS support for the task being created.  In other words,
*                 by default, TLS support is enabled if OS_CFG_TLS_TBL_SIZE is defined and > 0 so the user must
*                 specifically indicate that he/she doesn't want TLS supported for a task.
************************************************************************************************************************
*/

OS_TLS  OS_TLS_GetValue (OS_TCB     *p_tcb,
                         OS_TLS_ID   id,
                         OS_ERR     *p_err)
{
    OS_TLS    value;
    CPU_SR_ALLOC();



#ifdef OS_SAFETY_CRITICAL
    if (p_err == (OS_ERR *)0) {
        OS_SAFETY_CRITICAL_EXCEPTION();
        return ((OS_TLS)0);
    }
#endif


#if OS_CFG_ARG_CHK_EN > 0u
    if (id >= OS_TLS_NextAvailID) {                             /* Caller must specify an ID that's been assigned     */
       *p_err = OS_ERR_TLS_ID_INVALID;
        return ((OS_TLS)0);
    }
#endif

    CPU_CRITICAL_ENTER();
    if (p_tcb == (OS_TCB *)0) {                                 /* Does caller want to use current task's TCB?        */
        p_tcb = OSTCBCurPtr;                                    /* Yes                                                */
        if (OSTCBCurPtr == (OS_TCB *)0) {                       /* Is the kernel running?                             */
            CPU_CRITICAL_EXIT();                                /* No, then caller cannot specify NULL                */
           *p_err = OS_ERR_OS_NOT_RUNNING;
            return ((OS_TLS)0);
        }
    }
    if ((p_tcb->Opt & OS_OPT_TASK_NO_TLS) == OS_OPT_NONE) {     /* See if TLS is available for this task              */
        value = p_tcb->TLS_Tbl[id];                             /* Yes                                                */
        CPU_CRITICAL_EXIT();
       *p_err = OS_ERR_NONE;
        return ((OS_TLS)value);
    } else {
        CPU_CRITICAL_EXIT();                                    /* No                                                 */
       *p_err = OS_ERR_TLS_NOT_EN;
        return ((OS_TLS)0);
    }
}


/*
************************************************************************************************************************
*                                          DEFINE TLS DESTRUCTOR FUNCTION
*
* Description: This function is called by the user to assign a 'destructor' function to a specific TLS.  When a task is
*              deleted, all the destructors are called for all the task's TLS for which there is a destructor function
*              defined.  In other when a task is deleted, all the non-NULL functions present in OS_TLS_DestructPtrTbl[] 
*              will be called.
*
* Arguments  : id          is the ID of the TLS destructor to set
*
*              p_destruct  is a pointer to a function that is associated with a specific TLS register and is called when 
*                          a task is deleted.  The prototype of such functions is:
*
*                            void  MyDestructFunction (OS_TCB     *p_tcb,
*                                                      OS_TLS_ID   id,
*                                                      OS_TLS      value); 
*
*                          you can specify a NULL pointer if you don't want to have a fucntion associated with a TLS 
*                          register.  A NULL pointer (i.e. no function associated with a TLS register) is the default 
*                          value placed in OS_TLS_DestructPtrTbl[].
*
*              p_err       is a pointer to an error return code.  The possible values are:
*
*                            OS_ERR_NONE             The call was successful.
*                            OS_ERR_TLS_ID_INVALID   You you specified an invalid TLS ID
*
* Returns    : none 
*
* Note       : none
************************************************************************************************************************
*/

void  OS_TLS_SetDestruct (OS_TLS_ID            id,
                          OS_TLS_DESTRUCT_PTR  p_destruct,
                          OS_ERR              *p_err)
{
   (void)&id;
   (void)&p_destruct;
   *p_err = OS_ERR_NONE;
}


/*
************************************************************************************************************************
*                                       SET THE CURRENT VALUE OF A TASK TLS REGISTER
*
* Description: This function is called to change the current value of a task TLS register.  
*
* Arguments  : p_tcb     is a pointer to the OS_TCB of the task you want to set the task's TLS register for.  If 'p_tcb' 
*                        is a NULL pointer then you will change the TLS register of the current task.
*
*              id        is the 'id' of the desired task TLS register.  Note that the 'id' must be less than    
*                        'OS_TLS_NextAvailID'
*
*              value     is the desired value for the task TLS register.
*
*              p_err     is a pointer to a variable that will hold an error code related to this call.
*
*                            OS_ERR_NONE            if the call was successful
*                            OS_ERR_OS_NOT_RUNNING  if the kernel has not started yet
*                            OS_ERR_TLS_ID_INVALID  if you specified an invalid TLS ID
*                            OS_ERR_TLS_NOT_EN      if the task was created by specifying that TLS support was not
*                                                     needed for the task
*
* Returns    : none
*
* Note(s)    : 1) p_tcb->Opt contains options passed to OSTaskCreate().  One of these options (OS_OPT_TASK_NO_TLS) is
*                 used to specify that the user doesn't want TLS support for the task being created.  In other words,
*                 by default, TLS support is enabled if OS_CFG_TLS_TBL_SIZE is defined and > 0 so the user must
*                 specifically indicate that he/she doesn't want TLS supported for a task.
************************************************************************************************************************
*/

void  OS_TLS_SetValue (OS_TCB     *p_tcb,
                       OS_TLS_ID   id,
                       OS_TLS      value,
                       OS_ERR     *p_err)
{
    CPU_SR_ALLOC();



#ifdef OS_SAFETY_CRITICAL
    if (p_err == (OS_ERR *)0) {
        OS_SAFETY_CRITICAL_EXCEPTION();
        return;
    }
#endif

#if OS_CFG_ARG_CHK_EN > 0u
    if (id >= OS_TLS_NextAvailID) {                             /* Caller must specify an ID that's been assigned     */
       *p_err = OS_ERR_TLS_ID_INVALID;
        return;
    }
#endif

    CPU_CRITICAL_ENTER();                                       /* Does caller want to use current task's TCB?        */
    if (p_tcb == (OS_TCB *)0) {                                 /* Yes                                                */
        p_tcb = OSTCBCurPtr;                                    /* Is the kernel running?                             */
        if (OSTCBCurPtr == (OS_TCB *)0) {                       /* No, then caller cannot specify NULL                */
            CPU_CRITICAL_EXIT();
           *p_err = OS_ERR_OS_NOT_RUNNING;
            return;
        }
    }
    if ((p_tcb->Opt & OS_OPT_TASK_NO_TLS) == OS_OPT_NONE) {     /* See if TLS is available for this task              */
        p_tcb->TLS_Tbl[id] = value;                             /* Yes                                                */
        CPU_CRITICAL_EXIT();
       *p_err              = OS_ERR_NONE;
    } else {
        CPU_CRITICAL_EXIT();                                    /* No                                                 */
       *p_err              = OS_ERR_TLS_NOT_EN;
    }
}


/*
************************************************************************************************************************
************************************************************************************************************************
*                                             uC/OS-III INTERNAL FUNCTIONS
*                                         DO NOT CALL FROM THE APPLICATION CODE
************************************************************************************************************************
************************************************************************************************************************
*/

/*
************************************************************************************************************************
*                                       INITIALIZE THE TASK LOCAL STORAGE SERVICES
*
* Description: This function is called by uC/OS-III to initialize the TLS id allocator.
*
*              This function also initializes an array containing function pointers.  There is one function associated 
*                  to each task TLS register and all the functions (assuming non-NULL) will be called when the task is 
*                  deleted.
*
* Arguments  : none
*
* Returns    : none
*
* Note(s)    : 1) This function is INTERNAL to uC/OS-III and your application should not call it.
************************************************************************************************************************
*/

void  OS_TLS_Init (void)
{
    CPU_INT16U    ix;
    OS_TLS_LOCK  *p_lock;
    OS_ERR        err;
    CPU_SR_ALLOC();


    OS_TLS_NextAvailID = 0u;
    OS_TLS_LibSpaceID  = OS_TLS_GetID(&err);

    CPU_CRITICAL_ENTER();    
                                                                   /* Create the link list of OS_TLS_LOCK objects.    */
    for (ix = 0u; ix < (OS_TLS_LOCK_MAX - 1u); ix++) {
        p_lock             = &OS_TLS_LockPoolTbl[ix];
        p_lock->NextPtr    = &OS_TLS_LockPoolTbl[ix + 1u];
    }
    
    p_lock                 = &OS_TLS_LockPoolTbl[OS_TLS_LOCK_MAX - 1u];
    p_lock->NextPtr        = (OS_TLS_LOCK *)0;                     /* Last node points to 'NULL'                      */

    OS_TLS_LockPoolListPtr = &OS_TLS_LockPoolTbl[0];               /* Initialize the list head pointer.               */

    CPU_CRITICAL_EXIT();    
}


/*
************************************************************************************************************************
*                                                  TASK CREATE HOOK
*
* Description: This function is called by OSTaskCreate()
*
* Arguments  : p_tcb     is a pointer to the OS_TCB of the task being created.
*
* Returns    : none 
*
*
* Note(s)    : 1) This function is INTERNAL to uC/OS-III and your application should not call it.
*
*              2) OSTaskCreate() clears all entries in p_tcb->TLS_Tbl[] before calling OS_TLS_TaskCreate() so no need
*                 to this here.
************************************************************************************************************************
*/

void  OS_TLS_TaskCreate (OS_TCB  *p_tcb)
{
    OS_TLS  p_tls;


    if ((p_tcb->Opt & OS_OPT_TASK_NO_TLS) == OS_OPT_NONE) {    /* See if TLS is available for this task               */
        p_tls                             = malloc(128);       /* Allocate storage for STDLIB's per-thread storage    */
        p_tcb->TLS_Tbl[OS_TLS_LibSpaceID] = p_tls;             /* Save pointer to this storage area in the TCB        */
	}
}


/*
************************************************************************************************************************
*                                                  TASK DELETE HOOK
*
* Description: This function is called by OSTaskDel()
*
* Arguments  : p_tcb     is a pointer to the OS_TCB of the task being deleted.
*
* Returns    : none 
*
* Note(s)    : 1) This function is INTERNAL to uC/OS-III and your application should not call it.
************************************************************************************************************************
*/

void  OS_TLS_TaskDel (OS_TCB  *p_tcb)
{
    OS_TLS  p_tls;


    p_tls = p_tcb->TLS_Tbl[OS_TLS_LibSpaceID];
    if ((p_tcb->Opt & OS_OPT_TASK_NO_TLS) == OS_OPT_NONE) {    /* See if TLS is available for this task               */
        free((void *)p_tls);                                   /* Free storage used for STDLIB's per-thread storage   */
        p_tcb->TLS_Tbl[OS_TLS_LibSpaceID] = (OS_TLS)0;	       /* Put null pointer indicating no longer valid pointer */
	}
}


/*
************************************************************************************************************************
*                                                  TASK SWITCH HOOK
*
* Description: This function is called by OSSched() and OSIntExit() just prior to calling the context switch code
*
* Arguments  : none
*
* Returns    : none 
*
* Note(s)    : 1) This function is INTERNAL to uC/OS-III and your application should not call it.
*
*              2) It's assumed that OSTCBCurPtr points to the task being switched out and OSTCBHighRdyPtr points to the
*                 task being switched in.
************************************************************************************************************************
*/

void  OS_TLS_TaskSw (void)
{
    if ((OSTCBHighRdyPtr->Opt & OS_OPT_TASK_NO_TLS) == OS_OPT_NONE) {    /* See if TLS is available for this task     */
    }
}


/*
************************************************************************************************************************
*                                  KEIL STANDARD LIBRARY MULTITHREAD INTERFACE
*
* Description: These are functions expected by the Keil STDLIB to support multithreading.
*
* Arguments  : none
*
* Returns    : none 
*
* Note       : none
************************************************************************************************************************
*/

void  *__user_perthread_libspace (void)
{
    if (OSRunning == OS_STATE_OS_STOPPED) {
	    return ((void *)&__libspace_start);
	}
	return (OSTCBCurPtr->TLS_Tbl[OS_TLS_LibSpaceID]);
}



/*
************************************************************************************************************************
*                                            INITIALIZE THE STDLIB MUTEX
************************************************************************************************************************
*/

int  _mutex_initialize (void  **p_mutex)
{
    OS_TLS_LOCK   *p_tls_lock;
    OS_ERR         os_err;
    CPU_SR_ALLOC();

    
    if (p_mutex == (void **)0) {
        return (0);
    }

    if (OS_TLS_LockPoolListPtr == (OS_TLS_LOCK *)0) {             /* If 'OS_TLS_LOCK' object pool is empty?           */
        *p_mutex = (void *)0;                                     /*   return a 'NULL' pointer.                       */
        return (0);
    }

    p_tls_lock = OS_TLS_LockPoolListPtr;                          /* Get the first object in the list.                */

    OSMutexCreate((OS_MUTEX *)&p_tls_lock->Mutex,                 /* Create the mutex in the kernel.                  */
                  (CPU_CHAR *) 0,				       
                  (OS_ERR   *)&os_err);			       
                             					       
    if (os_err != OS_ERR_NONE) {                                  /* If the mutex create funtion fail?                */
        *p_mutex = (void *)0;                                     /* ... return a 'NULL' pointer.                     */
         return (0);        						       
    }											       

    CPU_CRITICAL_ENTER();
    OS_TLS_LockPoolListPtr = p_tls_lock->NextPtr;                 /* Move HEAD pointer to the next object in the list.*/
    CPU_CRITICAL_EXIT();
    
    *p_mutex = (void *)p_tls_lock;                                /* Return the new 'OS_TLS_LOCK' object pointer.     */
	return (1);
}

/*
************************************************************************************************************************
*                                            WAIT FOR THE STDLIB MUTEX
************************************************************************************************************************
*/

__used  void  _mutex_acquire (void  **p_mutex)
{
    OS_TLS_LOCK  *p_tls_lock;
    OS_ERR        os_err;    
    
    
    if ((p_mutex    == (void **)0         ) ||                    /* Return if the lock handler is 'NULL' or the ...  */
        (OSRunning != OS_STATE_OS_RUNNING)) {                     /* ... kernel is not running.                       */
        return;
    }
    
    p_tls_lock = (OS_TLS_LOCK *)*p_mutex;
    OSMutexPend((OS_MUTEX *)&p_tls_lock->Mutex,
                (OS_TICK   ) 0u,
                (OS_OPT    ) OS_OPT_PEND_BLOCKING,
                (CPU_TS   *) 0,
                (OS_ERR   *)&os_err);
    (void)&os_err;
}

/*
************************************************************************************************************************
*                                            RELEASE THE STDLIB MUTEX
************************************************************************************************************************
*/

__used  void  _mutex_release (void  **p_mutex)
{
    OS_TLS_LOCK  *p_tls_lock;
    OS_ERR        os_err;    
    
    
    if ((p_mutex   == (void **)0          ) ||                    /* Return if the lock handler is 'NULL' or the ...  */
        (OSRunning != OS_STATE_OS_RUNNING)) {                     /* ... kernel is not running.                       */
        return;
    }
    
    p_tls_lock = (OS_TLS_LOCK *)*p_mutex;
    OSMutexPost((OS_MUTEX *)&p_tls_lock->Mutex,
                (OS_OPT    ) OS_OPT_POST_NONE,
                (OS_ERR   *)&os_err);

    (void)&os_err;
}

#endif
