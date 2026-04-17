#define _CRT_SECURE_NO_WARNINGS
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include "global.h"
#include "tool.h"
//将time_t类型转化为字符串，再将字符串转化为“年-月-日 时 ：分 ：秒”
void timeToString(time_t t, char* pBuf)
{
	if (pBuf == NULL) return;
	pBuf[0] = '\0';
	if (t == 0) return;

	struct tm* pTimeInfo = localtime(&t);
	if (pTimeInfo == NULL) return;

	// TIMELENGTH 在 global.h 中定义为 20
	strftime(pBuf, TIMELENGTH, "%Y-%m-%d %H:%M:%S", pTimeInfo);
}

//将“年-月-日 时 ：分 ：秒”转化为time_t
time_t stringToTime(const char* pTimeStr)
{
	if (pTimeStr == NULL) return 0;

	struct tm timeInfo;
	memset(&timeInfo, 0, sizeof(timeInfo));
	timeInfo.tm_isdst = -1; // 让 mktime 决定夏令时

	// 支持 "YYYY-MM-DD" 或 "YYYY-MM-DD HH:MM:SS"
	int year = 0, mon = 0, mday = 0, hour = 0, minu = 0, sec = 0;
	int matched = sscanf(pTimeStr, "%d-%d-%d %d:%d:%d", &year, &mon, &mday, &hour, &minu, &sec);
	if (matched < 3)
	{
		// 解析失败
		return 0;
	}

	timeInfo.tm_year = year - 1900;
	timeInfo.tm_mon = mon - 1;
	timeInfo.tm_mday = mday;
	if (matched >= 6)
	{
		timeInfo.tm_hour = hour;
		timeInfo.tm_min = minu;
		timeInfo.tm_sec = sec;
	}
	else
	{
		// 只有日期部分，时间置为 00:00:00
		timeInfo.tm_hour = 0;
		timeInfo.tm_min = 0;
		timeInfo.tm_sec = 0;
	}

	time_t t = mktime(&timeInfo);
	if (t == (time_t)-1)
	{
		// mktime 失败，返回 0 作为无效时间
		return 0;
	}
	return t;
}

// 从 stdin 读取一行并修剪首尾空白，返回 1 成功，0 失败
int readLineTrim(char* buf, size_t size)
{
	if (buf == NULL || size == 0) return 0;
	if (fgets(buf, (int)size, stdin) == NULL) return 0;

	// 去掉末尾的换行和回车
	size_t len = strlen(buf);
	while (len > 0 && (buf[len - 1] == '\n' || buf[len - 1] == '\r'))
	{
		buf[--len] = '\0';
	}

	// 去掉前导空白
	size_t start = 0;
	while (buf[start] != '\0' && isspace((unsigned char)buf[start])) start++;

	if (start > 0)
	{
		size_t rem = strlen(buf + start);
		memmove(buf, buf + start, rem + 1); // 包括终止符
	}

	// 去掉尾部空白
	len = strlen(buf);
	while (len > 0 && isspace((unsigned char)buf[len - 1]))
	{
		buf[--len] = '\0';
	}

	return 1;
}