#include "queue.h"

 void CQueueInit(Queue *q)
	{
		q->mQueueCtr = 0;        
		q->mQueueInPtr = &q->mQueueBuf[0];
		q->mQueueOutPtr = &q->mQueueBuf[0];
	};


 void Clear(Queue *q)
	{
		q->mQueueCtr = 0;
		q->mQueueInPtr = &q->mQueueBuf[0];
		q->mQueueOutPtr = &q->mQueueBuf[0];
	};
    
    
	
	 uint GetElemNum(Queue *q)
	{
		return (q->mQueueCtr);
	};

	 TBOOL IsEmpty(Queue *q)
	{

		if (0 < q->mQueueCtr)
		{
			return (bFALSE);
		}
		else
		{
			return (bTRUE);
		}
	};
    
	
	 TBOOL IsFull(Queue *q)
	{
        
		if (QUEUE_SIZE > q->mQueueCtr)
		{
			return (bFALSE);
		}
		else
		{
			return (bTRUE);
		}
	};
    
	/*--------------------------------------------------------------------------
    *  Function:
    *  CQueue.EnQueue2
    * 
    *  Parameters:
    *  TType * - ָ��ָ���Ԫ��
    * 
    *  Returns value:
    *  TBOOL -���TType *ָ��ָ���Ԫ�ؽ����гɹ����򷵻�bTRUE,���򷵻�bFALSE.
    * 
    *  Description:
    *  ��ָ����Ԫ�ؽ����У�����ɹ�����bTRUE,ʧ�ܷ���bFALSE,��������������ȫ�ģ�
    *  ֻ�����ж���ʹ�ã����ߵ�����ʹ�á�
    * 
    *--------------------------------------------------------------------------*/
	 TBOOL EnQueue(Queue *q, const OneFrameData *inElemPtr)
	{
		if(NULL == inElemPtr)
		{
			return (bFALSE);
		}
        
		if (q->mQueueCtr < QUEUE_SIZE)
		{
			q->mQueueCtr++;
			*q->mQueueInPtr = *inElemPtr;
			q->mQueueInPtr++;
			if (q->mQueueInPtr == &q->mQueueBuf[QUEUE_SIZE])
			{
				q->mQueueInPtr = q->mQueueBuf;
			}
			return (bTRUE);
		}
		return (bFALSE);
	};
    
    

	 TBOOL GetHead(Queue *q, OneFrameData *outElemPtr)
	{
		if(NULL == outElemPtr)
		{
			return (bFALSE);
		}
        
		if (0 == q->mQueueCtr)
		{
			return (bFALSE);
		}
		else
		{
			*outElemPtr = *q->mQueueOutPtr;  
		}
        
		return (bTRUE);
	}
    



   
	 TBOOL DeQueue(Queue *q,OneFrameData *outElemPtr)
	{
        
		if(NULL == outElemPtr)
		{
			return (bFALSE);
		}
        
		if (0 == q->mQueueCtr)
		{
			return (bFALSE);
		}
		else
		{
			q->mQueueCtr--;
			*outElemPtr = *q->mQueueOutPtr;
			q->mQueueOutPtr++;
			if (q->mQueueOutPtr == &q->mQueueBuf[QUEUE_SIZE])
			{
				q->mQueueOutPtr = q->mQueueBuf;
			}          
		}
        
		return (bTRUE);
	};