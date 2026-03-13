#define _CRT_SECURE_NO_WARNINGS
#include<time.h>

//将time_t类型转化为字符串，再将字符串转化为“年-月-日 时 ：分 ：秒”
void timeToString(time_t t, char* pBuf)
{
	struct tm* pTimeInfo;
	pTimeInfo = localtime(&t);
	strftime(pBuf, 20, "%Y-%m-%d %H:%M:%S", pTimeInfo);
}