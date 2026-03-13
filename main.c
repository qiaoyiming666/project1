#define _CRT_SECURE_NO_WARNINGS
#include<stdio.h>
#include"menu.h"
int main(void)
{
	printf("欢迎进入计费管理系统\n");
	printf("\n");
	int nSelection = -1;

	do 
	{ 
		//输出菜单
		outputMenu();
		//接收用户输入的菜单项编号
		scanf("%d", &nSelection);

		//清除输入流缓存
		fflush(stdin);  
		//根据用户输入数字，输出选择的菜单信息
		switch (nSelection)
		{
		case 1:
		{
			add();
			break;
		}
		case 2:
		{
			query();
			break;
		}
		case 3:
		{
			printf("上机\n");
			break;
		}
		case 4:
		{
			printf("下机\n");
			break;
		}
		case 5:
		{
			printf("充值\n");
			break;
		}
		case 6:
		{
			printf("退费\n");
			break;
		}
		case 7:
		{
			printf("查询系统\n");
			break;
		}
		case 8:
		{
			printf("注销卡\n");
			break;
		}
		case 0:
		{
			exitApp();
			printf("退出\n");
			break;
		}
		default:
		{
			printf("输入的菜单序列号错误！\n");
			break;
		}
		}
	} while (nSelection != 0);
	return 0;
}
