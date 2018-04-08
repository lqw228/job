#include "StdAfx.h"
#include "TdxRowset.h"
//#include <vector>
//using namespace std;

TdxRowset::TdxRowset(void)
{
	FRowCount=0;
	FColCount=0;
	for (int i=0;i<100;i++)
		TitleID[100]=-1;
	TitleName=NULL;
	Rows=NULL;
}

TdxRowset::TdxRowset(CString RowStr)
{
	TdxRowset();
	Update(RowStr);
}

CString** fun(int iRow, int iCol)
{
	CString** ppData;
	ppData = new CString*[iRow];
	for(int i=0;i< iRow;++i) 
		ppData[i] = new CString[iCol]; 
	return ppData;
}

void TdxRowset::Update(CString RowStr)
{
	FRowCount=0;
	for (int i=0;i<100;i++)
		TitleID[i]=-1;
	FColCount=0;

	if (TitleName!=NULL)
		delete []TitleName;
	TitleName=NULL;
	if (FRowCount>0)
	{
		for(int j=0;j< FRowCount;j++)
		{
			delete[] Rows[j];
		}
		delete[] Rows; 
	}
	Rows=NULL;

	CStringArray sa;
	CStringArray sa1;
	int Count=String2Array(RowStr,sa,'\n');
	if (Count==0)
		return;
	CString str=sa.GetAt(0);
	FColCount=String2Array(str,sa1,'\t');
	if (FColCount==0)
		return;
	TitleName=new CString[FColCount];
	for (int j=0;j<FColCount;j++)
	{
		str=sa1.GetAt(j);
		int j1=str.Find('_');
		if (j1==-1)
			TitleName[j]=str;
		else
			TitleName[j]=str.Mid(j1+1);;

		TitleID[j]=atoi(str);
	}
	FRowCount=Count-1;
	if (FRowCount>0)
	{ 
		Rows=fun(FRowCount,FColCount);
		for (int i=1;i<Count;i++)
		{
			str=sa.GetAt(i);
			int cs=String2Array(str,sa1,'\t');
			for (int j=0;j<cs;j++)
				Rows[i-1][j]=sa1.GetAt(j);
		}
	}
}
TdxRowset::~TdxRowset(void)
{
	if (TitleName!=NULL)
		delete []TitleName;
	for(int j=0;j< FRowCount;j++)
		delete[] Rows[j];
	delete[] Rows; 
}
CString TdxRowset::GetCell(int x,int y)
{
	CString s1="";
	if ((y>-1) && (y<FRowCount) && (x>-1) && (x<FColCount))
		return Rows[y][x];
	return s1;
}
CString TdxRowset::GetID(int row,int ID)
{
	CString s1="";
	if ((row<0) || (row>=FRowCount))
		return s1;
	int id=-1;
	for (int i=0;i<FColCount;i++)
	{
		if (TitleID[i]==ID)
		{
			s1=Rows[row][i];
			break;
		}
	}
	return s1;
}
//void TdxRowset::RowToListCtrl(CListCtrl &lv)
//{
//    lv.DeleteAllItems();
//	CHeaderCtrl * pHeader = (CHeaderCtrl *)lv.GetHeaderCtrl(); 
//	int nCount = pHeader->GetItemCount();
//	for(int i=0;i<nCount;i++)
//		lv.DeleteColumn(0);
//
//	CString s1;
//	for (int i = 0; i <FColCount; i++)
//	{
//		s1.Format("%d-%s",TitleID[i],TitleName[i]);
//		lv.InsertColumn(i,s1, LVCFMT_LEFT, 120);//插入第一列标题
//	}
//	for (int j = 0; j < FRowCount; j++)
//		 lv.InsertItem(0 ,"");
//	for (int j = 0; j < FRowCount; j++)
//	{
//		for (int i = 0; i < FColCount; i++)
//		{
//			lv.SetItemText(j,i,Rows[j][i]);
//		}
//	}
//
//}
//
//void  TdxRowset::RowToListBox(CListBox &lb)
//{
//	if (FColCount <1)
//		return;
//	CString s1;
//	for (int i = 0; i <FColCount; i++)
//	{
//		s1.Format("%d ===>%d=%s",i,TitleID[i],TitleName[i]);
//		lb.AddString(s1);
//	}
//	if (FRowCount > 0)
//	{
//		for (int j = 0; j < FRowCount; j++)
//		{
//			s1.Format("记录:%d -------",j);
//			lb.AddString(s1);
//			lb.AddString("--------------------------------");
//			for (int i = 0; i < FColCount; i++)
//			{
//				s1.Format("%d ==>%s",i,Rows[j][i]);
//				lb.AddString(s1);
//			}
//		}
//		lb.AddString("=============================================================");
//	}
//}

int TdxRowset::String2Array(const CString& s, CStringArray &sa, char chSplitter)
{
	int nLen=s.GetLength(), nLastPos, nPos;
	bool bContinue;

	sa.RemoveAll();
	nLastPos=0;
	do
	{
		bContinue=false;	
		nPos = s.Find(chSplitter, nLastPos);
		if (-1!=nPos)
		{
			sa.Add(s.Mid(nLastPos, nPos-nLastPos));
			nLastPos=nPos+1;
			if (nLastPos != nLen) bContinue=true;
		}
	} while (bContinue);

	if (nLastPos != nLen)
		sa.Add(s.Mid(nLastPos, nLen-nLastPos));

	return (int)sa.GetSize();
}

