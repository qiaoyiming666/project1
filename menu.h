#ifndef MENU_H
#define MENU_H

int getSize(const char* pInfo);//获取字符长度

void outputMenu();//输出菜单  
void add();       //添加卡
void query();     //查询卡
void logon();     //上机
void logoff();    //下机
void addMoney(); //充值
void refundMoney();    //退费

void annul(); // 注销卡（新增）
void exitApp();     //退出
#endif