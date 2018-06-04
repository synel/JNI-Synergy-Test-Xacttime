// 20140324 CENT931 指纹模块 485通信发送指纹特征值
#include "485_finger.h"

_ZW485_INFO ZW485_INFO;

// 接口
READER_INFO	g_ReaderInfo[FPREADER_COUNT];
unsigned char			g_ReaderIndex=0;
unsigned long g_ReaderAckTime[FPREADER_COUNT] = {0, 0, 0, 0};
BOOL g_RcomLocked = FALSE;
BOOL rcom_VerifyFpFromSlave(long *Ret);
// 初始化串口指纹仪
// com: 串口号
int init_ComFingerPrinter(int com)
{
	char buf[32];
	struct termios tio;
	speed_t btl=B115200; // set to 115200

	memset(buf,0,sizeof(buf));
	sprintf(buf, "/dev/ttySAC%d", com);

	ZW485_INFO.fd = open(buf,O_RDWR|O_NOCTTY);//|O_NONBLOCK);
	if(ZW485_INFO.fd <= 0)
		return -1;

	tcgetattr(ZW485_INFO.fd, &tio);
	cfmakeraw(&tio);
	cfsetispeed(&tio,btl);
	cfsetospeed(&tio,btl);
	tcsetattr(ZW485_INFO.fd,TCSANOW,&tio);
	tcflush(ZW485_INFO.fd, TCIOFLUSH);		// clear port
//	Com_SetIOBlockFlag(ZW485_INFO.fd, 1);
	ZW485_INFO.DevId = 0x01;
	ZW485_INFO.HostId = RCOM_A30C_ID;

	rcom_updateReaderList(TRUE);
	return ZW485_INFO.fd;
}

// 指纹模板数据获取和比对
long Com_FP_One2NMatch()
{
	long nRet = -1;

	if(rcom_VerifyFpFromSlave(&nRet))
	{
		if(nRet < 0) return nRet;
		else return gMatchData[nRet].ID;
	}
	else return -1;
}

// 同步模式发送接收串口数据 (串口指纹未使用该函数)
int ComSendRecv( int comfd, unsigned char *iBuf, int iLen, unsigned char *oBuf, int oLen, int timeout )
{
	unsigned char	* addr = NULL;
	int				num=0, count=0;

	struct timeval tout;
	fd_set write_fd,read_fd;

	if(comfd <= 0 || !oBuf || oLen < 0) return -1;

	FD_ZERO(&write_fd);
	FD_ZERO(&read_fd);
	FD_SET(comfd,&write_fd);
	FD_SET(comfd,&read_fd);

	if (iBuf && iLen > 0){
		while( iLen != num ){
			tout.tv_sec  = 0;
			tout.tv_usec = 100000;
			if( select( comfd + 1, NULL, &write_fd, NULL, &tout ) <= 0 ){
				return -1;
			}
			addr	   = iBuf + num;
			count  = write( comfd, addr, iLen - num );
			if( count < 0 ){
				return -1;
			}
			num += count;
		}
	}
	num = 0;
	while( oLen != num ){
		tout.tv_sec  = 0;
		tout.tv_usec = 600000;
		if( select( comfd + 1, &read_fd, NULL, NULL, &tout ) <= 0 ){
			goto END;
		}
		addr	   = oBuf + num;
		count  = read( comfd, addr, oLen - num );
		if( count < 0 ){
			printf("dd\n");
			goto END;
		}
		num += count;
	}

END:
	return num;
}

// 设置比IO口的块标识 （该函数串口指纹未使用）
int Com_SetIOBlockFlag(int fd,int value)
{
  int oldflags;

  if (fd == -1)
    return -1;

  oldflags = fcntl(fd,F_GETFL,0);
  if(oldflags == -1) {
    printf("get IO flags error\n");
    return -1;
  }

  if(value == 0)
    oldflags &= ~O_NONBLOCK;  //设置成阻塞IO
  else
    oldflags |= O_NONBLOCK;   //设置成非阻塞IO

  return fcntl(fd,F_GETFL,oldflags);
}

// 向串口写数据
int UsartWrite( int comfd, unsigned char *iBuf, int iLen )
{
	unsigned char	* addr = NULL;
	int				num=0, count=0;

	struct timeval tout;
	fd_set write_fd;

	if(comfd <= 0 || !iBuf || iLen<=0) return -1;

	FD_ZERO(&write_fd);
	FD_SET(comfd,&write_fd);

	while( iLen != num ){
		tout.tv_sec  = 0;
		tout.tv_usec = 100000;
		if( select( comfd + 1, NULL, &write_fd, NULL, &tout ) <= 0 ){
			return -1;
		}
		addr	   = iBuf + num;
		count  = write( comfd, addr, iLen - num );
		if( count < 0 ){
			return -1;
		}
		num += count;
	}
	return num;
}

// 读取串口数据
int UsartRead( int comfd, unsigned char *oBuf, int oLen)
{
	unsigned char	* addr = NULL;
	int				num=0, count=0;
	struct timeval tout;
	fd_set read_fd;

	if(comfd <= 0 || !oBuf || oLen < 0) return -1;

	FD_ZERO(&read_fd);
	FD_SET(comfd,&read_fd);

	while( oLen != num ){
		tout.tv_sec  = 0;
		tout.tv_usec = 40000;
		if( select( comfd + 1, &read_fd, NULL, NULL, &tout ) <= 0 ){
			goto END;
		}
		addr	   = oBuf + num;
		count  = read( comfd, addr, oLen - num );
		if( count < 0 ){
			goto END;
		}
		num += count;
	}
END:
	return num;
}
// 向串口发送数据
BOOL rcom_send(void* pbuf, int nsize)
{
	return (UsartWrite(ZW485_INFO.fd, (unsigned char*)pbuf, (unsigned int)nsize) == nsize);
}

// 从串口公共缓冲区读取数据
BOOL rcom_recv(unsigned char* pbuf, unsigned int nsize)
{
	int i = 0;
	int tmpPtr = 0;
	unsigned char *headPtr = NULL;

	if (UsartRxFIFO.FFOutOffset == UsartRxFIFO.FFInOffset){
		return FALSE;
	}

	tmpPtr = UsartRxFIFO.FFOutOffset;
	headPtr = UsartRxFIFO.FIFO;

	while (tmpPtr != UsartRxFIFO.FFInOffset){
		pbuf[i] = *( headPtr + tmpPtr );
		i++;
		tmpPtr++;
		tmpPtr %= MAX_USART_FIFO_SIZE;
		if (i >= nsize){
			break;
		}
	}
	UsartRxFIFO.FFOutOffset = tmpPtr;

	if(i!=nsize)
		return FALSE;
	return TRUE;
}
// 从串口读取len字节数据到缓冲区
int _Read_UartData( int comfd, int Len )
{
	int				num	   = 0;
	unsigned char	dataBuf[2048];
	int 			remindLen = 0;	// 缓存剩余长度
	int 			dataLen = 0;
	unsigned char *Addr = NULL;

	if (comfd <= 0){
		return 0;
	}

	remindLen = UsartRxFIFO.FFInOffset - UsartRxFIFO.FFOutOffset;
	if (remindLen < 0){
		remindLen += MAX_USART_FIFO_SIZE;
	}

	remindLen = MAX_USART_FIFO_SIZE - 1 - remindLen; // 实际可用数 - 1

	if (remindLen == 0){	// 满
		return 0;
	}

	remindLen = (remindLen > sizeof(dataBuf) ? sizeof(dataBuf) : remindLen);	// 不能超出临时缓存

	if (Len){
		remindLen = (remindLen > Len ? Len : remindLen);
	}

	dataLen  = UsartRead( comfd, dataBuf, remindLen );
	if (dataLen <= 0){
		return 0;
	}

	num = dataLen;
	Addr = dataBuf;

	if (UsartRxFIFO.FFInOffset + dataLen > MAX_USART_FIFO_SIZE)	// 折回
	{
		memcpy(UsartRxFIFO.FIFO + UsartRxFIFO.FFInOffset, Addr, MAX_USART_FIFO_SIZE - UsartRxFIFO.FFInOffset);
		dataLen -= (MAX_USART_FIFO_SIZE - UsartRxFIFO.FFInOffset);
		Addr += (MAX_USART_FIFO_SIZE - UsartRxFIFO.FFInOffset);
		UsartRxFIFO.FFInOffset = 0; // must
	}

	memcpy(UsartRxFIFO.FIFO + UsartRxFIFO.FFInOffset, Addr, dataLen);
	UsartRxFIFO.FFInOffset += dataLen;
	UsartRxFIFO.FFInOffset %= MAX_USART_FIFO_SIZE;

	return num;
}
// 计算数据的sum码
WORD rcom_calcsum(void *pData, int nSize)
{
	WORD wChkSum = 0;
	int i;
	for ( i = 0; i < nSize; i++ )
		wChkSum += ((BYTE*)pData)[i];
	return wChkSum;
}
// 接收串口数据
int rcom_recdata(BYTE srcMID, void* pData, DWORD dwSize)
{
	WORD CheckSum;
	BYTE*	pPoint = (BYTE*)pData;
	int Bytes, Remain = (int)dwSize;
	BYTE pBuf[BLOCK_SIZE + 6 + 4];
	DWORD dwTime;
	int n, i;

	Bytes = dwSize;
	while (Remain > 0)
	{
		Bytes = (Remain > BLOCK_SIZE) ? BLOCK_SIZE : Remain;

		_Read_UartData(ZW485_INFO.fd, 0); // 读取全部串口数据

		dwTime = GetTickCount();
		n = 0;
		while (n < 6)
		{
			n = UsartRxFIFO.FFInOffset - UsartRxFIFO.FFOutOffset;
			if (n < 0)
				n += MAX_USART_FIFO_SIZE;
			if ((DWORD)(GetTickCount() - dwTime) > (DWORD)5000)
				return -1;
		}

		pBuf[1] = UsartRxFIFO.FIFO[UsartRxFIFO.FFOutOffset];
		UsartRxFIFO.FFOutOffset = KEY_BUF_MODINC(UsartRxFIFO.FFOutOffset);
		for (n = 0; n < 5; n++)
		{
			pBuf[0] = pBuf[1];
			pBuf[1] = UsartRxFIFO.FIFO[UsartRxFIFO.FFOutOffset];
			UsartRxFIFO.FFOutOffset = KEY_BUF_MODINC(UsartRxFIFO.FFOutOffset);
			if (pBuf[0] == STX3 && pBuf[1] == STX4)
				break;
		}
		if (pBuf[0] != STX3 || pBuf[1] != STX4)
			return -1;

#ifdef COMM_CRYPT_KEY
		if (!rcom_recv(pBuf + 2, Bytes + 4 + 4) ||
			(pBuf[2] != 0 && pBuf[2] != RCOM_A30C_ID) ||
			(srcMID != 0 && pBuf[3] != srcMID))
			return -1;

		CheckSum = pBuf[Bytes + 8] + (pBuf[Bytes + 9] << 8);
		if (rcom_calcsum(pBuf, Bytes + 4 + 4) != CheckSum)
		{
			return -1;
		}

		for (i = 0; i < 4; i++)
			vCryptKey.vRandKeyArray[i] = pBuf[Bytes + 4 + i];

		vCryptKey.vRandKey ^= COMM_CRYPT_KEY;

		for (i = 4; i < Bytes + 4; i++)
			pBuf[i] = pBuf[i] ^ vCryptKey.vRandKeyArray[i % 4];
		memcpy(pPoint, pBuf+4, Bytes);
		pPoint += Bytes;
		Remain -= Bytes;
#else

		if (!rcom_recv(pBuf + 2, Bytes + 4) ||
			(pBuf[2] != 0 && pBuf[2] != RCOM_A30C_ID) ||
			(srcMID != 0 && pBuf[3] != srcMID))
		{
			return -1;
		}
		CheckSum = pBuf[Bytes+4] + (pBuf[Bytes+5] << 8);
		if (rcom_calcsum(pBuf, Bytes + 4) != CheckSum)
			return -1;
		memcpy(pPoint, pBuf+4, Bytes);
		pPoint += Bytes;
		Remain -= Bytes;
#endif
	}
	return dwSize;
}

// 向主机发送数据---->
int rcom_senddata(BYTE destMID, void* pData, DWORD dwSize)
{
	WORD CheckSum;
	BYTE*	pPoint = (BYTE*)pData;
	int	Remain = dwSize, Bytes;
	BYTE pBuf[BLOCK_SIZE + 6 + 4];
	while (Remain > 0)
	{
		Bytes = (Remain > BLOCK_SIZE) ? BLOCK_SIZE : Remain;
		pBuf[0] = STX3; // 0x5A
		pBuf[1] = STX4; // 0xA5
		pBuf[2] = destMID;
		pBuf[3] = RCOM_A30C_ID; // host id  0x69=105
		memcpy(pBuf + 4, pPoint, Bytes); // data N byte

#ifdef COMM_CRYPT_KEY
		int nsize = Bytes + 4;
		int i;
		srand(GetTickCount());
		vCryptKey.vRandKey = (DWORD)rand() * (DWORD)rand();

		for (i = 4; i < nsize; i++)
		{
			pBuf[i] = pBuf[i] ^ vCryptKey.vRandKeyArray[i % 4];
		}

		vCryptKey.vRandKey ^= COMM_CRYPT_KEY;
		for (i = 0; i < 4; i++ )
			pBuf[nsize + i] = vCryptKey.vRandKeyArray[i % 4];
		nsize += 4;

		CheckSum = rcom_calcsum(pBuf, Bytes + 4 + 4);
		pBuf[nsize] = (BYTE)CheckSum;
		pBuf[nsize + 1] = (BYTE)(CheckSum >> 8);
		nsize += 2;
		if (!rcom_send(pBuf, nsize))
			return -1;
#else

		CheckSum = rcom_calcsum(pBuf, Bytes + 4);		// check sum is 2 byte
		pBuf[Bytes + 4] = (BYTE)CheckSum;
		pBuf[Bytes + 5] = (BYTE)(CheckSum >> 8);
		if (!rcom_send(pBuf, Bytes + 6))
			return -1;
#endif
		pPoint += Bytes;
		Remain -= Bytes;
	}
	return dwSize;
}


// 向主机发送命令包
BOOL rcom_sendpacket(BYTE byHead1, BYTE byHead2, BYTE byMID, BYTE byCode, DWORD dwData)
{
	RCOMPKT vPKT = {0};

	vPKT.Head1 = byHead1;	// 0
	vPKT.Head2 = byHead2;	// 1
	vPKT.destMID = byMID;	// 2
	vPKT.Code = byCode;		// 3
	vPKT.dwData = dwData;	// 4			// 4字节参数
	vPKT.srcMID = RCOM_A30C_ID;	// 8		// 主机 ID
	vPKT.ChkSum = (BYTE)rcom_calcsum(&vPKT, RCOMPKTSIZE - 1);	// 1
	return rcom_send(&vPKT, RCOMPKTSIZE);
}
// 从公共缓冲区拷贝数据
int rcom_copydata(void* pData, int Size)
{
	int i;
	BYTE *pTemp = (BYTE*)pData;
	unsigned short 	FFOutOffsetTemp = UsartRxFIFO.FFOutOffset;

	for (i = 0; i < Size; i++)
	{
		pTemp[i] = UsartRxFIFO.FIFO[FFOutOffsetTemp];
		FFOutOffsetTemp = KEY_BUF_MODINC(FFOutOffsetTemp);
	}
	return i;
}
// 检查报数据的合法性
int rcom_checkpkt(RCOMPKT* pPKT)
{
	int nRet = 0;

	if ( pPKT->destMID != 0 && pPKT->destMID != RCOM_A30C_ID )
		return 0;

	if (pPKT->Head1 == STX1)
	{
		if (pPKT->Head2 != STX2)
			return 0;
		nRet = RCOM_RES_CMD;
	}
	else if (pPKT->Head1 == STX2)
	{
		if (pPKT->Head2 != STX1)
			return 0;
		nRet = (pPKT->Code == 0) ? RCOM_RES_NAK : RCOM_RES_ACK;
	}
	else
	{
		return 0;
	}
	if (pPKT->ChkSum != (BYTE)rcom_calcsum(pPKT, RCOMPKTSIZE - 1))
		return 0;

	return nRet;
}

// 从缓冲区中读取一个数据包
int rcom_getpkt(RCOMPKT* pPKT)
{
	int n, nRet = 0;
	while (TRUE)
	{
		n = UsartRxFIFO.FFInOffset - UsartRxFIFO.FFOutOffset;
		if (n < 0)
			n += MAX_USART_FIFO_SIZE;
		if (n < RCOMPKTSIZE)
			return 0;
		rcom_copydata(pPKT, RCOMPKTSIZE);
		if ((nRet = rcom_checkpkt(pPKT)))
		{
			UsartRxFIFO.FFOutOffset = (UsartRxFIFO.FFOutOffset + RCOMPKTSIZE) % MAX_USART_FIFO_SIZE;
			return nRet;
		}
		UsartRxFIFO.FFOutOffset = KEY_BUF_MODINC(UsartRxFIFO.FFOutOffset);
	}
}

// 接收应答包
int rcom_receiveAck(BYTE srcMID, DWORD dwTimeOut, DWORD *pdwOut)
{
	RCOMPKT vPKT;

	int nRet;
	int nsize;

	nsize = 10;
	_Read_UartData(ZW485_INFO.fd, nsize); //

	if ((nRet = rcom_getpkt(&vPKT)) && vPKT.srcMID == srcMID && (nRet == RCOM_RES_ACK || nRet == RCOM_RES_NAK))
	{
		if (pdwOut)
			*pdwOut = vPKT.dwData;
		return nRet;
	}
	return -1;
}

// 0x67, isConnected 检查串口指纹是否在线
BOOL rcom_checkAlive(int nReaderID)
{
	BOOL bRet = FALSE;

	int count = 0;
	while (count < RETRY_SEARCHING_READER)
	{
		if (rcom_sendcmd(nReaderID, RCMD_AR1003_ALIVE, 0) &&
			rcom_receiveAck(nReaderID, RCOM_ACK_TIMEOUT_SHORT, NULL) >= 0)
		{
			printf("checkAlive OK\n");
			bRet = TRUE;
			break;
		}
		count++;
	}
	return bRet;
}

// 获得指纹仪的列表
void rcom_updateReaderList(BOOL bForce)
{
	int i;

	DWORD dwTouchUse = FALSE;
	g_ReaderIndex = 0;

	if ( bForce )
	{
		for ( i = 0; i < FPREADER_COUNT; i++ )
			g_ReaderAckTime[i] = 0;
	}
	for ( i = 0; i < FPREADER_COUNT; i++ )
	{
		g_ReaderAckTime[i] = GetTickCount();
		int count = 0;
		while (count < RETRY_SEARCHING_READER)
		{
			if (rcom_sendcmd(i+1, RCMD_AR1003_ALIVE, 0) &&
				rcom_receiveAck(i+1, RCOM_ACK_TIMEOUT_MEDIUM, &dwTouchUse) >= 0)
				break;
			count++;
		}
		if (count >= RETRY_SEARCHING_READER)
			g_ReaderInfo[i].byID = 0;
		else
		{
			g_ReaderInfo[i].byID =  i+1;
			g_ReaderInfo[i].bTouchUse = dwTouchUse;
		}
	}
}

// 打印信息和当前时间（ms）
void Print_CurrentTime(const char * as_content)
{
	printf ( "%s , time: %ld ms\n",as_content, GetTickCount());
}
// 检验指纹
BOOL rcom_VerifyFpFromSlave(long *Ret)
{
	int		nRet = TRUE;
	BOOL	bRet = FALSE;
	DWORD	nVerifyMode;
	int		nLength;
	int		dwID[2] = {0};
	DWORD	dwAlarm;
	void*	pTemlate;
	int		rcom_id = 0;
	unsigned long	dwAdapted;

	int i = 0;
	for ( i = 0; i < FPREADER_COUNT; i++ )
	{
		if ( g_ReaderInfo[(g_ReaderIndex+i+1) % FPREADER_COUNT].byID )
		{
			g_ReaderIndex = (g_ReaderIndex+i+1) % FPREADER_COUNT;
			break;
		}
	}
	rcom_id = g_ReaderInfo[g_ReaderIndex].byID;
	if (!rcom_id)
	{
		bRet = FALSE;
		goto _lexit;
	}
	// Print_CurrentTime("1 rcom_sendcmd");
	if (!rcom_sendcmd(rcom_id, RCMD_AR1003_FP_SLAVE_SEND, 0))
	{
		bRet = FALSE;
		goto _lexit;
	}
	// Print_CurrentTime("2 rcom_receiveAck");
	if ((nRet = rcom_receiveAck(rcom_id, RCOM_ACK_TIMEOUT_MEDIUM, &dwAlarm)) < 0)
	{
		bRet = FALSE;
		goto _lexit;
	}

	g_ReaderAckTime[rcom_id-1] = GetTickCount();

	if (nRet != RCOM_RES_ACK)
	{
		bRet = FALSE;
		goto _lexit;
	}
	// Print_CurrentTime("3 rcom_recdata nVerifyMode");
	if (rcom_recdata(rcom_id, &nVerifyMode, 4) < 0)
	{
		rcom_sendresp(rcom_id, 0, 0);
		bRet = FALSE;
		goto _lexit;
	}
	if (nVerifyMode == COM_UI_VERIFY_FP)
	{
		pTemlate = g_TempBuffer;
		nLength = sizeof(FPINFO);
		// char ls_buff[64];
		// sprintf(ls_buff, "3 rcom_recdata nVerifyMode %d",nLength);
		// Print_CurrentTime( ls_buff );
	}
//	else if (nVerifyMode == COM_UI_VERIFY_CARD)
//	{
//		pTemlate = dwCardData;
//		nLength = 8;
//	}
	else
	{
		rcom_sendresp(rcom_id, 0, 0);
		bRet = FALSE;
		goto _lexit;
	}

//	rcom_sendresp(rcom_id, 1, 0);  lee del
	// Print_CurrentTime("4 rcom_recdata pTemlate");
	if (rcom_recdata(rcom_id, pTemlate, nLength)< 0)
	{
		rcom_sendresp(rcom_id, 0, 0);
		bRet = FALSE;
		goto _lexit;
	}

	if (nVerifyMode == COM_UI_VERIFY_FP)
	{
		/* Identify fingerprint template from reader */
		bRet = TRUE;
		nRet = SB_fp(FP_IDENTIFYFPDATA, (long)(&g_TempBuffer), (long)&dwAdapted, 0, 0, 0);
		*Ret = nRet;
		// Print_CurrentTime("5 FP_IDENTIFYFPDATA");
		/* Reply result to reader */
		if( nRet >= 0 )
		{
			dwID[0] = TRUE;
			if (rcom_senddata(rcom_id, dwID, 8) < 0)
			{
				bRet = FALSE;
				goto _lexit;
			}
		}
		else
		{
			dwID[0] = FALSE;
			if (rcom_senddata(rcom_id, dwID, 8) < 0)
			{
				bRet = FALSE;
				goto _lexit;
			}
		}
	}
	else if (nVerifyMode == COM_UI_VERIFY_CARD)
	{
		/* Identify card number from reader */
//		...

		/* Reply result to reader */
//		if (success)
		if(0)
		{
			dwID[0] = TRUE;
			if (rcom_senddata(rcom_id, dwID, 8) < 0)
			{
				bRet = FALSE;
				goto _lexit;
			}
		}
		else
		{
			dwID[0] = FALSE;
			if (rcom_senddata(rcom_id, dwID, 8) < 0)
			{
				bRet = FALSE;
				goto _lexit;
			}
		}
	}
	else
	{
		bRet = FALSE;
	}

_lexit:
	return bRet;
}
// 检查命令结束
int rcom_checkCmdFinish(BYTE byMID, BYTE byCode, DWORD* pdwOut, DWORD dwWait)
{
	DWORD dwTime = GetTickCount();
	int nRet = -1;
	while (dwWait > (DWORD)(GetTickCount() - dwTime))
	{
		if (!rcom_sendcmd(byMID, byCode, CMD_FINISH_MARK)) return -1;
		if ((nRet = rcom_receiveAck(byMID, 20, pdwOut)) >= 0) break;
	}
	return nRet;
}
// 设置指纹仪的编号
BOOL rcom_setReaderID(DWORD dwReaderID)
{
	BOOL bRet;

	if ( !rcom_sendcmd(RCOM_A30R_INVALID_ID, RCMD_AR1003_DEV_ID_SET, dwReaderID) )
	{
		bRet = FALSE;
		goto _exit;
	}

	if (rcom_checkCmdFinish(dwReaderID, RCMD_AR1003_DEV_ID_SET, NULL, RCOM_ACK_TIMEOUT_LONG) < 0)
	{
		bRet = FALSE;
		goto _exit;
	}

	bRet = TRUE;

_exit:
	return bRet;
}

/******************* 调试函数 *********************

int leeShowHexData( unsigned char *in, unsigned int iLen, unsigned int oLen, unsigned int SorR, char *Tishi)
{
	unsigned int i = 0;

#if 1
	return 0;
#endif

if((int)iLen <0) // 防止溢出
	return 0;

	oLen = oLen > iLen ? iLen : oLen;
	printf( "---- %s,len = %d ----", Tishi, iLen);
	if( SorR == 0 )
	{
		NULL;
	}else if( SorR == 1 )
	{
		printf( "\n-->" );
	}else
	{
		printf( "\n<--" );
	}
	for( i = 0; i < iLen; i++ )
	{
		if( i && i % oLen == 0 )
		{
			printf( "\n" );
		}

		printf( "%02X ", in[i] );
	}
	printf( "\n" );

	return 0;
}

long GetTickCount( )
{
	long tck;

	tck = sysconf( _SC_CLK_TCK );
	return times( NULL ) * ( 1000 / tck );
}
*/
