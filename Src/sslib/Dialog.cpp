﻿//Dialog.cpp
//ダイアログウクラス


#include"../StdAfx.h"
#include"sslib.h"


namespace sslib{


INT_PTR CALLBACK messageRouter(HWND dlg_handle,UINT message,WPARAM wparam,LPARAM lparam){
	Dialog* dlg=NULL;

	if(message==WM_INITDIALOG){
		dlg=reinterpret_cast<Dialog*>(lparam);
	}else{
		//インスタンスのポインタを取り出す
		dlg=reinterpret_cast<Dialog*>(::GetWindowLongPtr(dlg_handle,DWLP_USER));
	}
	return (dlg!=NULL)?dlg->handleMessage(dlg_handle,message,wparam,lparam):FALSE;
}

//モーダルダイアログを表示
int Dialog::doModal(HWND parent_handle){
	setDlgType(MODAL);
	return ::DialogBoxParam(inst(),MAKEINTRESOURCE(id()),parent_handle,messageRouter,reinterpret_cast<LPARAM>(this));
}

//モードレスダイアログを表示
HWND Dialog::doModeless(HWND parent_handle,int cmd_show){
	setDlgType(MODELESS);
	HWND dlg_handle=::CreateDialogParam(inst(),MAKEINTRESOURCE(id()),parent_handle,messageRouter,reinterpret_cast<LPARAM>(this));
	showDialog(cmd_show);

	return dlg_handle;
}

//ダイアログを表示
bool Dialog::showDialog(int cmd_show){
	bool result=::ShowWindow(handle(),cmd_show)!=0;
//	::UpdateWindow(handle());
	return result;
}

//ダイアログを閉じる
bool Dialog::endDialog(UINT exit_code){
	setExitCode(exit_code);
	bool result=((type()==MODAL)?::EndDialog(handle(),exit_code):(::DestroyWindow(handle())))!=0;
	return result;
}

bool Dialog::onInitDialog(WPARAM wparam,LPARAM lparam){
	return true;
}

bool Dialog::onDestroy(){
	setHandle(NULL);
	::SetWindowLongPtr(handle(),DWLP_USER,0);
	if(type()!=MODAL)::PostQuitMessage(0);
	return true;
}

bool Dialog::onOk(){
	return true;
}

bool Dialog::onCancel(){
	return true;
}

bool Dialog::onCommand(WPARAM wparam,LPARAM lparam){
	return false;
}

bool Dialog::onSysCommand(WPARAM wparam,LPARAM lparam){
	return false;
}

bool Dialog::onNotify(WPARAM wparam,LPARAM lparam){
	return false;
}

bool Dialog::onPaint(){
	return false;
}

bool Dialog::onSize(WPARAM wparam,LPARAM lparam){
	return false;
}

bool Dialog::onMove(WPARAM wparam,LPARAM lparam){
	return false;
}

bool Dialog::onCtlColorStatic(WPARAM wparam,LPARAM lparam){
	//テキストの背景色を透過させる
	HDC dc=reinterpret_cast<HDC>(wparam);
	::SetBkMode(dc,TRANSPARENT);
	return true;
}

bool Dialog::onDropFiles(HDROP drop_handle){
	return false;
}

bool Dialog::onMessage(UINT message,WPARAM wparam,LPARAM lparam){
	return false;
}

LRESULT Dialog::sendMessage(UINT message,WPARAM wparam,LPARAM lparam){
	return ::SendMessage(handle(),message,wparam,lparam);
}

LRESULT Dialog::sendItemMessage(int item,UINT message,WPARAM wparam,LPARAM lparam){
	return ::SendDlgItemMessage(handle(),item,message,wparam,lparam);
}

bool Dialog::postMessage(UINT message,WPARAM wparam,LPARAM lparam){
	return ::PostMessage(handle(),message,wparam,lparam)!=0;
}

INT_PTR Dialog::handleMessage(HWND dlg_handle,UINT message,WPARAM wparam,LPARAM lparam){
	switch(message){
		case WM_INITDIALOG:{
			setHandle(dlg_handle);
			//lparamにインスタンスのポインタが代入されているので保存
			::SetWindowLongPtr(dlg_handle,DWLP_USER,lparam);
			return onInitDialog(wparam,lparam);
		}
		case WM_DESTROY:return onDestroy();
		case WM_COMMAND:{
			switch(LOWORD(wparam)){
				case IDOK:{
					if(onOk()){
						endDialog(IDOK);
					}
					return true;
				}
				case IDCANCEL:{
					if(onCancel()){
						endDialog(IDCANCEL);
					}
					return true;
				}
			}
			return onCommand(wparam,lparam);
		}
		case WM_SYSCOMMAND:return onSysCommand(wparam,lparam);
		case WM_NOTIFY:return onNotify(wparam,lparam);
		case WM_PAINT:return onPaint();
		case WM_SIZE:return onSize(wparam,lparam);
		case WM_MOVE:return onMove(wparam,lparam);
		case WM_DROPFILES:return onDropFiles(reinterpret_cast<HDROP>(wparam));
		default:return onMessage(message,wparam,lparam);
	}
}

//namespace sslib
}