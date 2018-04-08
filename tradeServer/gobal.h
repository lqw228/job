#pragma once

// ����
enum eFunctionID
{

	eFunctionID_OrderInsert = 0,          // ��������
	eFunctionID_OrderDelete = 1,          // ��������
	eFunctionID_MoneyQry    = 2,          // �˻��ʽ��ѯ
	eFunctionID_HoldQry     = 3,          //�ֲֲ�ѯ
	eFunctionID_OrderQry,             // ������ѯ
	eFunctionID_OrderStateAutoPush,   // ����״̬����
	eFunctionID_OrderInfoAutoPush,    // ������Ϣ����
	eFunctionID_MatchQry,             // �ɽ���ѯ
	eFunctionID_MatchStateAutoPush,   // �ɽ�״̬����
	eFunctionID_MatchInfoAutoPush,    // �ɽ���Ϣ����
	
	eFunctionID_MoneyChgAutoPush,     // �˻��ʽ�仯����
	eFunctionID_OrderModity,          // �����ĵ�
	//eFunctionID_Begin,
	eFunctionID_Login,            // ��¼
	eFunctionID_MoneyQry_End,         // �˻��ʽ��ѯ����
	eFunctionID_OrderQry_End,	

	eFunctionID_End = 99
};

//��ѯ����, 0�ʽ�",1"�ֲֹɷ�",2"����ί��",3"���ճɽ�",4"�����б�",5"�ɶ�����"
enum 
{
	eQryType_Fund = 0,
	eQryType_HoldPosition = 1,	
	eQryType_Entrust = 2,
	eQryType_Deal = 3,
	eQryType_Cancel = 4,
	eQryType_Stockholder = 5,
};

//ȯ�̻���ID
enum
{
	eTradeID_WUKUANG = 1, //���֤ȯ
	eTradeID_GUANGFA = 4, //�㷢֤ȯ
	eTradeID_CAITONG = 5, //��֤ͨȯ
	eTradeID_DONGBEI = 8, //����֤ȯ
	eTradeID_FANGZHENG = 9, //����֤ȯ
	eTradeID_PINGAN  = 106, //ƽ��֤ȯ (�ʽ����������⴦��)	
	eTradeID_GUOYUAN = 108, //��Ԫ֤ȯ
	eTradeID_HENGTAI = 109, //��̩֤ȯ
	eTradeID_CHANGCHENG = 110, //����֤ȯ
	eTradeID_CUOTAIJUNAN = 111, //��̩����
	eTradeID_DONGWU = 112, //����֤ȯ
	eTradeID_ZHAOSHANG = 113, //����֤ȯ
	eTradeID_DIYICHUANGYE = 114, //��һ��ҵ֤ȯ
	eTradeID_GUANGDA = 115, //���֤ȯ
	eTradeID_DONGWAN = 116, //��ݸ֤ȯ
	eTradeID_ZHONGYUAN = 117, //��ԭ֤ȯ
	eTradeID_XIANGCAI = 118, //���֤ȯ
	eTradeID_ZHONGXIN = 119, //����֤ȯ
	eTradeID_XINGYE = 120, //��ҵ֤ȯ
	eTradeID_HUAFU = 121, //����֤ȯ
	eTradeID_MINSHENG = 122, //����֤ȯ
	eTradeID_ZHONGTIAN = 123, //����֤ȯ
	eTradeID_XINDA = 124,     //�Ŵ�֤ȯ (�ʽ����������)
	
};

enum
{
	eTradeDeal_OTHER = -1,
	eTradeDeal_COMMON = 0,//�ɽ�ͨ�ô���
	eTradeDeal_ONE = 1,   //�ɽ���¼�а���������¼
};

enum
{
	eTradeEntrust_OTHER = -1,
	eTradeEntrust_COMMON = 0,//ί��ͨ�ô���
};

enum
{
	eTradeHold_OTHER = -1,
	eTradeHold_COMMON = 0,//�ֲ�ͨ�ô���
};

