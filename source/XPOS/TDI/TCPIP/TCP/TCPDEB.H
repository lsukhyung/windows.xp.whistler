/********************************************************************/
/**                     Microsoft LAN Manager                      **/
/**               Copyright(c) Microsoft Corp., 1990-1993          **/
/********************************************************************/
/* :ts=4 */

//** TCPDEB.H - TCP debug definitions.
//
// This file contains the definitions for the debug code.
//

#ifdef	DEBUG
extern	void CheckRBList(IPRcvBuf *RBList, uint Size);
extern	void CheckTCBSends(TCB *SendTcb);
extern	void CheckTCBRcv(TCB *RcvTCB);
#else

#define CheckRBList(R, S)
#define CheckTCBSends(T)
#define	CheckTCBRcv(T)
#endif

//
// Additional debugging support for NT
//
#ifdef NT
#if DBG

extern ULONG TCPDebug;

#define TCP_DEBUG_OPEN           0x00000001
#define TCP_DEBUG_CLOSE          0x00000002
#define TCP_DEBUG_ASSOCIATE      0x00000004
#define TCP_DEBUG_CONNECT        0x00000008
#define TCP_DEBUG_SEND           0x00000010
#define TCP_DEBUG_RECEIVE        0x00000020
#define TCP_DEBUG_INFO           0x00000040
#define TCP_DEBUG_IRP            0x00000080
#define TCP_DEBUG_SEND_DGRAM     0x00000100
#define TCP_DEBUG_RECEIVE_DGRAM  0x00000200
#define TCP_DEBUG_EVENT_HANDLER  0x00000400
#define TCP_DEBUG_CLEANUP        0x00000800
#define TCP_DEBUG_CANCEL         0x00001000


#define TCPTRACE(many_args) DbgPrint many_args

#define IF_TCPDBG(flag)  if (TCPDebug & flag)


#else // DBG


#define TCPTRACE(many_args)
#define IF_TCPDBG(flag)   if (0)


#endif // DBG
#else // NT


#define TCPTRACE(many_args)
#define IF_TCPDBG(flag)   if (0)


#endif // NT
