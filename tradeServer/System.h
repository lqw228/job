#pragma once

#include "ITrade.h"
#include <afx.h>

#include <json/json.h>
#include <algorithm>
#include <sstream>

class TradeSystem
{
public:
	TradeSystem(void);
	~TradeSystem(void);

	//��ʼ��
	bool init(void);
    //�˳�
	bool Uninit(void);
    //����
	int start(void);
    //ֹͣ
	int stop(void);

	//���׽��ս���
	void tradeParse(string& text, int funid);
	//��ѯ���ճɽ�
	int QryDeal();
	//��ѯ����ί��
	int QryEntrust();
	//��ѯ�ֲ��б�
	void QryHold(bool bQueue = true);
    //������������¼
	void checkDay();
    //�����ɽ����
	void parseDeal(char *Result);
	//����ί�н��
	void parseEntrust(char *Result);
	//��ѯ�ʽ�
	void QryMoney(bool bQueue = true);
	
protected:
	//���֤ȯί�д���
	void DoEntrustGuangDa(CString& con, CString& num, CString& price, CString& status, CString& numDeal);
    //����ͨ��ί��
	void DoEntrust(CString& con, CString& num, CString& price, CString& status, CString& numDeal);
    //����ͨ�óɽ�
	void DoDeal(CString& con, CString& deal, CString& num, CString& price);

	//�µ�
	void InsertOrder(Json::Value& root);
	//����
	int OrderDelete(Json::Value& root);
	//���ͳɽ����ƻر�
	void SendDeal(CString con,CString assets, CString marketValue,CString balance);
	//���ͳ������ƻر�
	void SendCancel(CString con,CString assets, CString marketValue,CString balance);

	//����ί�г�����ʽ
	void DoEntrustDongWu(CString& con, CString& num, CString& price, CString& status, CString& numDeal);
	//���̳���
	void DoEntrustZhaoShang(CString& con, CString& num, CString& price, CString& status, CString& numDeal);
private:
	ITrade* m_pTrade;
	bool m_bInit;
	bool m_bStart;
};
