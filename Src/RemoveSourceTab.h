﻿//RemoveSourceTab.h

//`~^`~^`~^`~^`~^`~^`~^`~^`~^`~^`~^`~^`~^`~^`~^`~^`~^`~^`
//            gui4reces Ver.0.0.0.9 by x@rgs
//              under NYSL Version 0.9982
//
//`~^`~^`~^`~^`~^`~^`~^`~^`~^`~^`~^`~^`~^`~^`~^`~^`~^`~^`


#ifndef _REMOVESOURCETAB_H_40151108_2A1E_4b2b_8F38_54313098227F
#define _REMOVESOURCETAB_H_40151108_2A1E_4b2b_8F38_54313098227F


#include"TabBase.h"



class RemoveSourceTab:public TabBase{
public:
	RemoveSourceTab(std::vector<Config*>& config_list):
		TabBase(IDD_TAB_REMOVESOURCE,config_list){}
	~RemoveSourceTab(){}
private:
	//メッセージハンドラ
	bool onInitDialog(WPARAM wparam,LPARAM lparam);
	bool onCommand(WPARAM wparam,LPARAM lparam);
public:
	void setCurrentSettings();
};

#endif //_REMOVESOURCETAB_H_40151108_2A1E_4b2b_8F38_54313098227F