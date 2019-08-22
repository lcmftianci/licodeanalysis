// stdafx.h : 标准系统包含文件的包含文件，
// 或是经常使用但不常更改的
// 特定于项目的包含文件
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             //  从 Windows 头文件中排除极少使用的信息
// Windows 头文件: 
#include <windows.h>

/*
* 当libjpeg-turbo为vs2010编译时，vs2015下静态链接libjpeg-turbo会链接出错:找不到__iob_func,
* 增加__iob_func到__acrt_iob_func的转换函数解决此问题,
* 当libjpeg-turbo用vs2015编译时，不需要此补丁文件
*/
//#if _MSC_VER>=1900
//#include "stdio.h" 
//_ACRTIMP_ALT FILE* __cdecl __acrt_iob_func(unsigned);
//#ifdef __cplusplus 
//extern "C"
//#endif 
//FILE* __cdecl __iob_func(unsigned i) {
//	return __acrt_iob_func(i);
//}
//#endif /* _MSC_VER>=1900 */

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS      // 某些 CString 构造函数将是显式的

#include <atlbase.h>
#include <atlstr.h>

// TODO:  在此处引用程序需要的其他头文件
#include <crtdbg.h>
