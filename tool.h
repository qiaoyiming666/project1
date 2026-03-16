#ifndef TOOL_H
#define TOOL_H

#include<time.h>
//将time_t类型转化为字符串，再将字符串转化为“年-月-日 时 ：分 ：秒”
void timeToString(time_t t, char* pBuf);
//将“年-月-日 时 ：分 ：秒”转化为time_t
time_t stringToTime(const char* pTimeStr);

#endif