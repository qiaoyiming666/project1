#ifndef TOOL_H
#define TOOL_H

#include<time.h>
#include<stddef.h>

//将time_t类型转化为字符串，再将字符串转化为“年-月-日 时 ：分 ：秒”
void timeToString(time_t t, char* pBuf);
//将“年-月-日 时 ：分 ：秒”转化为time_t
time_t stringToTime(const char* pTimeStr);

// 从 stdin 读取一行，去除首尾空白（包括换行），将结果写入 buf（包含终止 '\0'）
// 返回 1 表示成功读取并处理，0 表示失败或 EOF
int readLineTrim(char* buf, size_t size);

#endif