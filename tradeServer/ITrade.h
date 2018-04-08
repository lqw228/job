#pragma once

class ITrade
{
public:
	ITrade(void){};
	~ITrade(void){};

	//��ʼ��
	virtual int Init() = 0;
	//����
	virtual int Login(char *server,int port,char *Version,int AccountType,int ID,char *Account,char *JyPass,char *TxPass) = 0;
	//��ѯ
	virtual int QueryData(int type) = 0;
	//�µ�
	virtual int SendOrder(int TradeType,char *StkCode,double Price,int volume) = 0;
	//����
	virtual int CancelOrder(char* stockcode,char* orderid) =0;
	//�ǳ�
	virtual int Logout() = 0;
	//�˳�
	virtual int Exit() = 0;
    //��ȡ������Ϣ
	virtual char* GetError() = 0;
	//��ȡ���
	virtual char* GetResult() = 0;
};
