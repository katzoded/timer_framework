/* dllLib.c - efficient doubly linked list subroutine library */

/* Copyright 1984-1998 Wind River Systems, Inc. */

/*
modification history
--------------------
01n,11nov01,dee  Add ColdFire support
01m,04sep98,cdp  make ARM CPUs with ARM_THUMB==TRUE use portable routines.
01n,07mar00,zl   fixed syntax error from merge
01m,03mar00,zl   merged SH support from T1
01l,22apr97,jpd  added support for optimised ARM code.
01k,19mar95,dvs  removing tron references.
01j,09jun93,hdn  added a support for I80X86
01i,09jul92,hdn  put an optimized dllGet()
01h,24jun92,ajm  pulled in optimizations for mips
01g,26may92,rrr  the tree shuffle
01f,19nov91,rrr  shut up some ansi warnings.
01e,04oct91,rrr  passed through the ansification filter
                  -changed functions to ansi style
		  -fixed #else and #endif
		  -changed VOID to void
		  -changed copyright notice
01d,23jul91,hdn  added conditional macro for optimized TRON codes.
01c,28oct90,jcf  documentation.
01b,10may90,jcf  fixed PORTABLE definition.
01a,03jun89,jcf  written.
*/


/*
DESCRIPTION
This subroutine library supports the creation and maintenance of a
doubly linked list.  The user supplies a list descriptor (type DL_LIST)
that will contain pointers to the first and last nodes in the list.
The nodes in the list can be any user-defined structure, but they must reserve
space for a pointer as their first element.  The forward chain is terminated
with a nullptr pointer.

This library in conjunction with dllALib.s, and the macros defined in dllLib.h,
provide a reduced version of the routines offered in lstLib(1).  For
efficiency, the count field has been eliminated, and enqueueing and dequeueing
functions have been hand optimized.

.ne 16
NON-EMPTY LIST:
.CS

   ---------             --------          --------
   | head--------------->| next----------->| next---------
   |       |             |      |          |      |      |
   |       |        ------ prev |<---------- prev |      |
   |       |       |     |      |          |      |      |
   | tail------    |     | ...  |    ----->| ...  |      |
   |-------|  |    v                 |                   v
              |  -----               |                 -----
              |   ---                |                  ---
              |    -                 |                   -
              ------------------------

.CE
.ne 12
EMPTY LIST:
.CS

	-----------
        |  head------------------
        |         |             |
        |  tail----------       |
        |         |     v       v
        |         |   -----   -----
        -----------    ---     ---
                        -	-

.CE

INCLUDE FILE: dllLib.h
*/


/*
 * optimized version available for 680X0, MIPS, I80X86, SH and ARM,
 * but not yet Thumb.
 */


/* dllLib.h - doubly linked list library header */
#ifndef __INCdllLibh
#define __INCdllLibh


#ifndef OK
#define OK 0 
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* type definitions */
typedef struct dlnode    /* Node of a linked list. */
    {
    struct dlnode *next;  /* Points at the next node in the list */
    struct dlnode *previous;  /* Points at the previous node in the list */
    } DL_NODE;

typedef struct      /* Header for a linked list. */
    {
    DL_NODE *head;  /* header of list */
    DL_NODE *tail;  /* tail of list */
    } DL_LIST;

/*******************************************************************************
*
* dllFirst - find first node in list
*
* DESCRIPTION
* Finds the first node in a doubly linked list.
*
* RETURNS
*  Pointer to the first node in a list, or
*  nullptr if the list is empty.
*
* NOMANUAL
*/

#define DLL_FIRST(pList)    \
    (          \
    (((DL_LIST *)(pList))->head)  \
    )

/*******************************************************************************
*
* dllLast - find last node in list
*
* Finds the last node in a doubly linked list.
*
* RETURNS
*  pointer to the last node in list, or
*  nullptr if the list is empty.
*
* NOMANUAL
*/

#define DLL_LAST(pList)      \
    (          \
    (((DL_LIST *)(pList))->tail)  \
    )

/*******************************************************************************
*
* dllNext - find next node in list
*
* Locates the node immediately after the node pointed to by the pNode.
*
* RETURNS:
*   Pointer to the next node in the list, or
*  nullptr if there is no next node.
*
* NOMANUAL
*/

#define DLL_NEXT(pNode)      \
    (          \
    (((DL_NODE *)(pNode))->next)  \
    )

/*******************************************************************************
*
* dllPrevious - find preceding node in list
*
* Locates the node immediately before the node pointed to by the pNode.
*
* RETURNS:
*   Pointer to the preceding node in the list, or
*  nullptr if there is no next node.
*
* NOMANUAL
*/

#define DLL_PREVIOUS(pNode)    \
    (          \
    (((DL_NODE *)(pNode))->previous)  \
    )

/*******************************************************************************
*
* dllEmpty - boolean function to check for empty list
*
* RETURNS:
*   TRUE if list is empty
*  FALSE otherwise
*
* NOMANUAL
*/

#define DLL_EMPTY(pList)      \
    (            \
    (((DL_LIST *)pList)->head == nullptr)    \
    )

typedef int     (*DLL_EACH_FUNCPTR) (DL_NODE *node, void *cookie);     /* ptr to function returning int */


/*********************************************************************
*
* dllInit - initialize doubly linked list descriptor
*
* Initialize the specified list to an empty list.
*
* RETURNS: OK, or ERROR if doubly linked list could not be initialized.
*/

__inline__ short dllInit
        (
                DL_LIST *pList     /* pointer to list descriptor to be initialized */
        )
{
    pList->head	 = nullptr;
    pList->tail  = nullptr;

    return (OK);
}

/*********************************************************************
*
* dllCreate - create a doubly linked list descriptor
*
* This routine returns a pointer to an initialized doubly linked list
* descriptor.
*
* RETURNS: Pointer to a doubly linked list descriptor, or nullptr if ERROR.
*/

__inline__ DL_LIST *dllCreate (void)
{
    DL_LIST *pList = (DL_LIST *) malloc ((unsigned) sizeof (DL_LIST));

    dllInit (pList);

    return (pList);
}

/*******************************************************************************
*
* dllDelete - terminate doubly linked list head free associated memory
*
* Terminate the specified list and deallocated associated memory.
*
* RETURNS: OK, or ERROR if doubly linked list could not be deleted.
*
* ARGSUSED
*/

__inline__ short dllDelete
        (
                DL_LIST *pList     /* pointer to list head to be initialized */
        )
{
    free ((char *) pList);			/* free list */
    return OK;
}

/*******************************************************************************
*
* dllTerminate - terminate doubly linked list head
*
* Terminate the specified list without deallocating associated memory.
*
* RETURNS: OK, or ERROR if doubly linked list could not be terminated.
*
* ARGSUSED
*/

__inline__ short dllTerminate
        (
                DL_LIST *pList     /* pointer to list head to be initialized */
        )
{
    return (OK);
}

/************************************************************************
*
* dllInsert - insert node in list after specified node
*
* This routine inserts the specified node in the specified list.
* The new node is placed following the specified 'previous' node in the list.
* If the specified previous node is nullptr, the node is inserted at the head
* of the list.
*/

__inline__ void dllInsert
        (
                DL_LIST *pList,        /* pointer to list descriptor */
                DL_NODE *pPrev,        /* pointer to node after which to insert */
                DL_NODE *pNode         /* pointer to node to be inserted */
        )
{
    DL_NODE *pNext;

    if (pPrev == nullptr)
    {				/* new node is to be first in list */
        pNext = pList->head;
        pList->head = pNode;
    }
    else
    {				/* make prev node point fwd to new */
        pNext = pPrev->next;
        pPrev->next = pNode;
    }

    if (pNext == nullptr)
        pList->tail = pNode;		/* new node is to be last in list */
    else
        pNext->previous = pNode;	/* make next node point back to new */


    /* set pointers in new node */

    pNode->next		= pNext;
    pNode->previous	= pPrev;
}

/************************************************************************
*
* dllAdd - add node to end of list
*
* This routine adds the specified node to the end of the specified list.
*/

__inline__ void dllAdd
        (
                DL_LIST *pList,     /* pointer to list descriptor */
                DL_NODE *pNode      /* pointer to node to be added */
        )
{
    dllInsert (pList, pList->tail, pNode);
}

/************************************************************************
*
* dllRemove - remove specified node in list
*
* Remove the specified node in the doubly linked list.
*/

__inline__ void dllRemove
        (
                DL_LIST *pList,             /* pointer to list descriptor */
                DL_NODE *pNode              /* pointer to node to be deleted */
        )
{
    if (pNode->previous == nullptr)
        pList->head = pNode->next;
    else
        pNode->previous->next = pNode->next;

    if (pNode->next == nullptr)
        pList->tail = pNode->previous;
    else
        pNode->next->previous = pNode->previous;
}
/************************************************************************
*
* dllGet - get (delete and return) first node from list
*
* This routine gets the first node from the specified list, deletes the node
* from the list, and returns a pointer to the node gotten.
*
* RETURNS: Pointer to the node gotten, or nullptr if the list is empty.
*/

__inline__ DL_NODE *dllGet
        (
                DL_LIST *pList         /* pointer to list from which to get node */
        )
{
    DL_NODE *pNode = pList->head;

    if (pNode != nullptr)                      /* is list empty? */
    {
        pList->head = pNode->next;          /* make next node be 1st */

        if (pNode->next == nullptr)            /* is there any next node? */
            pList->tail = nullptr;             /*   no - list is empty */
        else
            pNode->next->previous = nullptr;   /*   yes - make it 1st node */
    }

    return (pNode);
}


/**************************************************************************
*
* dllCount - report number of nodes in list
*
* This routine returns the number of nodes in the given list.
*
* CAVEAT
* This routine must actually traverse the list to count the nodes.
* If counting is a time critical fuction, consider using lstLib(1) which
* maintains a count field.
*
* RETURNS: Number of nodes in specified list.
*
* SEE ALSO: lstLib(1).
*/

__inline__ int dllCount
        (
                DL_LIST *pList      /* pointer to list descriptor */
        )
{
    DL_NODE *pNode = DLL_FIRST (pList);
    int count = 0;

    while (pNode != nullptr)
    {
        count++;
        pNode = DLL_NEXT (pNode);
    }

    return (count);
}


/*******************************************************************************
*
* dllEach - call a routine for each node in a linked list
*
* This routine calls a user-supplied routine once for each node in the
* linked list.  The routine should be declared as follows:
*
* .CS
*  BOOL routine (pNode, arg)
*      DL_NODE *pNode;	/@ pointer to a linked list node    @/
*      int	arg;	/@ arbitrary user-supplied argument @/
* .CE
*
* The user-supplied routine should return TRUE if dllEach() is to
* continue calling it with the remaining nodes, or FALSE if it is done and
* dllEach() can exit.
*
* RETURNS: nullptr if traversed whole linked list, or pointer to DL_NODE that
*          dllEach ended with.
*/

__inline__ DL_NODE *dllEach
        (
                DL_LIST     *pList,         /* linked list of nodes to call routine for */
                DLL_EACH_FUNCPTR     routine,        /* the routine to call for each list node */
                void *routineArg      /* arbitrary user-supplied argument */
        )
{
    DL_NODE *pNode = DLL_FIRST (pList);

    while (pNode != nullptr) {
        DL_NODE *pNextNode = DLL_NEXT (pNode);
        if (!(*routine)(pNode, routineArg)) {
            break;
        }
        pNode = pNextNode;
    }

    return (pNode);			/* return node we ended with */
}


#ifdef __cplusplus
}
#endif

#endif /* __INCdllLibh */

