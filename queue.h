#ifndef QUEUE_H
#define QUEUE_H

#include "types.h"

extern	void CQueueInit(Queue *q);
extern	void Clear(Queue *q);
extern	TBOOL IsEmpty(Queue *q);
extern	TBOOL IsFull(Queue *q);
extern	TBOOL EnQueue(Queue *q, const OneFrameData *inElemPtr);
extern	TBOOL DeQueue(Queue *q,OneFrameData *outElemPtr);
#endif
