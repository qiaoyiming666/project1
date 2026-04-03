#ifndef MONEY_FILE_H
#define MONEY_FILE_H

#include "model.h"

// 将充值/退费记录保存到文本文件（追加）
// 返回 TRUE 成功，FALSE 失败
int saveMoney(const Money* pMoney, const char* pPath);

#endif // !1
