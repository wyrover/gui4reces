﻿//FilterTab.cpp
//除外フィルタタブ

//`~^`~^`~^`~^`~^`~^`~^`~^`~^`~^`~^`~^`~^`~^`~^`~^`~^`~^`
//            gui4reces Ver.0.0.0.9 by x@rgs
//              under NYSL Version 0.9982
//
//`~^`~^`~^`~^`~^`~^`~^`~^`~^`~^`~^`~^`~^`~^`~^`~^`~^`~^`

#include"StdAfx.h"
#include"FilterTab.h"


using namespace sslib;



namespace{
	const TCHAR* filter_type_list[]={
		_T("処理対象"),
		_T("処理対象外")
	};

	const TCHAR dtp_format[]=_T("yyyyMMddHHmm");

	struct ATTR_TABLE{
		const int id;
		const DWORD attr;
	}const attr_table[]={
		{IDC_CHECKBOX_FILTER_READONLY,FILE_ATTRIBUTE_READONLY},
		{IDC_CHECKBOX_FILTER_SYSTEM,FILE_ATTRIBUTE_SYSTEM},
		{IDC_CHECKBOX_FILTER_HIDDEN,FILE_ATTRIBUTE_HIDDEN},
		{IDC_CHECKBOX_FILTER_DIRECTORY,FILE_ATTRIBUTE_DIRECTORY}
	};

	struct SIZE_UNITS_TABLE{
		const int id;
		const TCHAR* units;
	}const size_units_table[]={
		{IDM_SIZE_B,_T("B")},
		{IDM_SIZE_KB,_T("KB")},
		{IDM_SIZE_MB,_T("MB")},
		{IDM_SIZE_GB,_T("GB")},
		{IDM_SIZE_TB,_T("TB")}
	};
}

bool FilterTab::longlong2SYSTEMTIME(SYSTEMTIME* result_st,const long long date_time){
	if(result_st&&date_time>str::min_date&&date_time<str::max_date){
		result_st->wYear=static_cast<WORD>(date_time/10000000000);
		result_st->wMonth=static_cast<WORD>((date_time/100000000)%100);
		result_st->wDay=static_cast<WORD>((date_time/1000000)%100);
		result_st->wHour=static_cast<WORD>((date_time/10000)%100);
		result_st->wMinute=static_cast<WORD>((date_time/100)%100);
		result_st->wSecond=static_cast<WORD>(date_time%100);

		if(!result_st->wMonth)result_st->wMonth=1;
		if(!result_st->wDay)result_st->wDay=1;
		return true;
	}
	return false;
}

long long FilterTab::SYSTEMTIME2longlong(const SYSTEMTIME st){
	long long result=0;

	result=st.wYear*10000000000;
	result+=st.wMonth*100000000;
	result+=st.wDay*1000000;
	result+=st.wHour*10000;
	result+=st.wMinute*100;
	result+=st.wSecond;
	return result;
}

bool FilterTab::onInitDialog(WPARAM wparam,LPARAM lparam){
	::SetWindowText(getDlgItem(IDC_BUTTON_FILTER),filter_type_list[m_filter_type]);

	//日付のフォーマットを変更
	sendItemMessage(IDC_DATETIMEPICKER_FILTER_FROM,
					   DTM_SETFORMAT,
					   0,
					   (LPARAM)dtp_format);
	sendItemMessage(IDC_DATETIMEPICKER_FILTER_TO,
					   DTM_SETFORMAT,
					   0,
					   (LPARAM)dtp_format);

	//サイズの文字数を制限
	sendItemMessage(IDC_EDIT_FILTER_SIZE_FROM,
					   EM_SETLIMITTEXT,
					   (WPARAM)20*sizeof(TCHAR),
					   0);
	sendItemMessage(IDC_EDIT_FILTER_SIZE_TO,
					   EM_SETLIMITTEXT,
					   (WPARAM)20*sizeof(TCHAR),
					   0);

	//サイズメニューを読み込む
	m_size_menu=::LoadMenu(inst(),MAKEINTRESOURCE(IDR_MENU_FILTER_SIZE));
	m_size_sub_menu=::GetSubMenu(m_size_menu,0);

	setCurrentSettings();
	return true;
}

bool FilterTab::onDestroy(){
	::DestroyMenu(m_size_menu);
	return true;
}

bool FilterTab::onCommand(WPARAM wparam,LPARAM lparam){
	switch(LOWORD(wparam)){
		case IDC_BUTTON_FILTER:
			//フィルタの種類
			m_filter_type^=1;
			::SetWindowText(getDlgItem(IDC_BUTTON_FILTER),filter_type_list[m_filter_type]);
			m_filter_ptr=(include_filter())?
				&m_config_list[0]->cfg().general.filefilter:
				&m_config_list[0]->cfg().general.file_ex_filter;
			setCurrentSettings();
			break;

		case IDC_CHECKBOX_FILTER_STRING:
			//文字列
			::EnableWindow(getDlgItem(IDC_EDIT_FILTER_STRING),sendItemMessage(LOWORD(wparam),BM_GETCHECK,0,0));

			if(sendItemMessage(LOWORD(wparam),BM_GETCHECK,0,0)){
				sendMessage(WM_COMMAND,MAKEWPARAM(IDC_EDIT_FILTER_STRING,EN_CHANGE),0);
			}else{
				m_filter_ptr->pattern_list.clear();
			}
			break;

		case IDC_CHECKBOX_FILTER_ATTRIBUTE:
			//属性
			::EnableWindow(getDlgItem(IDC_CHECKBOX_FILTER_READONLY),sendItemMessage(LOWORD(wparam),BM_GETCHECK,0,0));
			::EnableWindow(getDlgItem(IDC_CHECKBOX_FILTER_SYSTEM),sendItemMessage(LOWORD(wparam),BM_GETCHECK,0,0));
			::EnableWindow(getDlgItem(IDC_CHECKBOX_FILTER_HIDDEN),sendItemMessage(LOWORD(wparam),BM_GETCHECK,0,0));
			::EnableWindow(getDlgItem(IDC_CHECKBOX_FILTER_DIRECTORY),sendItemMessage(LOWORD(wparam),BM_GETCHECK,0,0));
			::EnableWindow(getDlgItem(IDC_CHECKBOX_FILTER_EMPTYDIR),sendItemMessage(LOWORD(wparam),BM_GETCHECK,0,0));

			if(!sendItemMessage(LOWORD(wparam),BM_GETCHECK,0,0)){
				m_filter_ptr->attr=
					(include_filter())?
						m_def_filefilter.attr:
						m_def_file_ex_filter.attr;
			}
			break;

		case IDC_CHECKBOX_FILTER_READONLY:
		case IDC_CHECKBOX_FILTER_SYSTEM:
		case IDC_CHECKBOX_FILTER_HIDDEN:
		case IDC_CHECKBOX_FILTER_DIRECTORY:
			//各属性ボタン
			for(int i=0;attr_table[i].id!=0;i++){
				if(LOWORD(wparam)==attr_table[i].id){
					if(sendItemMessage(attr_table[i].id,BM_GETCHECK,0,0)){
						m_filter_ptr->attr|=attr_table[i].attr;
					}else{
						m_filter_ptr->attr&=~attr_table[i].attr;
					}
					break;
				}
			}
			break;

		case IDC_CHECKBOX_FILTER_EMPTYDIR:
			//空ディレクトリ
			if(!include_filter()){
				m_config_list[0]->cfg().general.file_ex_filter.include_empty_dir=!sendItemMessage(LOWORD(wparam),BM_GETCHECK,0,0);
			}
			break;

		case IDC_CHECKBOX_FILTER_DATE_FROM:
			//日付
			::EnableWindow(getDlgItem(IDC_DATETIMEPICKER_FILTER_FROM),
						   sendItemMessage(LOWORD(wparam),BM_GETCHECK,0,0));

			if(sendItemMessage(LOWORD(wparam),BM_GETCHECK,0,0)){
				SYSTEMTIME st={};
				long long date_time;

				sendItemMessage(IDC_DATETIMEPICKER_FILTER_FROM,DTM_GETSYSTEMTIME,(WPARAM)NULL,(LPARAM)&st);
				date_time=SYSTEMTIME2longlong(st);

				if(date_time>=str::min_date&&date_time<=str::max_date){
					m_filter_ptr->oldest_date=date_time;
				}
			}else{
				m_filter_ptr->oldest_date=
					(include_filter())?
						m_def_filefilter.oldest_date:
						m_def_file_ex_filter.oldest_date;
			}
			break;

		case IDC_CHECKBOX_FILTER_DATE_TO:
			//日付
			::EnableWindow(getDlgItem(IDC_DATETIMEPICKER_FILTER_TO),
						   sendItemMessage(LOWORD(wparam),BM_GETCHECK,0,0));

			if(sendItemMessage(LOWORD(wparam),BM_GETCHECK,0,0)){
				SYSTEMTIME st={};
				long long date_time;

				sendItemMessage(IDC_DATETIMEPICKER_FILTER_TO,DTM_GETSYSTEMTIME,(WPARAM)NULL,(LPARAM)&st);
				date_time=SYSTEMTIME2longlong(st);

				if(date_time>=str::min_date&&date_time<=str::max_date){
					m_filter_ptr->newest_date=date_time;
				}
			}else{
				m_filter_ptr->newest_date=
					(include_filter())?
						m_def_filefilter.newest_date:
						m_def_file_ex_filter.newest_date;
			}
			break;

		case IDC_CHECKBOX_FILTER_SIZE_FROM:
			//サイズ
			::EnableWindow(getDlgItem(IDC_EDIT_FILTER_SIZE_FROM),
						   sendItemMessage(LOWORD(wparam),BM_GETCHECK,0,0));
			::EnableWindow(getDlgItem(IDC_BUTTON_FILTER_SIZE_FROM),
						   sendItemMessage(LOWORD(wparam),BM_GETCHECK,0,0));

			if(sendItemMessage(LOWORD(wparam),BM_GETCHECK,0,0)){
				sendMessage(WM_COMMAND,MAKEWPARAM(IDC_EDIT_FILTER_SIZE_FROM,EN_CHANGE),0);
			}else{
				m_filter_ptr->min_size=
				(include_filter())?
					m_def_filefilter.min_size:
					m_def_file_ex_filter.min_size;
			}
			break;

		case IDC_CHECKBOX_FILTER_SIZE_TO:
			//サイズ
			::EnableWindow(getDlgItem(IDC_EDIT_FILTER_SIZE_TO),
						   sendItemMessage(LOWORD(wparam),BM_GETCHECK,0,0));
			::EnableWindow(getDlgItem(IDC_BUTTON_FILTER_SIZE_TO),
						   sendItemMessage(LOWORD(wparam),BM_GETCHECK,0,0));

			if(sendItemMessage(LOWORD(wparam),BM_GETCHECK,0,0)){
				sendMessage(WM_COMMAND,MAKEWPARAM(IDC_EDIT_FILTER_SIZE_TO,EN_CHANGE),0);
			}else{
				m_filter_ptr->max_size=
					(include_filter())?
						m_def_filefilter.max_size:
						m_def_file_ex_filter.max_size;
			}
			break;

		case IDC_BUTTON_FILTER_SIZE_FROM:
		case IDC_BUTTON_FILTER_SIZE_TO:{
			//サイズボタン
			RECT rc={0};

			::GetWindowRect(getDlgItem(LOWORD(wparam)),&rc);
			if(int id=::TrackPopupMenu(m_size_sub_menu,
									   TPM_LEFTALIGN|TPM_LEFTBUTTON|TPM_VERTICAL|TPM_RETURNCMD,
									   rc.left,
									   rc.bottom,
									   0,
									   handle(),
									   NULL)){

				size_t i=0;

				for(;i<ARRAY_SIZEOF(size_units_table);i++){
					if(size_units_table[i].id==id){
						::SetWindowText(getDlgItem(LOWORD(wparam)),size_units_table[i].units);
						break;
					}
				}


				std::vector<TCHAR> buffer(64);

				::GetWindowText(getDlgItem((LOWORD(wparam)==IDC_BUTTON_FILTER_SIZE_FROM)?IDC_EDIT_FILTER_SIZE_FROM:IDC_EDIT_FILTER_SIZE_TO),
								&buffer[0],
								buffer.size());
				if(buffer.size()){
					((LOWORD(wparam)==IDC_BUTTON_FILTER_SIZE_FROM)?
					 m_filter_ptr->min_size:
					 m_filter_ptr->max_size)=
						str::filesize2longlong((tstring(&buffer[0])+size_units_table[i].units).c_str());
				}
			}
			break;
		}

		default:
			break;
	}

	switch(HIWORD(wparam)){
		case EN_CHANGE:
			switch(LOWORD(wparam)){
				case IDC_EDIT_FILTER_STRING:{
					//文字列
					std::vector<TCHAR> buffer(1024);

					::GetWindowText(getDlgItem(LOWORD(wparam)),&buffer[0],buffer.size());
					if(!tstring(&buffer[0]).empty()){
						m_filter_ptr->pattern_list.clear();
						//';'で分割
						str::splitString(&m_filter_ptr->pattern_list,&buffer[0],';');
						//重複を削除
						m_filter_ptr->pattern_list.sort();
						m_filter_ptr->pattern_list.unique();
					}
					break;
				}

				case IDC_EDIT_FILTER_SIZE_FROM:
				case IDC_EDIT_FILTER_SIZE_TO:{
					//サイズ
					std::vector<TCHAR> size(64),unit(12);

					::GetWindowText(getDlgItem(LOWORD(wparam)),
									&size[0],
									size.size());
					::GetWindowText(getDlgItem((LOWORD(wparam)==IDC_EDIT_FILTER_SIZE_FROM)?IDC_BUTTON_FILTER_SIZE_FROM:IDC_BUTTON_FILTER_SIZE_TO),
									&unit[0],
									unit.size());

					if(size.size()){
						((LOWORD(wparam)==IDC_EDIT_FILTER_SIZE_FROM)?
						 m_filter_ptr->min_size:
						 m_filter_ptr->max_size)=
							str::filesize2longlong((tstring(&size[0])+tstring(&unit[0])).c_str());
					}
					break;
				}

				default:
					break;
			}
	}
	return false;
}

bool FilterTab::onNotify(WPARAM wparam,LPARAM lparam){
	LPNMDATETIMECHANGE dtc=reinterpret_cast<LPNMDATETIMECHANGE>(lparam);

	if(dtc->nmhdr.code==DTN_DATETIMECHANGE){
		if(dtc->nmhdr.hwndFrom==getDlgItem(IDC_DATETIMEPICKER_FILTER_FROM)){
			sendMessage(WM_COMMAND,MAKEWPARAM(IDC_CHECKBOX_FILTER_DATE_FROM,0),0);
			return true;
		}else if(dtc->nmhdr.hwndFrom==getDlgItem(IDC_DATETIMEPICKER_FILTER_TO)){
			sendMessage(WM_COMMAND,MAKEWPARAM(IDC_CHECKBOX_FILTER_DATE_TO,0),0);
			return true;
		}
	}
	return false;
}

void FilterTab::setCurrentSettings(){
	//文字列
	tstring pattern;

	for(std::list<tstring>::iterator ite=m_filter_ptr->pattern_list.begin(),
		end=m_filter_ptr->pattern_list.end();
		ite!=end;
		++ite){
		if(ite!=m_filter_ptr->pattern_list.begin()){
			pattern+=_T(";");
		}
		pattern+=*ite;
	}

	::SetWindowText(getDlgItem(IDC_EDIT_FILTER_STRING),pattern.c_str());
	setCheck(IDC_CHECKBOX_FILTER_STRING,!pattern.empty());
	::EnableWindow(getDlgItem(IDC_EDIT_FILTER_STRING),sendItemMessage(IDC_CHECKBOX_FILTER_STRING,BM_GETCHECK,0,0));

	//属性
	setCheck(IDC_CHECKBOX_FILTER_READONLY,m_filter_ptr->attr&FILE_ATTRIBUTE_READONLY);
	setCheck(IDC_CHECKBOX_FILTER_SYSTEM,m_filter_ptr->attr&FILE_ATTRIBUTE_SYSTEM);
	setCheck(IDC_CHECKBOX_FILTER_HIDDEN,m_filter_ptr->attr&FILE_ATTRIBUTE_HIDDEN);
	setCheck(IDC_CHECKBOX_FILTER_DIRECTORY,m_filter_ptr->attr&FILE_ATTRIBUTE_DIRECTORY);
	if(m_filter_ptr->attr!=((include_filter())?m_def_filefilter:m_def_file_ex_filter).attr||
	   (!include_filter()&&
	   m_config_list[0]->cfg().general.file_ex_filter.include_empty_dir!=
	   m_def_file_ex_filter.include_empty_dir)){
		setCheck(IDC_CHECKBOX_FILTER_ATTRIBUTE,true);
	}else{
		setCheck(IDC_CHECKBOX_FILTER_ATTRIBUTE,false);
	}
	::EnableWindow(getDlgItem(IDC_CHECKBOX_FILTER_READONLY),sendItemMessage(IDC_CHECKBOX_FILTER_ATTRIBUTE,BM_GETCHECK,0,0));
	::EnableWindow(getDlgItem(IDC_CHECKBOX_FILTER_SYSTEM),sendItemMessage(IDC_CHECKBOX_FILTER_ATTRIBUTE,BM_GETCHECK,0,0));
	::EnableWindow(getDlgItem(IDC_CHECKBOX_FILTER_HIDDEN),sendItemMessage(IDC_CHECKBOX_FILTER_ATTRIBUTE,BM_GETCHECK,0,0));
	::EnableWindow(getDlgItem(IDC_CHECKBOX_FILTER_DIRECTORY),sendItemMessage(IDC_CHECKBOX_FILTER_ATTRIBUTE,BM_GETCHECK,0,0));
	::EnableWindow(getDlgItem(IDC_CHECKBOX_FILTER_EMPTYDIR),sendItemMessage(IDC_CHECKBOX_FILTER_ATTRIBUTE,BM_GETCHECK,0,0));
	::ShowWindow(getDlgItem(IDC_CHECKBOX_FILTER_EMPTYDIR),!include_filter());

	//日付
	SYSTEMTIME st={};

	if(m_filter_ptr->oldest_date!=((include_filter())?m_def_filefilter:m_def_file_ex_filter).oldest_date){
		longlong2SYSTEMTIME(&st,m_filter_ptr->oldest_date);
		setCheck(IDC_CHECKBOX_FILTER_DATE_FROM,true);
	}else{
		::GetLocalTime(&st);
		setCheck(IDC_CHECKBOX_FILTER_DATE_FROM,false);
	}
	sendItemMessage(IDC_DATETIMEPICKER_FILTER_FROM,DTM_SETSYSTEMTIME,(WPARAM)GDT_VALID,(LPARAM)&st);
	::EnableWindow(getDlgItem(IDC_DATETIMEPICKER_FILTER_FROM),sendItemMessage(IDC_CHECKBOX_FILTER_DATE_FROM,BM_GETCHECK,0,0));

	if(m_filter_ptr->newest_date!=((include_filter())?m_def_filefilter:m_def_file_ex_filter).newest_date){
		longlong2SYSTEMTIME(&st,m_filter_ptr->newest_date);
		setCheck(IDC_CHECKBOX_FILTER_DATE_TO,true);
	}else{
		::GetLocalTime(&st);
		setCheck(IDC_CHECKBOX_FILTER_DATE_TO,false);
	}
	SendDlgItemMessage(handle(),IDC_DATETIMEPICKER_FILTER_TO,DTM_SETSYSTEMTIME,(WPARAM)GDT_VALID,(LPARAM)&st);
	::EnableWindow(getDlgItem(IDC_DATETIMEPICKER_FILTER_TO),sendItemMessage(IDC_CHECKBOX_FILTER_DATE_TO,BM_GETCHECK,0,0));

	//サイズ
	if(m_filter_ptr->min_size!=((include_filter())?m_def_filefilter:m_def_file_ex_filter).min_size){
		tstring size_str(str::longlong2filesize(m_filter_ptr->min_size));
		size_t size_index=0;

		while(size_index<size_str.length()&&
			  !isalpha(size_str.c_str()[++size_index]));

		tstring size_unit(str::toUpper(size_str.substr(size_index)));
		size_str.assign(size_str.substr(0,size_index));

		for(size_t i=0;i<ARRAY_SIZEOF(size_units_table);i++){
			if(str::isEqualStringIgnoreCase(size_units_table[i].units,size_unit)){
				//順番注意
				::SetWindowText(getDlgItem(IDC_BUTTON_FILTER_SIZE_FROM),size_unit.c_str());
				::SetWindowText(getDlgItem(IDC_EDIT_FILTER_SIZE_FROM),size_str.c_str());
				break;
			}
		}

		setCheck(IDC_CHECKBOX_FILTER_SIZE_FROM,true);
	}else{
		::SetWindowText(getDlgItem(IDC_BUTTON_FILTER_SIZE_FROM),_T("KB"));
		::SetWindowText(getDlgItem(IDC_EDIT_FILTER_SIZE_FROM),_T(""));
		setCheck(IDC_CHECKBOX_FILTER_SIZE_FROM,false);
	}
	::EnableWindow(getDlgItem(IDC_EDIT_FILTER_SIZE_FROM),sendItemMessage(IDC_CHECKBOX_FILTER_SIZE_FROM,BM_GETCHECK,0,0));
	::EnableWindow(getDlgItem(IDC_BUTTON_FILTER_SIZE_FROM),sendItemMessage(IDC_CHECKBOX_FILTER_SIZE_FROM,BM_GETCHECK,0,0));

	if(m_filter_ptr->max_size!=((include_filter())?m_def_filefilter:m_def_file_ex_filter).max_size){
		tstring size_str(str::longlong2filesize(m_filter_ptr->max_size));
		size_t size_index=0;

		while(size_index<size_str.length()&&
			  !isalpha(size_str.c_str()[++size_index]));

		tstring size_unit(str::toUpper(size_str.substr(size_index)));
		size_str.assign(size_str.substr(0,size_index));

		for(size_t i=0;i<ARRAY_SIZEOF(size_units_table);i++){
			if(str::isEqualStringIgnoreCase(size_units_table[i].units,size_unit)){
				//順番注意
				::SetWindowText(getDlgItem(IDC_BUTTON_FILTER_SIZE_TO),size_unit.c_str());
				::SetWindowText(getDlgItem(IDC_EDIT_FILTER_SIZE_TO),size_str.c_str());
				break;
			}
		}

		setCheck(IDC_CHECKBOX_FILTER_SIZE_TO,true);
	}else{
		::SetWindowText(getDlgItem(IDC_BUTTON_FILTER_SIZE_TO),_T("KB"));
		::SetWindowText(getDlgItem(IDC_EDIT_FILTER_SIZE_TO),_T(""));
		setCheck(IDC_CHECKBOX_FILTER_SIZE_TO,false);
	}
	::EnableWindow(getDlgItem(IDC_EDIT_FILTER_SIZE_TO),sendItemMessage(IDC_CHECKBOX_FILTER_SIZE_TO,BM_GETCHECK,0,0));
	::EnableWindow(getDlgItem(IDC_BUTTON_FILTER_SIZE_TO),sendItemMessage(IDC_CHECKBOX_FILTER_SIZE_TO,BM_GETCHECK,0,0));
}