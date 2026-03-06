#define _CRT_SECURE_NO_WARNINGS
#include<stdio.h>

int main(void)
{
	int nSelection;

	do 
	{
		printf("---------------菜单---------------\n");
		printf("1.添加卡\n");
		printf("2.查询卡\n");
		printf("3.上机\n");
		printf("4.下机\n");
		printf("5.充值\n");
		printf("6.退费\n");
		printf("7.查询系统\n");
		printf("8.注销卡\n");
		printf("0.退出\n");
		//提示选择菜单编号
		printf("请选择菜单编号（0~8）：");

		scanf("%d", &nSelection);//输入数字
		//根据用户输入数字，输出选择的菜单信息
		switch (nSelection)
		{
		case 1:
		{
			printf("添加卡\n");
			break;
		}
		case 2:
		{
			printf("查询卡\n");
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