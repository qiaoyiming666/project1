#define _CRT_SECURE_NO_WARNINGS
#include<time.h>
#include<stdio.h>

//将time_t类型转化为字符串，再将字符串转化为“年-月-日 时 ：分 ：秒”
void timeToString(time_t t, char* pBuf)
{
	struct tm* pTimeInfo;
	pTimeInfo = localtime(&t);
	strftime(pBuf, 20, "%Y-%m-%d %H:%M:%S", pTimeInfo);
}

//
time_t stringToTime(const char* pTimeStr)
{
	struct tm timeInfo;
	time_t t;
	sscanf(pTimeStr, "%d-%d-%d %d:%d:%d",
		&timeInfo.tm_year,
		&timeInfo.tm_mon,
		&timeInfo.tm_mday,
		&timeInfo.tm_hour,
		&timeInfo.tm_min,
		&timeInfo.tm_sec);
	timeInfo.tm_year -= 1900; // tm_year是从1900年开始的
	timeInfo.tm_mon -= 1;     // tm_mon是从0开始的
	t = mktime(&timeInfo);
	return t;
} 