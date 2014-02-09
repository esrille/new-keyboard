//DOM-IGNORE-BEGIN
/*******************************************************************************
Software License Agreement

The software supplied herewith by Microchip Technology Incorporated
(the "Company") for its PICmicro(R) Microcontroller is intended and
supplied to you, the Company's customer, for use solely and
exclusively on Microchip PICmicro Microcontroller products. The
software is owned by the Company and/or its supplier, and is
protected under applicable copyright laws. All rights are reserved.
Any use in violation of the foregoing restrictions may subject the
user to criminal sanctions under applicable laws, as well as to
civil liability for the breach of the terms and conditions of this
license.

THIS SOFTWARE IS PROVIDED IN AN "AS IS" CONDITION. NO WARRANTIES,
WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING, BUT NOT LIMITED
TO, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
PARTICULAR PURPOSE APPLY TO THIS SOFTWARE. THE COMPANY SHALL NOT,
IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL OR
CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.

*******************************************************************************/
//DOM-IGNORE-END

#ifndef STRUCT_QUEUE_H
#define STRUCT_QUEUE_H


/* StructQueueInit
 *************************************************************************
 * Precondition:    None
 *
 * Input:           q   Pointer to the queue data structure
 *
 *                  N   Number of elements in the queue data buffer array
 *
 * Output:          None
 *
 * Returns:         zero (0)
 *
 * Side Effects:    The queue structure has been initialized and is ready 
 *                  to use.
 *
 * Overview:        This operation initializes a queue and makes it empty.
 *
 * Note:            This operation is implemented with a macro that 
 *                  supports queues of any type of data items.
 *************************************************************************/
 
#define StructQueueInit(q,N) (  (q)->head  = (N), \
                                (q)->tail  = (N), \
                                (q)->count =  0   )


/* StructQueueAdd
 *************************************************************************
 * Precondition:    The queue must have been initialized and must not 
 *                  currently be full.
 *
 * Input:           q   Pointer to the queue data structure
 *
 *                  N   Number of elements in the queue data buffer array
 *
 * Output:          None
 *
 * Returns:         The address of the new item in the queue.
 *
 * Side Effects:    The item has been added to the queue.
 *
 *                  IMPORTANT!  No data has been copied to the item.
 *
 * Overview:        This operation adds (enqueues) a new item into the 
 *                  queue data buffer and updates the head index,
 *                  handling buffer wrap correctly.
 *
 * Notes:           The caller must first ensure that the queue is not 
 *                  full by performing one of the other operations (such 
 *                  as "StructQueueIsNotFull") before performing this 
 *                  operation.  Adding an item into a full queue will 
 *                  cause an access violation.
 *
 *                  This operation is implemented with a macro that 
 *                  supports queues of any type of data items.
 *************************************************************************/

#define StructQueueAdd(q,N) ( (q)->count++,            \
                              ( ((q)->head < (N-1)) ?  \
                                  ((q)->head++)     :  \
                                  ((q)->head=0)     ), \
                              &(q)->buffer[(q)->head]  )


/* StructQueueRemove
 *************************************************************************
 * Precondition:    The queue must have been initialized and not currently
 *                  be empty.
 *
 * Input:           q   Pointer to the queue data structure
 *
 *                  N   Number of elements in the queue data buffer array
 *
 * Output:          None
 *
 * Returns:         The item removed.
 *
 * Side Effects:    The item has been removed from the queue.
 *
 *                  IMPORTANT!  No data has been copied from the item.
 *
 * Overview:        This routine removes (dequeues) an item from the 
 *                  queue data buffer and updates the tail index,
 *                  handling buffer wrap correctly.
 *
 * Notes:           The caller must first ensure that the queue is not
 *                  empty by calling one or more of the other operations
 *                  (such as "StructQueueIsNotEmpty") before performing this
 *                  operation.  Dequeueing an item from an empty queue
 *                  will cause an access violation.
 *
 *                  This operation is implemented with a macro that 
 *                  supports queues of any type of data items.
 *************************************************************************/

#define StructQueueRemove(q,N) ( (q)->count--,            \
                                 ( ((q)->tail < (N-1)) ?  \
                                     ((q)->tail++)     :  \
                                     ((q)->tail=0)     ), \
                                 &(q)->buffer[(q)->tail]  )


/* StructQueuePeekTail
 *************************************************************************
 * Precondition:    The queue must have been initialized and not currently
 *                  be empty.
 *
 * Input:           q   Pointer to the queue data structure
 *
 *                  N   Number of elements in the queue data buffer array
 *
 * Output:          None
 *
 * Returns:         The item at the tail of the queue.
 *
 * Side Effects:    None
 *
 *                  IMPORTANT!  No data has been copied from the item.
 *
 * Overview:        This routine provides access to an item in the 
 *                  queue data buffer at the tail index position,
 *                  handling buffer wrap correctly.
 *
 * Notes:           The caller must first ensure that the queue is not
 *                  empty by calling one or more of the other operations
 *                  (such as "StructQueueIsNotEmpty") before performing this
 *                  operation.
 *
 *                  This operation is implemented with a macro that 
 *                  supports queues of any type of data items.
 *************************************************************************/

#define StructQueuePeekTail(q,N) ( ((q)->tail < (N-1))         ?  \
                                     &(q)->buffer[(q)->tail+1] :  \
                                     &(q)->buffer[0]              )


/* StructQueueIsFull
 *************************************************************************
 * Precondition:    The queue must be initialized.
 *
 * Input:           q   Pointer to the queue data structure
 *
 *                  N   Number of elements in the queue data buffer array
 *
 * Output:          None
 *
 * Returns:         TRUE if the queue is full, FALSE otherwise.
 *
 * Side Effects:    None
 *
 * Overview:        This routine checks to see if the queue is full.
 *
 * Note:            This operation is implemented with a macro that 
 *                  supports queues of any type of data items.
 *************************************************************************/

#define StructQueueIsFull(q,N) ( (q)->count >= N )


/* StructQueueIsNotFull
 *************************************************************************
 * Precondition:    The queue must be initialized.
 *
 * Input:           q   Pointer to the queue data structure
 *
 *                  N   Number of elements in the queue data buffer array
 *
 * Output:          None
 *
 * Returns:         FALSE if the queue is full, TRUE otherwise.
 *
 * Side Effects:    None
 *
 * Overview:        This routine checks to see if the queue is full.
 *
 * Note:            This operation is implemented with a macro that 
 *                  supports queues of any type of data items.
 *************************************************************************/

#define StructQueueIsNotFull(q,N) ( (q)->count < N )


/* StructQueueIsEmpty
 *************************************************************************
 * Precondition:    The queue must be initialized.
 *
 * Input:           q   Pointer to the queue data structure
 *
 *                  N   Number of elements in the queue data buffer array
 *
 * Output:          None
 *
 * Returns:         TRUE if the queue is empty, FALSE otherwise.
 *
 * Side Effects:    None
 *
 * Overview:        This routine checks to see if the queue is empty.
 *
 * Note:            This operation is implemented with a macro that 
 *                  supports queues of any type of data items.
 *************************************************************************/

#define StructQueueIsEmpty(q,N) ( (q)->count == 0 )


/* StructQueueIsNotEmpty
 *************************************************************************
 * Precondition:    The queue must be initialized.
 *
 * Input:           q   Pointer to the queue data structure
 *
 *                  N   Number of elements in the queue data buffer array
 *
 * Output:          None
 *
 * Returns:         FALSE if the queue is empty, TRUE otherwise.
 *
 * Side Effects:    None
 *
 * Overview:        This routine checks to see if the queue is not empty.
 *
 * Note:            This operation is implemented with a macro that 
 *                  supports queues of any type of data items.
 *************************************************************************/

#define StructQueueIsNotEmpty(q,N) ( (q)->count != 0 )


/* StructQueueSpaceAvailable
 *************************************************************************
 * Precondition:    The queue must be initialized.
 *
 * Input:           q   Pointer to the queue data structure
 *
 *                  N   Number of elements in the queue data buffer array
 *
 * Output:          None
 *
 * Returns:         TRUE if the queue has the indicated number of slots
                    available, FALSE otherwise.
 *
 * Side Effects:    None
 *
 * Overview:        This routine checks to see if the queue has at least
 *                  the specified number of slots free.
 *
 * Note:            This operation is implemented with a macro that 
 *                  supports queues of any type of data items.
 *************************************************************************/

#define StructQueueSpaceAvailable(c,q,N) ( ((q)->count + c) <= N )


/* StructQueueCount
 *************************************************************************
 * Precondition:    The queue must be initialized.
 *
 * Input:           q   Pointer to the queue data structure
 *
 *                  N   Number of elements in the queue data buffer array
 *
 * Output:          None
 *
 * Returns:         The number of items in the queue.
 *
 * Side Effects:    None
 *
 * Overview:        This routine provides the number of items in the queue.
 *
 * Note:            This operation is implemented with a macro that 
 *                  supports queues of any type of data items.
 *************************************************************************/

#define StructQueueCount(q,N) ( (q)->count )


#endif // STRUCT_QUEUE_H
/*************************************************************************
 * EOF struct_queue.c
 */

