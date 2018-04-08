#pragma once
#include "afxwin.h"
//#include <vector>
//using namespace std;
typedef   CArray<CStringArray*,CStringArray*>   CMyArray;   

class TdxRowset
{
public:
	TdxRowset(void);
	TdxRowset(CString RowStr);
	virtual ~TdxRowset(void);
	void Update(CString RowStr);
    int String2Array(const CString& s, CStringArray &sa, char chSplitter);
	CString GetID(int row,int ID);
	CString GetCell(int x,int y);
	//void RowToListBox(CListBox &lb);
	//void RowToListCtrl(CListCtrl &lv);

public:
	int FRowCount;
	int FColCount;
	int TitleID[100];
	CString *TitleName;
    CString** Rows;
};
