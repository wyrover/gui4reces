﻿//ExtractTab.h

//`~^`~^`~^`~^`~^`~^`~^`~^`~^`~^`~^`~^`~^`~^`~^`~^`~^`~^`
//            gui4reces Ver.0.0.0.9 by x@rgs
//              under NYSL Version 0.9982
//
//`~^`~^`~^`~^`~^`~^`~^`~^`~^`~^`~^`~^`~^`~^`~^`~^`~^`~^`


#ifndef _EXTRACTTAB_H_6EEF17CF_37BD_45e1_922A_191031CC56B5
#define _EXTRACTTAB_H_6EEF17CF_37BD_45e1_922A_191031CC56B5


#include"TabBase.h"



class ExtractTab:public TabBase{
public:
	ExtractTab(std::vector<Config*>& config_list):
		TabBase(IDD_TAB_EXTRACT,config_list){}
	~ExtractTab(){}
private:
	//メッセージハンドラ
	bool onInitDialog(WPARAM wparam,LPARAM lparam);
	bool onCommand(WPARAM wparam,LPARAM lparam);
	bool onNotify(WPARAM wparam,LPARAM lparam);
public:
	void setCurrentSettings();
};

#endif //_EXTRACTTAB_H_6EEF17CF_37BD_45e1_922A_191031CC56B5