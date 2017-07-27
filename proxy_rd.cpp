// proxy_rd.cpp : Defines the entry point for the application.
//
// TODO: Удаленное обновление конфигурации (GET|POST) http://127.0.0.1/script/
// TODO: Полное шифрование трафика между proxy_rd и proxy_rd
// TODO: Шифрование только данных между proxy_rd и web-script
// TODO: Прозрачная пересылка данных с поддержкой шифрования
// TODO: Сделать правильную обработку при завершении работы
// TODO: Обеспечить корректный выход из функций при ошибках
//
// Предусмотреть схемы проксирования:
//   1. client - proxy_rd (модификация + шифрование полное)
//      - proxy_rd (дешифрование) - server
//
//   2. client - proxy_rd (модификация + шифрование данных)
//      - proxy_rd (пересылка) - web-script
//
//   3. client - proxy_rd (модификация + шифрование данных)
//      - web-script
//
// Схема 2 и 3 реализуется спомощью двойного заголовка.
// Первый заголовок CONNECT host:port, второй - настоящий.
// В скрипте делаем options &= ~PXO_MDCONNECT, чтобы не
// отправлять ответ на фиктивный заголовок CONNECT.
//
// Методы обработки:
//   1. Жесткий транзит - когда адрес пересылки жёстко задан
//      в скрипте.
//   2. Транзит - когда адрес пересылки берётся из HTTP
//      заголовка.
//
// Обработка URI: Путь разбить на части (разделитель слеш).
//   Составить дерево db_tree_node состоящее из названия ветки,
//   которое соответствует части пути и соответствующей этой
//   части функции. Найти конечную точку пути в дереве и вызвать
//   соответствующую этой точки функцию.
//   Пример пути: /settings/script/
//

#include "stdafx.h"
#include "autobuild.h"
#include "resource.h"
#include <windowsx.h>
#include <shellapi.h>
#include <shlobj.h>
//#include <commdlg.h>
#include <commctrl.h>
//#include <mmsystem.h>
#include <stdlib.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include "../zinc/base64.h"
#include "../zinc/zini.h"
#include "../zinc/utstrings.h"
#include "../zinc/vector.h"
#include "../zinc/zalloc.h"
#include "../zinc/zcontrols.h"
#include "../zinc/zdbg.h"
#include "../zinc/zfiles.h"
#include "../zinc/zwindows.h"
#include "../zinc/zpcre.h"
#include "../zinc/zlist.h"
#include "../zinc/zlc/zl.h"
#include "../zinc/zlc/zl_parser.h"
#include "../zinc/zlog.h"

#pragma comment (lib, "shell32.lib")
#pragma comment (lib, "ws2_32.lib")


#define PX_GUI

#define proxy_ini				"proxy_rd.ini"
#define proxy_log				"proxy_rd.log"
#define MAX_FILE_SIZE_KB		2048			// proxy_log

//#define PX_PORT					8080
#define PX_LISTENPORT			"0.0.0.0:3128"
#define PX_REDIR_PORT			80
//#define PX_REDIR_HOST			"localhost[:port]"
#define PX_TIMEOUT				90

//#define PX_ERROR_ADDR			"127.0.0.1"
#define PX_REDIR_HEADER			"import __cdecl msg(2);\r\n"\
								"\r\n"\
								"function main()\r\n"\
								"{\r\n"\
								"	msg(\"proxy_rd: warning\", \"please configure me!\");\r\n"\
								"}\r\n"\
								"\r\n"\
								"function proxy_rd(\r\n"\
								"	unsigned long *header_len, char **post_data, unsigned long *post_data_len, char **footer, unsigned long *footer_len,\r\n"\
								"	unsigned long *options, char **rdhost, char **rdport,\r\n"\
								"	char *method, char *protocol, char *uri_full, char *uri_relative, char *uri_full_encoded,\r\n"\
								"	char *host, char *build, char *h_host_port, char *h_unknown, char *h_cookie, char *h_accept,\r\n"\
								"	char *h_accept_lang, char *h_accept_charset, char *h_if_mod_since, char *h_if_none_match,\r\n"\
								"	char *h_referer, char *h_user_agent, char *h_proxy_connection, char *h_keep_alive,\r\n"\
								"	char *h_authorization, char *h_cache_control, char *h_pragma, char *h_range, char *h_content_type,\r\n"\
								"	char *h_content_length, char *port, char *query, char *scheme, char *authority, char *path, char *fragment\r\n"\
								")\r\n"\
								"{\r\n"\
								"	char *header;\r\n"\
								"	header = NULL;\r\n"\
								"\r\n"\
								"	return header;\r\n"\
								"}\r\n"\
								"\r\n"
//#define PX_REDIR_HEADER			"GET /redir_nx.php?id=%uriencoded%&md=%method% %protocol%\r\n"\
//								"Host: %rdhost%\r\n"\
//								"%unknown%\r\n"

//#define PX_REDIR_HEADER		"%method% %path%%if%query%{?%query%} %protocol%\r\n"\
//								"Host: %rdhost%%if%port%{:%port%}\r\n"\
//								"%if%accept%{Accept: %accept%\r\n}"\
//								"%unknown%\r\n"
#define PX_CONNECT_HEADER		"%protocol% 200 Connection established\r\n"\
								"Proxy-agent: proxy_rd (build "__BUILD_STR__")\r\n\r\n"

#define PXO_HIDDEN				0x00000001
#define PXO_INSTALL				0x00000002
#define PXO_MDCONNECT			0x00000004
#define PXO_DEBUG				0x00000008
#define PXO_MULTIINSTANCE		0x00000010
#define PXO_ASSECONDHDR			0x00000020
#define PXO_SKIPHEADER			0x00000040
#define PXO_TRANSITE			0x00000080

#define PXO_ENCRYPTIN			0x00000100
#define PXO_ENCRYPTOUT			0x00000200
#define PXO_DECRYPTIN			0x00000400
#define PXO_DECRYPTOUT			0x00000800
#define PXO_READPOST			0x00001000

typedef struct _PXINFO {
	struct _PXINFO *prev_node, *next_node;
	SOCKET socket;
	SOCKET socket_rem;
	char *lpszInfo;
	char *lpszStatus;
	unsigned long dwThreadId;
	HANDLE hThread;
	unsigned long dwRX;
	unsigned long dwTX;
} PXINFO, *LPPXINFO;

/*
typedef struct _addr_list {
	unsigned long addr_len;
	char *name;
	void *addr;
	struct _addr_list *next_node;
} addr_list;
*/
//unsigned short redirport = PX_REDIR_PORT; // этой опции не будет (включить через ':' после хоста)
unsigned short proxyport;
//char *lppxhost = nullstring;
char *bind_addressport = nullstring;
char *proxy_script = nullstring;

#ifdef PX_GUI
unsigned int WM_TRAY;
HWND hwndMainDlg = NULL;
HMENU hMenu, hMenuZero;
NOTIFYICONDATA NotifyIconData;
HWND hwndListView, hwndEditLog;
#endif

SOCKET s;
db_list_node *myaddr = NULL;
HINSTANCE g_hInstance = NULL;
HANDLE hThread = NULL;
HANDLE hMutex = NULL;
HANDLE hFile = NULL;

LPPXINFO lppxinfo = NULL;

unsigned long g_options = 0; /*PXO_HIDDEN|PXO_TRANSPARENT|PXO_ENCRYPTIN|PXO_DECRYPTIN*/
unsigned long dwThreadSleep = 1; // if == 0 -> CPU USAGE == 100%

unsigned long dwRXTotal = 0; // принято от удалённого хоста к прокси
unsigned long dwTXTotal = 0; // отправлено от прокси к удалённому хосту

//char szLogPath[MAX_PATH];
//char szIniPath[MAX_PATH];
//char szAppPath[MAX_PATH];

LPINI_NODE ini_node = NULL;
char *ini_path = nullstring;

char hash[257] =	"erfVBWRE#@214354SDFGJOuyklghjmDxVDSGFVAdFVfdahbgfswRT4wTRhbGFhjnDfvbdefW@"
					"Rey6i987098OLYIKj<GFdsFa!@#$fdsfa*#@44325ergtffhU&^9o7pOuyjk<MfJdsAdVxzad"
					"fgqetrwq!$%hhsR!#tCVXBeryt2wt14^%@43^%15!43654876ytIJI%ueGThnszSghAyhtyWT"
					"RY@$%uytrHJDEtjU&TEiTykuiol67987oklui";

inline char *crypt_xor(char *buf, unsigned long len, unsigned long *decode_pos);

unsigned long fn_isempty(char *text)
{
	return isempty(text);
}

unsigned long fn_isset(char *text)
{
	return isalloc(text);
}

char *fn_strstr(char *text1, char *text2)
{
	return strstr(text1, text2);
}

unsigned long fn_msgbox(char *text1, char *text2)
{
	MessageBox(NULL, text2, text1, MB_TOPMOST | MB_OK);
	return 0L;
}

unsigned long fn_yes(char *text1, char *text2)
{
	return (MessageBox(NULL, text2, text1, MB_TOPMOST | MB_YESNO) == IDYES);
}

char *fn_encode(char *text)
{
	return (char *) alloc_en64((const unsigned char *) text, strlen(text));
}

unsigned long fn_free(char *text)
{
	free_str(text);
	return 0L;
}

unsigned long fn_pcre_match_free(char ***res)
{
	pcre_match_free(res);
	return 0L;
}

char *fn_strstri(char *text1, char *text2)
{
	char *res;
	text1 = _strlwr(alloc_string(text1));
	text2 = _strlwr(alloc_string(text2));
	res = strstr(text1, text2);
	free_str(text1);
	free_str(text2);

	return res;
}

char *fn_dupn(unsigned long n)
{
	return alloc_string_ex("%d", n);
}

char *fn_crypt_xor(char *text, unsigned long len, unsigned long pos)
{
	return crypt_xor(text, len, &pos);
}

/*
bool fn_strcmp(char *text1, char *text2)
{
	return !strcmp(text1, text2);
}

bool fn_stricmp(char *text1, char *text2)
{
	return !stricmp(text1, text2);
}

char *fn_add(char **text1, char *text2)
{
	return alloc_strcat(text1, text2);
}

char *fn_set(char *text)
{
	return alloc_string(text);
}
*/

// user defined table
zl_funcs_list fn_list[] = {
	{"isset",				fn_isset},
	{"isempty",				fn_isempty},
	{"encode",				fn_encode},
	{"free",				fn_free},
	{"strstri",				fn_strstri},
	{"msg",					fn_msgbox},
	{"yes",					fn_yes},
	{"dupn",				fn_dupn},
	{"pcre_match_free",		fn_pcre_match_free},
	{"crypt",				fn_crypt_xor},

	{"dup",					alloc_string},
	{"add",					alloc_strcat},
	{"addslashes",			alloc_addslashes},
	{"urlencode",			alloc_urlencode},
	{"zalloc",				zalloc},
	{"zrealloc",			zrealloc},
	{"zfree",				zfree},
	{"pcre_match_all",		pcre_match_all},
	{"pcre_replace",		pcre_replace},
	{"encoden",				alloc_en64},

	{"strstr",				strstr},
	{"strcmp",				strcmp},
	{"stricmp",				stricmp},
	{"urldecode",			urldecode},
	{"atoi",				atoi},
	{"swapchar",			swapchar},
	{"trim",				trim},
	{"ltrim",				ltrim},
	{"rtrim",				rtrim},
	{"isfloat",				isfloat},
	{"isinteger",			isinteger},
	{"strlen",				strlen},
	{"hex2char",			hex2char},
	{"killspaces",			killspaces},

	{NULL,			NULL}
	};

// data generated by zl_compile (global variables)
unsigned char *hardcode;
unsigned long hard_code_size;
unsigned char *const_sect;
unsigned long const_size;
unsigned char *data_sect;
unsigned long data_size;
unsigned char *import_sect;
unsigned long import_size;
unsigned char *reloc_sect;
unsigned long reloc_size;

unsigned char *export_sect;
unsigned long export_size;
unsigned char *map_sect;
unsigned long map_size;

unsigned long *modules;

//unsigned char *data_table;
//unsigned long vars_count;

//zl_names_map *vars_map;

void cb_addr_free(void *data)
{
	if(isalloc((char *) data))
	{
		zfree(data);
	}
}

int cb_addr_cmp(void *data, void *str)
{
	return stricmp((char *) data, (char *) str) == 0;
}

#ifdef PX_GUI
inline BOOL ListView_RedrawVisibleItems(HWND hwndListView)
{
	int fst = ListView_GetTopIndex(hwndListView);
	int lst = fst + ListView_GetCountPerPage(hwndListView);
	if(lst > ListView_GetItemCount(hwndListView))
	{
		lst = ListView_GetItemCount(hwndListView);
	}
	return ListView_RedrawItems(hwndListView, fst, lst);
}
#endif

inline char *crypt_xor(char *buf, unsigned long len, unsigned long *decode_pos)
{
	unsigned long pos;
	for(pos = 0; pos < len; pos++)
	{
		buf[pos] = buf[pos] ^ hash[(*decode_pos)++];
		if(*decode_pos == 256)
		{
			*decode_pos = 0;
		}
	}

	return buf;
}

/*
char *alloc_getstringbefore(const char *buffer, unsigned long len, unsigned long *pos, const char *delimeters)
{
	register unsigned long lnend;
	register unsigned long lnsize;

	if(*pos >= len) return nullstring;

	lnend = *pos;
	//while((lnend < len) && !strchr(delimeters, *(buffer+lnend)))
	while((lnend < len) && (strncmp(delimeters, buffer+lnend, strlen(delimeters)) != 0))
	{
		lnend++;
	}
	lnsize = lnend-*pos;

	char *temp_str = NULL;
	if(lnsize > 0)
	{
		temp_str = (char *) zalloc(lnsize+1);
		ZASSERT(temp_str);
		if(temp_str)
		{
			memcpy(temp_str, buffer+*pos, lnsize);
			*(temp_str+lnsize) = '\0';
			*pos += lnsize;
		}
	}
	//while((*pos < len) && strchr(delimeters, *(buffer+*pos)))
	//{
	//	(*pos)++;
	//}
	(*pos) += strlen(delimeters);
	return temp_str?temp_str:nullstring;
}
*/

unsigned long WINAPI PXTranslateThread(LPVOID lpParameter)
{
	// написать парсер для урлов
	// улучшить заменитель переменных

	//char *lpheader = alloc_string(lpheadertemplate);
	KEYWORD_NODE param_list[] =
	{
						{"method%",					nullstring},		// 0
						{"protocol%",				nullstring},		// 1
						{"uri_full%",				nullstring},		// 2
						{"uri_relative%",			nullstring},		// 3
						{"uri_full_encoded%",		nullstring},		// 4
						{"host%"	,				nullstring},		// 5
						{"build%",					nullstring},		// 6

						{"h_hostport%",				nullstring},		// 7
						{"h_unknown%",				nullstring},		// 8
						{"h_cookie%",				nullstring},		// 9
						{"h_accept%",				nullstring},		// 10
						{"h_accept_lang%",			nullstring},		// 11
						{"h_accept_charset%",		nullstring},		// 12
						{"h_if_mod_since%",			nullstring},		// 13
						{"h_if_none_match%",		nullstring},		// 14
						{"h_referer%",				nullstring},		// 15
						{"h_user_agent%",			nullstring},		// 16
						{"h_proxy_connection%",		nullstring},		// 17
						{"h_keep_alive%",			nullstring},		// 18
						{"h_authorization%",		nullstring},		// 19
						{"h_cache_control%",		nullstring},		// 20
						{"h_pragma%",				nullstring},		// 21
						{"h_range%",				nullstring},		// 22
						{"h_content_type%",			nullstring},		// 23
						{"h_content_length%",		nullstring},		// 24

						{"rdhost%",					nullstring},		// 25
						{"rdport%",					nullstring},		// 26
						{"port%",					nullstring},		// 27
						{"query%",					nullstring},		// 28
						{"scheme%",					nullstring},		// 29
						{"authority%",				nullstring},		// 30
						{"path%",					nullstring},		// 31
						{"fragment%",				nullstring},		// 32

						{"post_data%",				nullstring},		// 33

						{NULL, 						NULL}
	};

	char *&method			= param_list[0].szValue;
	char *&protocol			= param_list[1].szValue;
	char *&uri_full			= param_list[2].szValue;
	char *&uri_relative		= param_list[3].szValue;
	char *&uri_encoded		= param_list[4].szValue;
	char *&lphost_port		= param_list[7].szValue;
	char *&rdhost			= param_list[25].szValue;
	char *&rdport_str		= param_list[26].szValue;

	char *header			= nullstring;
	char *footer			= nullstring;
	char *post_data			= NULL;
	char *eoh				= nullstring;
	char *uri_original		= nullstring;

	unsigned long header_len	= 0;
	unsigned long post_data_len	= 0;
	unsigned long footer_len	= 0;
	unsigned short rdport		= PX_REDIR_PORT;

	//char *buf= (char *)zalloc(4097);
	char buf[4100];

#ifdef PX_GUI
	char sz_temp[32];
#endif

	char *temp_str = nullstring;

	//char *uri_param;

	LPPXINFO lpnode = (LPPXINFO) lpParameter;
	LP_QUERY_STRING_NODE uri_node = NULL;

	char chPrev[3] = {'x','e','p'};
	BOOL fHeaderSkipped = FALSE;

	HOSTENT *he;
	SOCKADDR_IN sa;
	fd_set readfds;
	timeval tv = {1, 0};

	unsigned long pos;
	unsigned long time_out = 0;
	unsigned long buflen;
	unsigned long decodein_pos = 0;
	unsigned long encodein_pos = 0;
	unsigned long decodeout_pos = 0;
	unsigned long encodeout_pos = 0;
	unsigned long recv_size = 0;
	int j;
	int i;

	unsigned long l_options = g_options & ~PXO_MDCONNECT;

	// temporary local variables
	//unsigned long *vars_table;
	//void **map_table;
	unsigned long stack[ZL_STACK_SIZE];
	unsigned long regs[REG_COUNT];


	//if(l_options & PXO_TRANSITE)
	//{
	//	goto lb_connecting;
	//}

	param_list[6].szValue = alloc_string(__BUILD_STR__);

	// читаем заголовок до пустой строки, либо пока буфер не заполнится до 4096 байт
	while(!fHeaderSkipped && (recv_size < 4096))
	{
		FD_ZERO(&readfds);
		FD_SET(lpnode->socket, &readfds);
		j = select(0, &readfds, NULL, NULL, &tv);
		if(j == 0)
		{
			time_out++;
			if(time_out >= PX_TIMEOUT)
			{
				break;
			}
			continue;
		}
		else if(j == SOCKET_ERROR)
		{
			goto end_th;
		}

		time_out = 0;

		if(FD_ISSET(lpnode->socket, &readfds))
		{
			j = recv(lpnode->socket, buf + recv_size, 4096 - recv_size, 0);
			if((j == 0) || (j == SOCKET_ERROR))
			{
				goto end_th;
			}
			if(l_options & PXO_DECRYPTIN) crypt_xor(buf, j, &decodein_pos);
			i = 0;
			while(i < j)
			{
				if(	(chPrev[0] == '\r') &&
					(chPrev[1] == '\n') &&
					(chPrev[2] == '\r') &&
					(buf[i + recv_size] == '\n'))
				{
					fHeaderSkipped = TRUE;
					break;
				}
				chPrev[0] = chPrev[1];
				chPrev[1] = chPrev[2];
				chPrev[2] = buf[i + recv_size];
				i++;
			}
			recv_size += j;
		}
	}

#if defined(_DEBUG) && defined(PX_GUI)
	if(!fHeaderSkipped)
	{
		MessageBox(NULL, "Header too big!", "proxy_rd: warning", MB_ICONEXCLAMATION | MB_OK);
	}
#endif

	chPrev[0] = 'h';
	chPrev[1] = 'u';
	chPrev[2] = 'i';

	/*
	recvsize = recv(lpnode->socket, buf, 4096, 0);
	if((recvsize == 0) || (recvsize == SOCKET_ERROR))
	{
		goto end_th;
	}
	if(l_options & PXO_CRYPTTRAFIN) crypt_xor(buf, recvsize, &decodein_pos);
	//*/

	buf[recv_size] = '\0';

	if(l_options & PXO_DEBUG)
	{
		_logd(hFile, 1, 0, "incoming header:");
		WriteFile(hFile, buf, recv_size, &pos, NULL);
	}

	// разбираем первую строку запроса
	pos = 0;
	method = alloc_getstringbefore(buf, recv_size, &pos, " ");
	if((stricmp(method, "GET") != 0)
		&& (stricmp(method, "POST") != 0)
		&& (stricmp(method, "CONNECT") != 0)
		//&& (stricmp(method, "PUT") != 0)
		//&& (stricmp(method, "TRACE") != 0)
		//&& (stricmp(method, "HEAD") != 0)
	)
	{
		goto end_th;
	}

	uri_original = alloc_getstringbefore(buf, recv_size, &pos, " ");

	protocol = alloc_getstringbefore(buf, recv_size, &pos, "\r\n");
	if(	(stricmp(protocol, "HTTP/1.0") != 0) &&
		(stricmp(protocol, "HTTP/1.1") != 0))
	{
		goto end_th;
	}

	// разбираем последующие строки запроса до пустой строки.
	while(true)
	{
		temp_str = alloc_getstringbefore(buf, recv_size, &pos, "\r\n");
		if(!isalloc(temp_str))
		{
			break;
		}
		if(strnicmp(temp_str, "Host: ", 6) == 0)
		{
			free_str(param_list[7].szValue);
			free_str(param_list[5].szValue);
			free_str(param_list[27].szValue);
			param_list[7].szValue = alloc_string(temp_str+6);
			//alloc_strcat(&param_list[7].szValue, "\r\n");

			char *dlm;
			dlm = strchr(temp_str+6, ':');
			if(dlm)
			{
				param_list[5].szValue = alloc_strncpy(temp_str+6, dlm-temp_str-6);
				param_list[27].szValue = alloc_string(dlm+1);
			}
			else
			{
				param_list[5].szValue = alloc_string(temp_str+6);
			}
		}
		else if(strnicmp(temp_str, "Proxy-Connection: ", 18) == 0)
		{
			free_str(param_list[17].szValue);
			param_list[17].szValue = alloc_string(temp_str+18);
			//alloc_strcat(&param_list[17].szValue, "\r\n");
		}
		else if(strnicmp(temp_str, "Keep-Alive: ", 12) == 0)
		{
			free_str(param_list[18].szValue);
			param_list[18].szValue = alloc_string(temp_str+12);
			//alloc_strcat(&param_list[18].szValue, "\r\n");
		}
		else if(strnicmp(temp_str, "User-Agent: ", 12) == 0)
		{
			free_str(param_list[16].szValue);
			param_list[16].szValue = alloc_string(temp_str+12);
			//alloc_strcat(&param_list[16].szValue, "\r\n");
		}
		else if(strnicmp(temp_str, "Accept: ", 8) == 0)
		{
			free_str(param_list[10].szValue);
			param_list[10].szValue = alloc_string(temp_str+8);
			//alloc_strcat(&param_list[10].szValue, "\r\n");
		}
		else if(strnicmp(temp_str, "Accept-Language: ", 17) == 0)
		{
			free_str(param_list[11].szValue);
			param_list[11].szValue = alloc_string(temp_str+17);
			//alloc_strcat(&param_list[11].szValue, "\r\n");
		}
		else if(strnicmp(temp_str, "Accept-Charset: ", 16) == 0)
		{
			free_str(param_list[12].szValue);
			param_list[12].szValue = alloc_string(temp_str+16);
			//alloc_strcat(&param_list[12].szValue, "\r\n");
		}
		else if(strnicmp(temp_str, "Cookie: ", 8) == 0)
		{
			free_str(param_list[9].szValue);
			param_list[9].szValue = alloc_string(temp_str+8);
			//alloc_strcat(&param_list[9].szValue, "\r\n");
		}
		else if(strnicmp(temp_str, "If-Modified-Since: ", 19) == 0)
		{
			free_str(param_list[13].szValue);
			param_list[13].szValue = alloc_string(temp_str+19);
			//alloc_strcat(&param_list[13].szValue, "\r\n");
		}
		else if(strnicmp(temp_str, "If-None-Match: ", 15) == 0)
		{
			free_str(param_list[14].szValue);
			param_list[14].szValue = alloc_string(temp_str+15);
			//alloc_strcat(&param_list[14].szValue, "\r\n");
		}
		else if(strnicmp(temp_str, "Referer: ", 9) == 0)
		{
			free_str(param_list[15].szValue);
			param_list[15].szValue = alloc_string(temp_str+9);
			//alloc_strcat(&param_list[15].szValue, "\r\n");
		}
		else if(strnicmp(temp_str, "Authorization: ", 15) == 0)
		{
			free_str(param_list[19].szValue);
			param_list[19].szValue = alloc_string(temp_str+15);
			//alloc_strcat(&param_list[19].szValue, "\r\n");
		}
		else if(strnicmp(temp_str, "Cache-Control: ", 15) == 0)
		{
			free_str(param_list[20].szValue);
			param_list[20].szValue = alloc_string(temp_str+15);
			//alloc_strcat(&param_list[20].szValue, "\r\n");
		}
		else if(strnicmp(temp_str, "Pragma: ", 8) == 0)
		{
			free_str(param_list[21].szValue);
			param_list[21].szValue = alloc_string(temp_str+8);
			//alloc_strcat(&param_list[21].szValue, "\r\n");
		}
		else if(strnicmp(temp_str, "Range: ", 7) == 0)
		{
			free_str(param_list[22].szValue);
			param_list[22].szValue = alloc_string(temp_str+7);
			//alloc_strcat(&param_list[22].szValue, "\r\n");
		}
		else if(strnicmp(temp_str, "Content-Type: ", 13) == 0)
		{
			free_str(param_list[23].szValue);
			param_list[23].szValue = alloc_string(temp_str+13);
			//alloc_strcat(&param_list[23].szValue, "\r\n");
		}
		else if(strnicmp(temp_str, "Content-Length: ", 16) == 0)
		{
			free_str(param_list[24].szValue);
			param_list[24].szValue = trim(alloc_string(temp_str+16));
			//alloc_strcat(&param_list[24].szValue, "\r\n");
		}
		else //unknown
		{
			/*
			if(!isempty(param_list[8].szValue))
			{
				alloc_strcat(&param_list[8].szValue, "\r\n");
			}
			*/
			alloc_strcat(&param_list[8].szValue, temp_str);
			alloc_strcat(&param_list[8].szValue, "\r\n");
		}
		free_str(temp_str);
	}

	eoh = (buf+pos);			// link to end of header (start of post data)
	buflen = recv_size - pos;	// size of part post data

	// build new uri full and relative
	if(stricmp(method, "CONNECT") != 0)
	{
		url_string_parse(&uri_node, uri_original);

		temp_str = query_string_get(uri_node, "%scheme%");
		if(isalloc(temp_str))
		{
			uri_full = alloc_string(temp_str);
			alloc_strcat(&uri_full, ":");
			param_list[29].szValue = alloc_string(temp_str);
		}
		else
		{
			uri_full = alloc_string("http:");
		}

		alloc_strcat(&uri_full, "//");

		temp_str = query_string_get(uri_node, "%authority%");
		if(isalloc(temp_str))
		{
			alloc_strcat(&uri_full, temp_str);
			param_list[30].szValue = alloc_string(temp_str);
		}
		else
		{
			alloc_strcat(&uri_full, lphost_port);
		}

		temp_str = query_string_get(uri_node, "%path%");
		if(isalloc(temp_str))
		{
			alloc_strcat(&uri_full, temp_str);
			alloc_strcat(&uri_relative, temp_str);
			param_list[31].szValue = alloc_string(temp_str);
		}
		else
		{
			alloc_strcat(&uri_full, "/");
		}

		temp_str = query_string_get(uri_node, "%query%");
		if(isalloc(temp_str))
		{
			alloc_strcat(&uri_full, "?");
			alloc_strcat(&uri_full, temp_str);
			alloc_strcat(&uri_relative, "?");
			alloc_strcat(&uri_relative, temp_str);
			param_list[28].szValue = alloc_string(temp_str);
		}

		temp_str = query_string_get(uri_node, "%fragment%");
		if(isalloc(temp_str))
		{
			alloc_strcat(&uri_full, "#");
			alloc_strcat(&uri_full, temp_str);
			alloc_strcat(&uri_relative, "#");
			alloc_strcat(&uri_relative, temp_str);
			param_list[32].szValue = alloc_string(temp_str);
		}
	}
	else
	{
		l_options |= PXO_MDCONNECT;
		uri_full = alloc_string(uri_original);
		uri_relative = alloc_string(uri_original);
		free_str(lphost_port);
		lphost_port = alloc_string(uri_original);
		free_str(param_list[5].szValue);
		free_str(param_list[27].szValue);

		char *dlm;
		dlm = strchr(lphost_port, ':');
		if(dlm)
		{
			param_list[5].szValue = alloc_strncpy(lphost_port, dlm-lphost_port);
			param_list[27].szValue = alloc_string(dlm+1);
		}
		else
		{
			param_list[5].szValue = alloc_string(lphost_port);
		}
	}

	uri_encoded = (char *) alloc_en64((const unsigned char *) uri_full, strlen(uri_full));

	// *** process here request to it self - alpha
	if(
		((isempty(param_list[27].szValue)
		&& (proxyport == 80)) 
		|| ((!isempty(param_list[27].szValue))
		&& (proxyport == (unsigned short) strtoul(param_list[27].szValue, NULL, 10))))
		&& list_find(myaddr, cb_addr_cmp, param_list[5].szValue)
	)
	{
		if(_strnicmp(param_list[31].szValue, "/settings", 9))
		{
			goto end_th;
		}

		if(stricmp(method, "GET") == 0)
		{
			char temp[10];
			header = alloc_string("HTTP/1.1 200 OK\r\nContent-Type: text/html; charset=windows-1251\r\n\r\n");
			alloc_strcat(&header, "<html>\r\n<body>\r\n<form method=\"post\" action=\"/settings\">\r\nOptions:&nbsp;<input type=\"text\" name=\"options\" value=\"");
			itoa(g_options, temp, 10);
			alloc_strcat(&header, temp);
			alloc_strcat(&header, "\" size=\"10\"><br />\r\n<textarea name=\"code\" cols=\"100\" rows=\"40\">\r\n");
			alloc_strcat(&header, proxy_script);
			alloc_strcat(&header, "</textarea><br />\r\n<input type=\"submit\" value=\"compile and apply\">\r\n</form>\r\n</body>\r\n</html>\r\n");
			recv_size = strlen(header);
			if(l_options & PXO_ENCRYPTIN) crypt_xor(header, recv_size, &encodein_pos);
			send(lpnode->socket, header, recv_size, 0);
			free_str(header);
		}
		else if(stricmp(method, "POST") == 0)
		{
			// here read post data, compile and apply code
			// return compile result message

			QUERY_STRING_NODE *query;
			char *code;
			//char *data;
			unsigned long cl;

			if(isempty(param_list[24].szValue))
			{
				goto end_th;
			}

			cl = strtoul(param_list[24].szValue, NULL, 10);
			post_data = (char *) zalloc(cl+1);
			memcpy(post_data, eoh, buflen+1);
			recv_size = buflen;
			while(recv_size < cl)
			{
				FD_ZERO(&readfds);
				FD_SET(lpnode->socket, &readfds);
				j = select(0, &readfds, NULL, NULL, &tv);
				if(j == 0)
				{
					time_out++;
					if(time_out >= PX_TIMEOUT)
					{
						goto end_th;
					}
					continue;
				}
				else if(j == SOCKET_ERROR)
				{
					break;
				}

				time_out = 0;

				if(FD_ISSET(lpnode->socket, &readfds))
				{
					j = recv(lpnode->socket, post_data+recv_size, cl-recv_size, 0);
					if((j == 0) || (j == SOCKET_ERROR))
					{
						goto end_th;
					}
					if(l_options & PXO_DECRYPTIN) crypt_xor(buf, j, &decodein_pos);
					recv_size += j;
				}
			}
			post_data[cl] = 0;

			query_string_parse(&query, post_data);
			code = query_string_get(query, "options");
			if(!isempty(code))
			{
				g_options = strtoul(trim(urldecode(code)), NULL, 10);
			}

			code = query_string_get(query, "code");

			urldecode(code);
			
			header = alloc_string("HTTP/1.1 200 OK\r\nContent-Type: text/plain; charset=windows-1251\r\n\r\n");
			if(!isempty(code))
			{
				char *error_msg;

				unsigned char *new_hardcode;
				unsigned long new_hard_code_size;
				unsigned char *new_const_sect;
				unsigned long new_const_size;
				unsigned char *new_data_sect;
				unsigned long new_data_size;
				unsigned char *new_import_sect;
				unsigned long new_import_size;
				unsigned char *new_reloc_sect;
				unsigned long new_reloc_size;

				unsigned char *new_export_sect;
				unsigned long new_export_size;
				unsigned char *new_map_sect;
				unsigned long new_map_size;

				unsigned long stack[ZL_STACK_SIZE];
				unsigned long regs[REG_COUNT];

				if(zl_compile(&new_hardcode, &new_hard_code_size, code, &error_msg,
					&new_const_sect, &new_const_size,
					&new_data_sect, &new_data_size,
					&new_reloc_sect, &new_reloc_size,
					&new_import_sect, &new_import_size,
					&new_export_sect, &new_export_size,
					&new_map_sect, &new_map_size
					))
				{
					alloc_strcat(&header, error_msg);
					alloc_strcat(&header, "\r\n\r\n");
					free_str(error_msg);
					zl_free(new_hardcode, new_const_sect, new_data_sect, new_reloc_sect, new_import_sect, new_export_sect, new_map_sect);
				}
				else
				{
					alloc_strcat(&header, "zlc: no errors. ok.\r\n\r\n");
					zl_unload_modules(modules);
					zl_free(hardcode, const_sect, data_sect, reloc_sect, import_sect, export_sect, map_sect);
					hardcode = new_hardcode;
					hard_code_size = new_hard_code_size;
					const_sect = new_const_sect;
					const_size = new_const_size;
					data_sect = new_data_sect;
					data_size = new_data_size;
					import_sect = new_import_sect;
					import_size = new_import_size;
					reloc_sect = new_reloc_sect;
					reloc_size = new_reloc_size;

					export_sect = new_export_sect;
					export_size = new_export_size;
					map_sect = new_map_sect;
					map_size = new_map_size;
					free_str(proxy_script);
					proxy_script = alloc_string(code);

					zl_load_functions(import_sect, (zl_map_section *) map_sect, map_size/sizeof(zl_map_section), fn_list, &modules);

					zl_init(zl_offset("main", (zl_export_section *) export_sect, export_size/sizeof(zl_export_section)), hardcode, stack, regs, const_sect, data_sect, reloc_sect, import_sect);
					zl_call(regs);

					ini_put_string(&ini_node, "Options", "options", NULL, "%d", g_options);
					//ini_put_string(&ini_node, "Options", "bindport", NULL, "%s", bind_addressport);
					ini_put_string(&ini_node, "Options", "redir_header", NULL, "%s", proxy_script);
					ini_flush(ini_node, ini_path, 0);
				}

				alloc_strcat(&header, code);
			}
			else
			{
				alloc_strcat(&header, "Error: empty script");
			}
			recv_size = strlen(header);
			if(l_options & PXO_ENCRYPTIN) crypt_xor(header, recv_size, &encodein_pos);
			send(lpnode->socket, header, recv_size, 0);
			free_str(header);
			zfree(post_data);
			post_data = NULL;
			query_string_free(&query);
		}
		goto end_th;
	}
	// *** end process request to it self

	// reading POST data to memory if it size no more 10 Mb
	if((l_options & PXO_READPOST) && (strcmp(method, "POST") == 0) && !isempty(param_list[24].szValue))
	{
		post_data_len = strtoul(param_list[24].szValue, NULL, 10);
		if(post_data_len <= 10485760)
		{
			post_data = (char *) zalloc(post_data_len+1);
			memcpy(post_data, eoh, buflen+1);
			recv_size = buflen;
			buflen = 0;
			while(recv_size < post_data_len)
			{
				FD_ZERO(&readfds);
				FD_SET(lpnode->socket, &readfds);
				j = select(0, &readfds, NULL, NULL, &tv);
				if(j == 0)
				{
					time_out++;
					if(time_out >= PX_TIMEOUT)
					{
						goto end_th;
					}
					continue;
				}
				else if(j == SOCKET_ERROR)
				{
					goto end_th;
				}

				time_out =  0;

				if(FD_ISSET(lpnode->socket, &readfds))
				{
					j = recv(lpnode->socket, post_data + recv_size, post_data_len - recv_size, 0);
					if((j == 0) || (j == SOCKET_ERROR))
					{
						goto end_th;
					}
					if(l_options & PXO_DECRYPTIN) crypt_xor(buf, j, &decodein_pos);
					recv_size += j;
				}
			}
			post_data[post_data_len] = 0;
			
			if(post_data_len != recv_size)
			{
				goto end_th;
			}

		}
		else
		{
			post_data_len = 0;
		}
	}

	//if(l_options & PXO_TRANSITE)
	//{
	//	goto lb_connecting;
	//}

	//replace_variable(&header, param_list);

	/*
	zl_memtable_make2(&map_table, vars_map);

	zl_set(map_table, vars_map, "header", &header);
	zl_set(map_table, vars_map, "options", &l_options);

	zl_set(map_table, vars_map, "method", &param_list[0].szValue);
	zl_set(map_table, vars_map, "protocol", &param_list[1].szValue);
	zl_set(map_table, vars_map, "uri_full", &param_list[2].szValue);
	zl_set(map_table, vars_map, "uri_relative", &param_list[3].szValue);
	zl_set(map_table, vars_map, "uri_full_encoded", &param_list[4].szValue);
	zl_set(map_table, vars_map, "host", &param_list[5].szValue);
	zl_set(map_table, vars_map, "build", &param_list[6].szValue);

	zl_set(map_table, vars_map, "h_host_port", &param_list[7].szValue);
	zl_set(map_table, vars_map, "h_unknown", &param_list[8].szValue);
	zl_set(map_table, vars_map, "h_cookie", &param_list[9].szValue);
	zl_set(map_table, vars_map, "h_accept", &param_list[10].szValue);
	zl_set(map_table, vars_map, "h_accept_lang", &param_list[11].szValue);
	zl_set(map_table, vars_map, "h_accept_charset", &param_list[12].szValue);
	zl_set(map_table, vars_map, "h_if_mod_since", &param_list[13].szValue);
	zl_set(map_table, vars_map, "h_if_none_match", &param_list[14].szValue);
	zl_set(map_table, vars_map, "h_referer", &param_list[15].szValue);
	zl_set(map_table, vars_map, "h_user_agent", &param_list[16].szValue);
	zl_set(map_table, vars_map, "h_proxy_connection", &param_list[17].szValue);
	zl_set(map_table, vars_map, "h_keep_alive", &param_list[18].szValue);
	zl_set(map_table, vars_map, "h_authorization", &param_list[19].szValue);
	zl_set(map_table, vars_map, "h_cache_control", &param_list[20].szValue);
	zl_set(map_table, vars_map, "h_pragma", &param_list[21].szValue);
	zl_set(map_table, vars_map, "h_range", &param_list[22].szValue);
	zl_set(map_table, vars_map, "h_content_type", &param_list[23].szValue);
	zl_set(map_table, vars_map, "h_content_length", &param_list[24].szValue);
	zl_set(map_table, vars_map, "rdhost", &param_list[25].szValue);
	zl_set(map_table, vars_map, "rdport", &param_list[26].szValue);
	zl_set(map_table, vars_map, "port", &param_list[27].szValue);
	zl_set(map_table, vars_map, "query", &param_list[28].szValue);
	zl_set(map_table, vars_map, "scheme", &param_list[29].szValue);
	zl_set(map_table, vars_map, "authority", &param_list[30].szValue);
	zl_set(map_table, vars_map, "path", &param_list[31].szValue);
	zl_set(map_table, vars_map, "fragment", &param_list[32].szValue);

	zl_execute(hardcode, data_table, map_table, fn_list);

	zl_free(NULL, &map_table, NULL, NULL);
	*/

	zl_init(zl_offset("proxy_rd", (zl_export_section *) export_sect, export_size/sizeof(zl_export_section)), hardcode, stack, regs, const_sect, data_sect, reloc_sect, import_sect);

	//zl_push(regs, dw(&header));
	zl_push(regs, dw(&header_len));
	zl_push(regs, dw(&post_data));
	zl_push(regs, dw(&post_data_len));
	zl_push(regs, dw(&footer));
	zl_push(regs, dw(&footer_len));
	zl_push(regs, dw(&l_options));
	zl_push(regs, dw(&param_list[25].szValue));
	zl_push(regs, dw(&param_list[26].szValue));

	zl_push(regs, dw(param_list[0].szValue));
	zl_push(regs, dw(param_list[1].szValue));
	zl_push(regs, dw(param_list[2].szValue));
	zl_push(regs, dw(param_list[3].szValue));
	zl_push(regs, dw(param_list[4].szValue));
	zl_push(regs, dw(param_list[5].szValue));
	zl_push(regs, dw(param_list[6].szValue));
	zl_push(regs, dw(param_list[7].szValue));
	zl_push(regs, dw(param_list[8].szValue));
	zl_push(regs, dw(param_list[9].szValue));
	zl_push(regs, dw(param_list[10].szValue));
	zl_push(regs, dw(param_list[11].szValue));
	zl_push(regs, dw(param_list[12].szValue));
	zl_push(regs, dw(param_list[13].szValue));
	zl_push(regs, dw(param_list[14].szValue));
	zl_push(regs, dw(param_list[15].szValue));
	zl_push(regs, dw(param_list[16].szValue));
	zl_push(regs, dw(param_list[17].szValue));
	zl_push(regs, dw(param_list[18].szValue));
	zl_push(regs, dw(param_list[19].szValue));
	zl_push(regs, dw(param_list[20].szValue));
	zl_push(regs, dw(param_list[21].szValue));
	zl_push(regs, dw(param_list[22].szValue));
	zl_push(regs, dw(param_list[23].szValue));
	zl_push(regs, dw(param_list[24].szValue));
	zl_push(regs, dw(param_list[27].szValue));
	zl_push(regs, dw(param_list[28].szValue));
	zl_push(regs, dw(param_list[29].szValue));
	zl_push(regs, dw(param_list[30].szValue));
	zl_push(regs, dw(param_list[31].szValue));
	zl_push(regs, dw(param_list[32].szValue));

	header = (char *) zl_call(regs);

	zl_pop(regs, 34);

	free_str(lpnode->lpszInfo);
	free_str(lpnode->lpszStatus);
	lpnode->lpszInfo = alloc_string(uri_full);
	lpnode->lpszStatus = alloc_string(uri_relative);

#ifdef PX_GUI
	ListView_RedrawVisibleItems(hwndListView);
#endif

//lb_connecting:

	fHeaderSkipped = (l_options & PXO_SKIPHEADER) != PXO_SKIPHEADER;
	//rdhost = alloc_string((l_options & PXO_TRANSPARENT)?lphost_port:lppxhost);

	if(isempty(rdhost)/* || (isempty(header) && !(l_options & PXO_TRANSITE))*/)
	{
		goto end_th;
	}

	if(!isempty(rdport_str))
	{
		rdport = (unsigned short) strtoul(rdport_str, NULL, 10);
	}

	he = gethostbyname(rdhost); // хост, на который отсылаем запрос
	if(he)
	{
		memcpy(&(sa.sin_addr), he->h_addr, he->h_length);
	}
	else
	{
		sa.sin_addr.s_addr = inet_addr(rdhost);
		if(sa.sin_addr.s_addr == INADDR_NONE)
		{
			goto end_th;
		}
	}

	/*
	if((l_options & PXO_TRANSPARENT) && isalloc(lpport))
	{
		rdport = atoi(lpport);
	}
	*/

	sa.sin_family = AF_INET;
	sa.sin_port = htons(rdport);

	lpnode->socket_rem = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(lpnode->socket_rem == INVALID_SOCKET)
	{
		goto end_th;
	}
	if(connect(lpnode->socket_rem, (struct sockaddr FAR *) &sa, sizeof(sa)) == SOCKET_ERROR)
	{
		//rdport = WSAGetLastError();
		closesocket(lpnode->socket_rem);
		goto end_th;
	}

	// old was - if(stricmp(method, "CONNECT") != 0)
	if(!isempty(header) && (header_len > 0))
	{
		recv_size = header_len;
		//recv_size = strlen(header); // переделать. может выдавать неверную длину.
		// но вряд ли. в заголовке не может использоваться ноль!
		if(l_options & PXO_DEBUG)
		{
			_logd(hFile, 1, 0, "modified header:");
			WriteFile(hFile, header, recv_size, &pos, NULL);
		}
		if(l_options & PXO_ENCRYPTOUT)
		{
			if(l_options & PXO_ASSECONDHDR)
			{
				char *dlm;
				dlm = strstr(header, "\r\n\r\n");
				if(dlm)
				{
					crypt_xor(dlm+4, header+recv_size-dlm-4, &encodeout_pos);
				}
			}
			else
			{
				crypt_xor(header, recv_size, &encodeout_pos);
			}
		}
		send(lpnode->socket_rem, header, recv_size, 0);
		free_str(header);

		lpnode->dwTX += recv_size;
		dwTXTotal += recv_size;
	}

	if(l_options & PXO_TRANSITE)
	{
		if(l_options & PXO_ENCRYPTOUT) crypt_xor(buf, recv_size, &encodeout_pos);
		send(lpnode->socket_rem, buf, recv_size, 0);

		lpnode->dwTX += recv_size;
		dwTXTotal += recv_size;
	}
	else if(post_data && (post_data_len > 0))
	{
		if(l_options & PXO_ENCRYPTOUT) crypt_xor(post_data, post_data_len, &encodeout_pos);
		send(lpnode->socket_rem, post_data, post_data_len, 0);
		zfree(post_data);
		post_data = NULL;
	
		lpnode->dwTX += post_data_len;
		dwTXTotal += post_data_len;
	}
	else if(buflen > 0)
	{
		if(l_options & PXO_ENCRYPTOUT) crypt_xor(eoh, buflen, &encodeout_pos);
		send(lpnode->socket_rem, eoh, buflen, 0);
	
		lpnode->dwTX += buflen;
		dwTXTotal += buflen;
	}

	if(!isempty(footer) && (footer_len > 0))
	{
		recv_size = footer_len;
		//recv_size = strlen(footer); // переделать. может выдавать неверную длину.
		// но вряд ли. в заголовке не может использоваться ноль!
		if(l_options & PXO_ENCRYPTOUT)
		{
			crypt_xor(footer, recv_size, &encodeout_pos);
		}
		send(lpnode->socket_rem, footer, recv_size, 0);
		free_str(footer);

		lpnode->dwTX += recv_size;
		dwTXTotal += recv_size;
	}

	/*
	if(l_options & PXO_TRANSITE)
	{
		if(l_options & PXO_ENCRYPTOUT) crypt_xor(buf, recvsize, &encodeout_pos);
		send(lpnode->socket_rem, buf, recvsize, 0);
		goto lb_dataexchange;
	}

	// old was - if(stricmp(method, "CONNECT") != 0)
	if(!isempty(header))
	{
		recvsize = strlen(header); // переделать. может выдавать неверную длину.
		// но вряд ли. в заголовке не может использоваться ноль!
		if(l_options & PXO_DEBUG) WriteFile(hFile, header, recvsize, &pos, NULL);
		if((l_options & PXO_ENCRYPTOUT) & ~PXO_ASSECONDHDR) crypt_xor(header, recvsize, &encodeout_pos);
		send(lpnode->socket_rem, header, recvsize, 0);
		free_str(header);

		lpnode->dwTX += recvsize;
		dwTXTotal += recvsize;
	}

	if(l_options & PXO_ASSECONDHDR)
	{
		if(l_options & PXO_ENCRYPTOUT) crypt_xor(buf, recvsize, &encodeout_pos);
		send(lpnode->socket_rem, buf, recvsize, 0);
	}
	else if(buflen > 0)
	{
		if(l_options & PXO_ENCRYPTOUT) crypt_xor(postdata, buflen, &encodeout_pos);
		send(lpnode->socket_rem, postdata, buflen, 0);
	
		lpnode->dwTX += buflen;
		dwTXTotal += buflen;
	}
	//*/

	//if(stricmp(method, "CONNECT") == 0)
	if(l_options & PXO_MDCONNECT)
	{
		free_str(header);
		header = alloc_string(PX_CONNECT_HEADER); //HTTP/1.1 200 Connection established
		replace_variable(&header, param_list);
		recv_size = strlen(header);
		if(l_options & PXO_ENCRYPTIN) crypt_xor(header, recv_size, &encodein_pos);
		send(lpnode->socket, header, recv_size, 0);
		free_str(header);
		lpnode->dwRX += recv_size;
		dwRXTotal += recv_size;
	}

//lb_dataexchange:

#ifdef PX_GUI
	ListView_RedrawVisibleItems(hwndListView);
	wsprintf(sz_temp, "%d", dwTXTotal);
	SetDlgItemText(hwndMainDlg, IDC_LOCALE, sz_temp);
#endif

	while(TRUE)
	{
		FD_ZERO(&readfds);
		FD_SET(lpnode->socket, &readfds);
		FD_SET(lpnode->socket_rem, &readfds);
		j = select(0, &readfds, NULL, NULL, &tv);
		if(j == 0)
		{
			time_out++;
			if(time_out >= PX_TIMEOUT)
			{
				break;
			}
			continue;
		}
		else if(j == SOCKET_ERROR)
		{
			break;
		}

		time_out = 0;

		if(FD_ISSET(lpnode->socket_rem, &readfds))
		{
			j = recv(lpnode->socket_rem, buf, 2048, 0);
			if((j == 0) || (j == SOCKET_ERROR))
			{
				break;
			}
			lpnode->dwRX += j;
			dwRXTotal += j;
#ifdef PX_GUI
			wsprintf(sz_temp, "%d", dwRXTotal);
			SetDlgItemText(hwndMainDlg, IDC_REMOTE, sz_temp);
#endif
			if(fHeaderSkipped)
			{
				if(l_options & PXO_DECRYPTOUT) crypt_xor(buf, j, &decodeout_pos);
				if(l_options & PXO_ENCRYPTIN) crypt_xor(buf, j, &encodein_pos);
				send(lpnode->socket, buf, j, 0);
			}
			else
			{
				i = 0;
				while(i < j)
				{
					if(	(chPrev[0] == '\r') &&
						(chPrev[1] == '\n') &&
						(chPrev[2] == '\r') &&
 						(buf[i]	== '\n'))
					{
						fHeaderSkipped = TRUE;
						i++;
						if(i < j)
						{
							if(l_options & PXO_DECRYPTOUT) crypt_xor(&buf[i], j - i, &decodeout_pos);
							if(l_options & PXO_ENCRYPTIN) crypt_xor(&buf[i], j - i, &encodein_pos);
							send(lpnode->socket, &buf[i], j - i, 0);
						}
						break;
					}
					chPrev[0] = chPrev[1];
					chPrev[1] = chPrev[2];
					chPrev[2] = buf[i];
					i++;
				}
			}
		}
		if(FD_ISSET(lpnode->socket, &readfds))
		{
			j = recv(lpnode->socket, buf, 2048, 0);
			if((j == 0) || (j == SOCKET_ERROR))
			{
				break;
			}
			if(l_options & PXO_DECRYPTIN) crypt_xor(buf, j, &decodein_pos);
			lpnode->dwTX += j;
			dwTXTotal += j;
#ifdef PX_GUI
			wsprintf(sz_temp, "%d", dwTXTotal);
			SetDlgItemText(hwndMainDlg, IDC_LOCALE, sz_temp);
#endif
			if(l_options & PXO_ENCRYPTOUT) crypt_xor(buf, j, &encodeout_pos);
			send(lpnode->socket_rem, buf, j, 0);
		}

#ifdef PX_GUI
		ListView_RedrawVisibleItems(hwndListView);
#endif

	}

	closesocket(lpnode->socket_rem);

end_th:
	query_string_free(&uri_node);

	//free_str(buf);
	free_str(header);
	free_str(footer);
	//free_str(rdhost);
	free_str(uri_original);
	if(post_data)
	{
		zfree(post_data);
	}

	for(pos = 0; param_list[pos].szKey; pos++)
	{
		free_str(param_list[pos].szValue);
	}

#ifdef PX_GUI
	ListView_SetItemCount(hwndListView, ListView_GetItemCount(hwndListView)-1);
#endif

	if(lpnode->prev_node)
	{
		lpnode->prev_node->next_node = lpnode->next_node;
	}
	else
	{
		lppxinfo = lpnode->next_node;
	}

	if(lpnode->next_node)
	{
		lpnode->next_node->prev_node = lpnode->prev_node;
	}

	closesocket(lpnode->socket);

	CloseHandle(lpnode->hThread);

	free_str(lpnode->lpszInfo);
	free_str(lpnode->lpszStatus);

	zfree(lpnode);

#ifdef PX_GUI
	ListView_RedrawVisibleItems(hwndListView);
#endif

	return 0L;
}

unsigned long WINAPI PXMainThread(LPVOID lpParameter)
{
	int tmp;
	//SOCKET s;
	SOCKADDR_IN sa;
	HOSTENT *he;
	int salen;
	char *temp_str, *lpchr;
	fd_set readfds;
	timeval tv = {2, 0};

	sa.sin_family = AF_INET;
	//sa.sin_addr.s_addr = htonl(INADDR_ANY);
	//sa.sin_port = htons(proxyport);

	temp_str = alloc_string(bind_addressport);

	lpchr = strchr(temp_str, ':');
	if(lpchr)
	{
		*lpchr = '\0';
		proxyport = (unsigned short) strtoul(lpchr+1, NULL, 10);
		sa.sin_port = htons(proxyport);

		he = gethostbyname(temp_str);
		if(!he)
		{
			sa.sin_addr.s_addr = inet_addr(temp_str);
			if(sa.sin_addr.s_addr == INADDR_NONE)
			{
				WSACleanup();
				ExitProcess(0);
				return 0;
			}
		}
		else
		{
			memcpy(&(sa.sin_addr), he->h_addr, he->h_length);
		}
	}
	else
	{
		sa.sin_addr.s_addr = htonl(INADDR_ANY);
		proxyport = (unsigned short) strtoul(bind_addressport, NULL, 10);
		sa.sin_port = htons(proxyport);
	}

	free_str(temp_str);

	s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); //IPPROTO_UDP
	if(s == INVALID_SOCKET)
	{
		WSACleanup();
		ExitProcess(0);
		return 0;
	}

	tmp = bind(s, (LPSOCKADDR) &sa, sizeof(sa));
	if(tmp == SOCKET_ERROR)
	{
		closesocket(s);
		WSACleanup();
		ExitProcess(0);
		return 0;
	}

	tmp = listen(s, SOMAXCONN);
	if(tmp == SOCKET_ERROR)
	{
		closesocket(s);
		WSACleanup();
		ExitProcess(0);
		return 0;
	}

	while(WaitForSingleObject(hMutex, 0) == WAIT_TIMEOUT)
	{
		FD_ZERO(&readfds);
		FD_SET(s, &readfds);
		tmp = select(0, &readfds, NULL, NULL, &tv);
		if(tmp == SOCKET_ERROR)
		{
			break;
		}
		
		if((tmp > 0) && FD_ISSET(s, &readfds))
		{
			salen = sizeof(sa);
			SOCKET ns = accept(s, (struct sockaddr FAR *) &sa, &salen);
			if(ns == INVALID_SOCKET)
			{
				closesocket(s);
				WSACleanup();
				ExitProcess(0);
				return 0;
			}

			LPPXINFO lpnode = (LPPXINFO) zalloc(sizeof(PXINFO));
			if(lpnode)
			{
				memset(lpnode, 0, sizeof(PXINFO));

				lpnode->socket = ns;
				lpnode->hThread = CreateThread(NULL, 0, PXTranslateThread, lpnode, 0, &lpnode->dwThreadId);
				lpnode->lpszInfo = alloc_string("connection");
				lpnode->lpszStatus = alloc_string("active");

				int icount = 1;
				LPPXINFO lptemp = lppxinfo;
				if(lptemp)
				{
					icount++;
					while(lptemp->next_node)
					{
						lptemp = lptemp->next_node;
						icount++;
					}

					lptemp->next_node = lpnode;
					lpnode->prev_node = lptemp;
				}
				else
				{
					lppxinfo = lpnode;
				}

	#ifdef PX_GUI
				ListView_SetItemCount(hwndListView, icount);
				ListView_RedrawVisibleItems(hwndListView);
	#endif
			}
		}
	} //endwhile

	closesocket(s);
	
	//WSACleanup();
	//ExitProcess(0);
	return 0;
}

void PXKillAll()
{
	// слишком жестокое убиение процессов
	// пересмотреть, переделать
	while(lppxinfo)
	{
		TerminateThread(lppxinfo->hThread, 0);
		CloseHandle(lppxinfo->hThread);
		closesocket(lppxinfo->socket);
		closesocket(lppxinfo->socket_rem);
		free_str(lppxinfo->lpszInfo);
		free_str(lppxinfo->lpszStatus);
		if(lppxinfo->next_node)
		{
			lppxinfo = lppxinfo->next_node;
			zfree(lppxinfo->prev_node);
		}
		else
		{
			zfree(lppxinfo);
			lppxinfo = NULL;
		}
	}

	if(hMutex)
	{
		ReleaseMutex(hMutex);
		CloseHandle(hMutex);
		hMutex = NULL;
	}

	if(hThread)
	{
		ZTRACE("Waiting thread exit...\n");
		if(WaitForSingleObject(hThread, 180000) == WAIT_TIMEOUT)
		{
			ZTRACE("Terminating thread...\n");
			TerminateThread(hThread, 0);
		}
		CloseHandle(hThread);
		hThread = NULL;
	}

	closesocket(s);
}

#ifdef PX_GUI
BOOL PXShutDown()
{
	if(SendDlgItemMessage(hwndMainDlg, IDC_CHKEN, BM_GETCHECK, 0, 0L) == BST_CHECKED)
	{
		PXKillAll();
	}
	DestroyIcon(NotifyIconData.hIcon);
	DestroyMenu(hMenu);
	//EndDialog(hwndMainDlg, 0);
	DestroyWindow(hwndMainDlg);
	PostQuitMessage(0); //new method
	return FALSE;
}

BOOL CALLBACK DialogProc_Options(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
		case WM_INITDIALOG:
			{
				int tab_size = 8;
				SendDlgItemMessage(hwnd, IDC_EDHEADER, EM_SETTABSTOPS, 1, (LPARAM) &tab_size);
				SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
				//SetDlgItemText(hwnd, IDC_EDHST, lppxhost);
				//SetDlgItemInt(hwnd, IDC_EDPRT, redirport, FALSE);
				//SetDlgItemInt(hwnd, IDC_EDPXPRT, proxyport, FALSE);
				SetDlgItemText(hwnd, IDC_EDPXPRT, bind_addressport);
				SetDlgItemText(hwnd, IDC_EDHEADER, proxy_script);

				CheckDlgButton(hwnd, IDC_CHECK1, (g_options & PXO_SKIPHEADER)?BST_CHECKED:BST_UNCHECKED);
				CheckDlgButton(hwnd, IDC_CHECK2, (g_options & PXO_ASSECONDHDR)?BST_CHECKED:BST_UNCHECKED);
				//CheckDlgButton(hwnd, IDC_CHECK3, (g_options & PXO_TRANSPARENT)?BST_CHECKED:BST_UNCHECKED);
				CheckDlgButton(hwnd, IDC_CHECK5, (g_options & PXO_DEBUG)?BST_CHECKED:BST_UNCHECKED);
				CheckDlgButton(hwnd, IDC_CHECK4, (g_options & PXO_ENCRYPTIN)?BST_CHECKED:BST_UNCHECKED);
				CheckDlgButton(hwnd, IDC_CHECK10, (g_options & PXO_ENCRYPTOUT)?BST_CHECKED:BST_UNCHECKED);
				CheckDlgButton(hwnd, IDC_CHECK11, (g_options & PXO_DECRYPTIN)?BST_CHECKED:BST_UNCHECKED);
				CheckDlgButton(hwnd, IDC_CHECK12, (g_options & PXO_DECRYPTOUT)?BST_CHECKED:BST_UNCHECKED);
				CheckDlgButton(hwnd, IDC_CHECK6, (g_options & PXO_TRANSITE)?BST_CHECKED:BST_UNCHECKED);
				CheckDlgButton(hwnd, IDC_CHECK13, (g_options & PXO_READPOST)?BST_CHECKED:BST_UNCHECKED);
			}
			break;
		case WM_COMMAND:
			switch(LOWORD(wParam))
			{
				case IDC_MAILME:
					{
						ShellExecute(NULL, "open", "mailto:pfzim@mail.ru", NULL, NULL, SW_SHOW);
					}
					break;
				case IDC_COMPILE:
					{
						char *error_msg, *code;
						int len;

						unsigned char *new_hardcode;
						unsigned long new_hard_code_size;
						unsigned char *new_const_sect;
						unsigned long new_const_size;
						unsigned char *new_data_sect;
						unsigned long new_data_size;
						unsigned char *new_import_sect;
						unsigned long new_import_size;
						unsigned char *new_reloc_sect;
						unsigned long new_reloc_size;

						unsigned char *new_export_sect;
						unsigned long new_export_size;
						unsigned char *new_map_sect;
						unsigned long new_map_size;

						unsigned long stack[ZL_STACK_SIZE];
						unsigned long regs[REG_COUNT];

						len = SendDlgItemMessage(hwnd, IDC_EDHEADER, WM_GETTEXTLENGTH, 0, 0L);
						code = (char *) zalloc(len+1);
						if(code)
						{
							GetDlgItemText(hwnd, IDC_EDHEADER, code, len+1);

							SetDlgItemText(hwnd, IDC_ZLCSTATUS, "zlc: compiling. please wait...");

							if(zl_compile(&new_hardcode, &new_hard_code_size, code, &error_msg,
								&new_const_sect, &new_const_size,
								&new_data_sect, &new_data_size,
								&new_reloc_sect, &new_reloc_size,
								&new_import_sect, &new_import_size,
								&new_export_sect, &new_export_size,
								&new_map_sect, &new_map_size
								))
							{
								SetDlgItemText(hwnd, IDC_ZLCSTATUS, error_msg);
								free_str(error_msg);
								zl_free(new_hardcode, new_const_sect, new_data_sect, new_reloc_sect, new_import_sect, new_export_sect, new_map_sect);
								//zl_free(&new_hardcode, NULL, &new_data_table, &new_vars_map);
							}
							else
							{
								SetDlgItemText(hwnd, IDC_ZLCSTATUS, "zlc: no errors. ok.");
								zl_unload_modules(modules);
								zl_free(hardcode, const_sect, data_sect, reloc_sect, import_sect, export_sect, map_sect);
								hardcode = new_hardcode;
								hard_code_size = new_hard_code_size;
								const_sect = new_const_sect;
								const_size = new_const_size;
								data_sect = new_data_sect;
								data_size = new_data_size;
								import_sect = new_import_sect;
								import_size = new_import_size;
								reloc_sect = new_reloc_sect;
								reloc_size = new_reloc_size;

								export_sect = new_export_sect;
								export_size = new_export_size;
								map_sect = new_map_sect;
								map_size = new_map_size;

								zl_load_functions(import_sect, (zl_map_section *) map_sect, map_size/sizeof(zl_map_section), fn_list, &modules);

								zl_init(zl_offset("main", (zl_export_section *) export_sect, export_size/sizeof(zl_export_section)), hardcode, stack, regs, const_sect, data_sect, reloc_sect, import_sect);
								zl_call(regs);
							}

							/*
							if(zl_compile(&new_hardcode, &new_data_table, fn_list, code, &new_vars_map, &error_msg))
							{
								SetDlgItemText(hwnd, IDC_ZLCSTATUS, error_msg);
								free_str(error_msg);
								zl_free(&new_hardcode, NULL, &new_data_table, &new_vars_map);
							}
							else
							{
								SetDlgItemText(hwnd, IDC_ZLCSTATUS, "zlc: no errors. ok.");
								zl_free(&hardcode, NULL, &data_table, &vars_map);
								hardcode = new_hardcode;
								data_table = new_data_table;
								vars_map = new_vars_map;
							}
							//*/

							free_str(code);
						}
					}
					break;
				case IDOK:
					{
						g_options = IsDlgButtonChecked(hwnd, IDC_CHECK1) == BST_CHECKED?g_options|PXO_SKIPHEADER:g_options&~PXO_SKIPHEADER;
						g_options = IsDlgButtonChecked(hwnd, IDC_CHECK2) == BST_CHECKED?g_options|PXO_ASSECONDHDR:g_options&~PXO_ASSECONDHDR;
						//g_options = IsDlgButtonChecked(hwnd, IDC_CHECK3) == BST_CHECKED?g_options|PXO_TRANSPARENT:g_options&~PXO_TRANSPARENT;
						g_options = IsDlgButtonChecked(hwnd, IDC_CHECK5) == BST_CHECKED?g_options|PXO_DEBUG:g_options&~PXO_DEBUG;
						g_options = IsDlgButtonChecked(hwnd, IDC_CHECK4) == BST_CHECKED?g_options|PXO_ENCRYPTIN:g_options&~PXO_ENCRYPTIN;
						g_options = IsDlgButtonChecked(hwnd, IDC_CHECK10) == BST_CHECKED?g_options|PXO_ENCRYPTOUT:g_options&~PXO_ENCRYPTOUT;
						g_options = IsDlgButtonChecked(hwnd, IDC_CHECK11) == BST_CHECKED?g_options|PXO_DECRYPTIN:g_options&~PXO_DECRYPTIN;
						g_options = IsDlgButtonChecked(hwnd, IDC_CHECK12) == BST_CHECKED?g_options|PXO_DECRYPTOUT:g_options&~PXO_DECRYPTOUT;
						g_options = IsDlgButtonChecked(hwnd, IDC_CHECK6) == BST_CHECKED?g_options|PXO_TRANSITE:g_options&~PXO_TRANSITE;
						g_options = IsDlgButtonChecked(hwnd, IDC_CHECK13) == BST_CHECKED?g_options|PXO_READPOST:g_options&~PXO_READPOST;

						/*
						if(g_options & PXO_INSTALL)
						{
							installtosystem("prd", "prd.exe");
						}
						else
						{
							deinstallfromsystem("prd", "prd.exe");
						}
						*/

						//proxyport = GetDlgItemInt(hwnd, IDC_EDPXPRT, NULL, FALSE);
						//redirport = GetDlgItemInt(hwnd, IDC_EDPRT, NULL, FALSE);

						int len = SendDlgItemMessage(hwnd, IDC_EDPXPRT, WM_GETTEXTLENGTH, 0, 0L);
						free_str(bind_addressport);
						bind_addressport = (char *) zalloc(len+1);
						if(bind_addressport) GetDlgItemText(hwnd, IDC_EDPXPRT, bind_addressport, len+1);

						/*
						len = SendDlgItemMessage(hwnd, IDC_EDHST, WM_GETTEXTLENGTH, 0, 0L);
						free_str(lppxhost);
						lppxhost = (char *) zalloc(len+1);
						if(lppxhost) GetDlgItemText(hwnd, IDC_EDHST, lppxhost, len+1);
						*/

						len = SendDlgItemMessage(hwnd, IDC_EDHEADER, WM_GETTEXTLENGTH, 0, 0L);
						free_str(proxy_script);
						proxy_script = (char *) zalloc(len+1);
						if(proxy_script) GetDlgItemText(hwnd, IDC_EDHEADER, proxy_script, len+1);

						ini_put_string(&ini_node, "Options", "options", NULL, "%d", g_options);
						//ini_put_string(&ini_node, "Options", "redir_port", NULL, "%d", redirport);
						ini_put_string(&ini_node, "Options", "bindport", NULL, "%s", bind_addressport);
						//ini_put_string(&ini_node, "Options", "pxport", NULL, "%d", proxyport);
						//ini_put_string(&ini_node, "Options", "redir_host", NULL, "%s", lppxhost);
						ini_put_string(&ini_node, "Options", "redir_header", NULL, "%s", proxy_script);
					}
				case IDCANCEL:
					EndDialog(hwnd, LOWORD(wParam));
					break;
			}
			break;
	}
	return FALSE;
}

BOOL MainDlg_OnInitDialog (HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
	hwndMainDlg = hwnd;

	hMenu = GetSystemMenu(hwnd, FALSE);
	EnableMenuItem(hMenu, SC_MAXIMIZE, MF_GRAYED);
	EnableMenuItem(hMenu, SC_SIZE, MF_GRAYED);

	hMenu = LoadMenu(g_hInstance, MAKEINTRESOURCE(IDR_MENU));
	hMenuZero = GetSubMenu(hMenu, 0);

	hwndListView = GetDlgItem(hwnd, IDC_CONLIST);

	NotifyIconData.cbSize = sizeof(NOTIFYICONDATA);
	NotifyIconData.hWnd = hwnd;
	NotifyIconData.uID = 1;
	NotifyIconData.uFlags = NIF_MESSAGE|NIF_ICON|NIF_TIP;
	NotifyIconData.uCallbackMessage = WM_TRAY;
	NotifyIconData.hIcon = LoadIcon(g_hInstance, MAKEINTRESOURCE(IDI_TRICON));
	lstrcpy(NotifyIconData.szTip, "proxy_rd");

	ListView_SetExtendedListViewStyleEx(hwndListView, LVS_EX_FULLROWSELECT, LVS_EX_FULLROWSELECT);

	LVCOLUMN lvColumn;
	lvColumn.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	lvColumn.fmt = LVCFMT_LEFT;
	lvColumn.pszText = "url";
	lvColumn.cx = 115;
	ListView_InsertColumn(hwndListView, 0, &lvColumn);
	lvColumn.pszText = "status";
	lvColumn.cx = 75;
	ListView_InsertColumn(hwndListView, 1, &lvColumn);
	lvColumn.pszText = "recv";
	lvColumn.cx = 60;
	ListView_InsertColumn(hwndListView, 2, &lvColumn);
	lvColumn.pszText = "send";
	lvColumn.cx = 60;
	ListView_InsertColumn(hwndListView, 3, &lvColumn);
	ListView_SetItemCount(hwndListView, 0);

	SetDlgItemText(hwnd, IDC_BUILD, "build# "__BUILD_STR__);
	SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
	SendDlgItemMessage(hwnd, IDC_CHKEN, BM_SETCHECK, BST_CHECKED, 0L);
	SendDlgItemMessage(hwnd, IDC_CHKTP, BM_SETCHECK, BST_CHECKED, 0L);

	return TRUE;
}

void MainDlg_OnCommand (HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
	switch(id)
	{
		case IDC_OPTIONS:
			DialogBox(g_hInstance, MAKEINTRESOURCE(IDD_OPTIONSDLG), NULL, (DLGPROC)DialogProc_Options);
			break;
		case IDC_CPYURL:
			if(ListView_GetSelectedCount(hwndListView) > 0)
			{
				int i = ListView_GetNextItem(hwndListView, -1, LVNI_SELECTED);
				LPPXINFO lpnode = lppxinfo;
				while(lpnode && (i > 0))
				{
					lpnode = lpnode->next_node;
					i--;
				}
				if(lpnode && lpnode->lpszInfo)
				{
					HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE | GMEM_DDESHARE, strlen(lpnode->lpszInfo)+1);
					if(!hMem) break;
					strcpy((char*)GlobalLock(hMem), lpnode->lpszInfo);
					GlobalUnlock(hMem);
					if(OpenClipboard(hwnd))
					{
						if(EmptyClipboard())
						{
							SetClipboardData(CF_TEXT, lpnode->lpszInfo);
						}
						else
						{
							GlobalFree(hMem);
						}
						CloseClipboard();
					}
					else
					{
						GlobalFree(hMem);
					}
				}
			}
			break;
		case IDC_KILLTH:
			if(ListView_GetSelectedCount(hwndListView) > 0)
			{
				int i = ListView_GetNextItem(hwndListView, -1, LVNI_SELECTED);
				LPPXINFO lpnode = lppxinfo;
				while(lpnode && (i > 0))
				{
					lpnode = lpnode->next_node;
					i--;
				}
				if(lpnode)
				{
					SOCKET s;
					/*
					ListView_SetItemCount(hwndListView, ListView_GetItemCount(hwndListView)-1);
					if(lpnode->prev_node)
					{
						lpnode->prev_node->next_node = lpnode->next_node;
					}
					else
					{
						lppxinfo = lpnode->next_node;
					}

					if(lpnode->next_node)
					{
						lpnode->next_node->prev_node = lpnode->prev_node;
					}

					TerminateThread(lpnode->hThread, 0);
					*/
					s = lpnode->socket_rem;
					lpnode->socket_rem = INVALID_SOCKET;
					closesocket(s);
					s = lpnode->socket;
					lpnode->socket = INVALID_SOCKET;
					closesocket(s);
					/*
					if(lpnode->lpszInfo)
					{
						zfree(lpnode->lpszInfo);
					}
					if(lpnode->lpszStatus)
					{
						zfree(lpnode->lpszStatus);
					}
					zfree(lpnode);

					ListView_RedrawVisibleItems(hwndListView);
					*/
				}
			}
			break;
		case IDC_CHKTP:
			if(SendDlgItemMessage(hwnd, IDC_CHKTP, BM_GETCHECK, 0, 0L) == BST_CHECKED)
			{
				SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
			}
			else
			{
				SetWindowPos(hwnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
			}
			break;
		case IDC_CHKEN:
			if(SendDlgItemMessage(hwnd, IDC_CHKEN, BM_GETCHECK, 0, 0L) == BST_CHECKED)
			{
				unsigned long dwThreadId;
				hMutex = CreateMutex(NULL, TRUE, NULL);
				hThread = CreateThread(NULL, 0, PXMainThread, NULL, 0, &dwThreadId);
			}
			else
			{
				PXKillAll();
				ListView_SetItemCount(hwndListView, 0);
				ListView_RedrawVisibleItems(hwndListView);
			}
			break;
		case ID_MAINMENU_RESTORE:
			SendMessage(hwnd, WM_TRAY, (WPARAM) 1, (LPARAM) WM_LBUTTONDBLCLK);
			break;
		case ID_MAINMENU_EXIT:
			if(!IsWindowVisible(hwnd))
			{
				SendMessage(hwnd, WM_TRAY, (WPARAM) 1, (LPARAM) WM_LBUTTONDBLCLK);
			}
			PXShutDown();
			break;
		case IDC_HIDEPX:
		case IDCANCEL:
			ShowWindow(hwnd, SW_HIDE);
			Shell_NotifyIcon(NIM_ADD, &NotifyIconData);
			break;
	}
}

void MainDlg_TrayIconNotify (HWND hwnd, LONG wParam, LONG lParam)
{
	// wParam - icon uID;
	// lParam - WM_MESSAGE;
	switch (wParam)
	{
		case 1:
			switch (lParam)
			{
				case WM_LBUTTONDBLCLK:
					if(!IsWindowVisible(hwnd))
					{
						ShowWindow(hwnd, SW_SHOW);
						Shell_NotifyIcon(NIM_DELETE, &NotifyIconData);
					}
					if(IsIconic(hwnd))
					{
						ShowWindow(hwnd, SW_RESTORE);
					}
					break;
				case WM_RBUTTONDOWN:
					{
						POINT pt;
						GetCursorPos(&pt);
						SetForegroundWindow(hwnd);
						TrackPopupMenu(hMenuZero, TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, 0, hwnd, NULL);
						PostMessage(hwnd, WM_NULL, 0, 0L);
					}
					break;
			}
			break;
	}
}

INT_PTR CALLBACK MainDlg_DlgProc (HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
		HANDLE_MSG(hwnd, WM_COMMAND,	MainDlg_OnCommand);
		HANDLE_MSG(hwnd, WM_INITDIALOG,	MainDlg_OnInitDialog);
		case WM_NOTIFY:
			if(((LPNMHDR)lParam)->hwndFrom == hwndListView)
			{
				switch(((LPNMITEMACTIVATE)lParam)->hdr.code)
				{
					case LVN_GETDISPINFO:
						{
							LVITEM *lvItem = &((LV_DISPINFO *)lParam)->item;
							LPPXINFO lptemp = lppxinfo;
							int i = lvItem->iItem;
							while(lptemp && (i > 0))
							{
								lptemp = lptemp->next_node;
								i--;
							}

							if(!lptemp) break;

							switch(lvItem->iSubItem)
							{
								case 0:
									if(lvItem->mask & LVIF_TEXT)
									{
										if(lptemp->lpszInfo)
										{
											strncpy_tiny(lvItem->pszText, lptemp->lpszInfo, lvItem->cchTextMax -1);
										}
										else
										{
											lstrcpy(lvItem->pszText, "no info ready");
										}
									}
									break;
								case 1:
									if(lvItem->mask & LVIF_TEXT)
									{
										if(lptemp->lpszStatus)
										{
											strncpy_tiny(lvItem->pszText, lptemp->lpszStatus, lvItem->cchTextMax -1);
										}
									}
									break;
								case 2:
									if(lvItem->mask & LVIF_TEXT)
									{
										wsprintf(lvItem->pszText, "%d", lptemp->dwRX);
									}
									break;
								case 3:
									if(lvItem->mask & LVIF_TEXT)
									{
										wsprintf(lvItem->pszText, "%d", lptemp->dwTX);
									}
									break;
							}
						}
						return 0;
				}
			}
			break;
		case WM_QUERYENDSESSION:
			//If an application can terminate conveniently, it
			//should return TRUE; otherwise, it should return FALSE.
			SetWindowLong(hwnd, DWL_MSGRESULT, TRUE);
			return 1L;
		case WM_ENDSESSION:
			if(wParam) return (LPARAM) PXShutDown();
			break;
		case WM_SIZE:
			if(wParam == SIZE_MINIMIZED)
			{
				ShowWindow(hwnd, SW_HIDE);
				Shell_NotifyIcon(NIM_ADD, &NotifyIconData);
			}
			else
			{
				HWND hw;
				RECT rc;
				POINT lp;
				GetClientRect(hwnd, &rc);
				lp.x = 0; lp.y = 0;
				//MoveWindow(hwndListView, lp.x, lp.y, rc.right-90,rc.bottom-20, TRUE);
				SetWindowPos(hwndListView, 0, 0, 0, rc.right-90,rc.bottom-20, SWP_NOMOVE | SWP_NOZORDER );
				hw = GetDlgItem(hwnd, IDC_KILLTH);
				lp.x = 0; lp.y = 0;
				ClientToScreen(hw, &lp);
				ScreenToClient(hwnd, &lp);
				SetWindowPos(hw, 0, rc.right-70, lp.y, 0, 0, SWP_NOSIZE | SWP_NOZORDER );
				hw = GetDlgItem(hwnd, IDC_CPYURL);
				lp.x = 0; lp.y = 0;
				ClientToScreen(hw, &lp);
				ScreenToClient(hwnd, &lp);
				SetWindowPos(hw, 0, rc.right-70, lp.y, 0, 0, SWP_NOSIZE | SWP_NOZORDER );
				hw = GetDlgItem(hwnd, IDC_HIDEPX);
				lp.x = 0; lp.y = 0;
				ClientToScreen(hw, &lp);
				ScreenToClient(hwnd, &lp);
				SetWindowPos(hw, 0, rc.right-70, lp.y, 0, 0, SWP_NOSIZE | SWP_NOZORDER );
				hw = GetDlgItem(hwnd, IDC_INFO);
				lp.x = 0; lp.y = 0;
				ClientToScreen(hw, &lp);
				ScreenToClient(hwnd, &lp);
				SetWindowPos(hw, 0, rc.right-70, lp.y, 0, 0, SWP_NOSIZE | SWP_NOZORDER );
				hw = GetDlgItem(hwnd, IDC_OPTIONS);
				lp.x = 0; lp.y = 0;
				ClientToScreen(hw, &lp);
				ScreenToClient(hwnd, &lp);
				SetWindowPos(hw, 0, rc.right-70, lp.y, 0, 0, SWP_NOSIZE | SWP_NOZORDER );
				hw = GetDlgItem(hwnd, ID_MAINMENU_EXIT);
				lp.x = 0; lp.y = 0;
				ClientToScreen(hw, &lp);
				ScreenToClient(hwnd, &lp);
				SetWindowPos(hw, 0, rc.right-70, lp.y, 0, 0, SWP_NOSIZE | SWP_NOZORDER );
				hw = GetDlgItem(hwnd, IDC_BUILD);
				lp.x = 0; lp.y = 0;
				ClientToScreen(hw, &lp);
				ScreenToClient(hwnd, &lp);
				SetWindowPos(hw, 0, rc.right-70, lp.y, 0, 0, SWP_NOSIZE | SWP_NOZORDER );
				hw = GetDlgItem(hwnd, IDC_REMOTE);
				lp.x = 0; lp.y = 0;
				ClientToScreen(hw, &lp);
				ScreenToClient(hwnd, &lp);
				SetWindowPos(hw, 0, rc.right-70, lp.y, 0, 0, SWP_NOSIZE | SWP_NOZORDER );
				hw = GetDlgItem(hwnd, IDC_LOCALE);
				lp.x = 0; lp.y = 0;
				ClientToScreen(hw, &lp);
				ScreenToClient(hwnd, &lp);
				SetWindowPos(hw, 0, rc.right-70, lp.y, 0, 0, SWP_NOSIZE | SWP_NOZORDER );
				hw = GetDlgItem(hwnd, IDC_CHKTP);
				lp.x = 0; lp.y = 0;
				ClientToScreen(hw, &lp);
				ScreenToClient(hwnd, &lp);
				SetWindowPos(hw, 0, rc.right-70, lp.y, 0, 0, SWP_NOSIZE | SWP_NOZORDER );
				hw = GetDlgItem(hwnd, IDC_CHKEN);
				lp.x = 0; lp.y = 0;
				ClientToScreen(hw, &lp);
				ScreenToClient(hwnd, &lp);
				SetWindowPos(hw, 0, rc.right-70, lp.y, 0, 0, SWP_NOSIZE | SWP_NOZORDER );
			}
			break;
		case WM_LBUTTONDOWN:
			PostMessage(hwnd, WM_NCLBUTTONDOWN, HTCAPTION, lParam);
			break;
		default:
			if(uMsg == WM_TRAY)
			{
				MainDlg_TrayIconNotify(hwnd, wParam, lParam);
			}
	}
	return 0L;
}
#endif

/*
unsigned long installtosystem(unsigned long dwOption, LPCSTR cmdln, char *keyname, char *filename)
{
	// скопировать в указанное место и под заданным именем
	// занести в автозагрузку (сервис)
	// запустить немедленно
	// завершить текущее приложение

	unsigned long sysdir_len, filename_len;
	char *sysdir, *sysdirfile;
	
	sysdir_len = GetSystemDirectory(NULL, 0);
	filename_len = strlen(filename);
	if((sysdir_len+filename_len) > MAX_PATH)
	{
		return 1L;
	}

	sysdir = (char *) zalloc(sysdir_len+filename_len+1);
	if(sysdir)
	{
		GetSystemDirectory(sysdir, sysdir_len);
		strcat(sys_path, filename);

		HKEY hSubKey;
		//Software\\Microsoft\\Windows\\CurrentVersion\\Run
		//Software\\Microsoft\\Windows\\CurrentVersion\\RunServices
		if(RegOpenKey(HKEY_LOCAL_MACHINE, "Software\\Microsoft\\Windows\\CurrentVersion\\RunServices", &hSubKey) == ERROR_SUCCESS)
		{
			RegSetValueEx(hSubKey, keyname, NULL, REG_SZ, (BYTE*)sys_path, tmp);
			RegCloseKey(hSubKey);
		}

		if(cmdln[0] == '\"')
		{
			tmp = (unsigned long) strchr(cmdln+1, '\"');
			if(!tmp)
			{
				zfree(sys_path);
				return 2L;
			}
			tmp = tmp - (unsigned long)(cmdln +1);
		}
		else if(tmp = (unsigned long) strchr(cmdln, ' '))
		{
			tmp = tmp - (unsigned long)(cmdln);
		}
		else
		{
			tmp = strlen(cmdln);
		}

		char * thisfile = (char *) zalloc(tmp+1);
		if(thisfile)
		{
			if(cmdln[0] == '\"')
			{
				strncpy(thisfile, cmdln+1, tmp);
			}
			else
			{
				strncpy(thisfile, cmdln, tmp);
			}
			thisfile[tmp] = '\0';

			CopyFile(thisfile, sys_path, TRUE);

			zfree(thisfile);
		}

		zfree(sys_path);

		return 0L;
	}

	return 3L;
}
/**/

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	/*
	LP_QUERY_STRING_NODE node = NULL;

	url_string_parse(&node, "/i/i-help2.gif");
	//url_string_parse(&node, "http://user:pass@www.ya.ru:8080/server/test.php?query=23321#frag12");
	return 0;
	/**/

#ifndef PX_GUI
	SetErrorMode(SEM_NOGPFAULTERRORBOX);
#endif

	WSADATA wsaData;
	unsigned long dwThreadId;
	int exit_code;
	char *temp_str;
	char *log_path;

	register char *error_msg;
	unsigned long stack[ZL_STACK_SIZE];
	unsigned long regs[REG_COUNT];

	char alias[80];
	addrinfo ai_req, *ai_temp, *ai_res;
	//char *temp_str;
	//hostent *he;
	//int i;

	MSG msg;

	g_hInstance = hInstance;
	exit_code = 0;
	ini_path = nullstring;
	log_path = nullstring;

	if(WSAStartup(0x0202, &wsaData) != 0)
	{
		return 1;
	}

	if(wsaData.wVersion != 0x0202)
	{
		exit_code = 1;
		goto lb_exit1;
	}

	temp_str = (char *) zalloc(MAX_PATH);
	if(temp_str)
	{
		GetModuleFileName(hInstance, temp_str, MAX_PATH);
		ini_path = alloc_filepath(temp_str, FPI_DIR | FPI_FNAME);
		zfree(temp_str);
	}

	log_path = alloc_string(ini_path);
	alloc_strcat(&ini_path, ".ini");
	alloc_strcat(&log_path, ".log");

	//hFile = CreateFile(szLogPath, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	hFile = _openlog(log_path, 1);
	_logd(hFile, g_options & PXO_DEBUG, 0, "starting");

	ini_load(&ini_node, ini_path);

	g_options = ini_get_long(ini_node, "Options", "options", g_options);
	dwThreadSleep = ini_get_long(ini_node, "Options", "dwThreadSleep", dwThreadSleep);
	bind_addressport = ini_alloc_get_string(ini_node, "Options", "bindport", PX_LISTENPORT);
	proxy_script = ini_alloc_get_string(ini_node, "Options", "redir_header", PX_REDIR_HEADER);
	//proxyport = (unsigned short) ini_get_long(ini_node, "Options", "pxport", PX_PORT);
	//redirport = (unsigned short) ini_get_long(ini_node, "Options", "redir_port", PX_REDIR_PORT);
	//lppxhost = ini_alloc_get_string(ini_node, "Options", "redir_host", PX_REDIR_HOST);

#ifdef PX_GUI
	WM_TRAY = RegisterWindowMessage("proxy_rd_tray_icon");
	//*
	if(~g_options & PXO_MULTIINSTANCE)
	{
		hwndMainDlg = FindWindow("#32770", "Free.NET redirect proxy 2002 (c)oded by dEmon");
		if(hwndMainDlg != NULL)
		{
			if(IsIconic(hwndMainDlg) || !IsWindowVisible(hwndMainDlg))
			{
				SendMessage(hwndMainDlg, WM_TRAY, (WPARAM) 1, (LPARAM) WM_LBUTTONDBLCLK);
			}
			SetForegroundWindow(hwndMainDlg);
			//exit_code = 1;
			goto lb_exit2;
		}
	}
	/**/
#endif

	myaddr = NULL;
	temp_str = alloc_string("localhost");
	if(temp_str)
	{
		list_insert(&myaddr, temp_str);
	}

	temp_str = alloc_string("127.0.0.1");
	if(temp_str)
	{
		list_insert(&myaddr, temp_str);
	}

	if(!gethostname(alias, sizeof(alias)))
	{
		memset(&ai_req, 0, sizeof(addrinfo));
		ai_req.ai_family = PF_INET;
		ai_req.ai_socktype = SOCK_STREAM;

		temp_str = alloc_string(alias);
		if(temp_str)
		{
			list_insert(&myaddr, temp_str);
		}

		if(!getaddrinfo(alias, NULL, &ai_req, &ai_res))
		{
			ai_temp = ai_res;
			while(ai_temp)
			{
				*alias = '\0';
				temp_str = alloc_string(inet_ntoa(((sockaddr_in *) ai_temp->ai_addr)->sin_addr));
				if(temp_str)
				{
					list_insert(&myaddr, temp_str);
				}
				if(!getnameinfo(ai_temp->ai_addr, ai_temp->ai_addrlen, alias, sizeof(alias), NULL, 0, 0))
				{
					if(*alias)
					{
						temp_str = alloc_string(alias);
						if(temp_str)
						{
							list_insert(&myaddr, temp_str);
						}
					}
				}

				ai_temp = ai_temp->ai_next;
			}

			freeaddrinfo(ai_res);
		}
	}

	if(g_options & PXO_INSTALL)
	{
		installtosystem("prd", "prd.exe");
	}

	//if(zl_compile(&hardcode, &data_table, /*&vars_count,*/ fn_list, lpheadertemplate, &vars_map, &error_msg))
	if(zl_compile(&hardcode, &hard_code_size, proxy_script, &error_msg,
		&const_sect, &const_size,
		&data_sect, &data_size,
		&reloc_sect, &reloc_size,
		&import_sect, &import_size,
		&export_sect, &export_size,
		&map_sect, &map_size
		))
	{
#if defined(_DEBUG) && defined(PX_GUI)
		MessageBox(NULL, error_msg, "zlc: compilation error", MB_OK);
#endif
		_logd(hFile, g_options & PXO_DEBUG, 0, "zlc: compilation error");
		_logd(hFile, g_options & PXO_DEBUG, 0, error_msg);
		free_str(error_msg);
	}
	else
	{
		zl_load_functions(import_sect, (zl_map_section *) map_sect, map_size/sizeof(zl_map_section), fn_list, &modules);

		zl_init(zl_offset("main", (zl_export_section *) export_sect, export_size/sizeof(zl_export_section)), hardcode, stack, regs, const_sect, data_sect, reloc_sect, import_sect);
		zl_call(regs);
	}


#ifdef PX_GUI
	RegisterMyControlClasses();

	//DialogBox(hInstance, MAKEINTRESOURCE(IDD_STATUSDLG), NULL, (DLGPROC)MainDlg_DlgProc);
	hwndMainDlg = CreateDialog(hInstance, MAKEINTRESOURCE(IDD_STATUSDLG), NULL, (DLGPROC) MainDlg_DlgProc);

	if(!(g_options & PXO_HIDDEN)) ShowWindow(hwndMainDlg, SW_SHOW);
#endif

	hMutex = CreateMutex(NULL, TRUE, NULL);
	hThread = CreateThread(NULL, 0, PXMainThread, NULL, 0, &dwThreadId);

	while(GetMessage(&msg, NULL, 0L, 0L) > 0)
	{
#ifdef PX_GUI
		//if(!TranslateAccelerator(hwndMainDlg, hAccelTable, &msg))
		//{
			if(!IsDialogMessage(hwndMainDlg, &msg))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		//}
#endif
	}

	//MessageBox(NULL, "Normal exit", "WinMain", MB_OK);

	if(hMutex)
	{
		ReleaseMutex(hMutex);
		CloseHandle(hMutex);
	}

	ini_put_string(&ini_node, "Options", "options", NULL, "%d", g_options);
	//ini_put_string(&ini_node, "Options", "redir_port", NULL, "%d", redirport);
	//ini_put_string(&ini_node, "Options", "pxport", NULL, "%d", proxyport);
	ini_put_string(&ini_node, "Options", "bindport", NULL, "%s", bind_addressport);
//	ini_put_string(&ini_node, "Options", "redir_host", NULL, "%s", lppxhost);
	ini_put_string(&ini_node, "Options", "redir_header", NULL, "%s", proxy_script);
	
	ini_flush(ini_node, ini_path, FALSE);

	_logd(hFile, g_options & PXO_DEBUG, 0, "normal shutdown");

	if(hThread)
	{
		ZTRACE("Waiting thread exit...\n");
		if(WaitForSingleObject(hThread, 180000) == WAIT_TIMEOUT)
		{
			ZTRACE("Terminating thread...\n");
			TerminateThread(hThread, 0);
		}
		CloseHandle(hThread);
	}

#ifdef PX_GUI
	//DestroyWindow(hwndMainDlg);
#endif

//lb_exit3:
	zl_unload_modules(modules);
	//zl_free(&hardcode, NULL, &data_table, &vars_map);
	zl_free(hardcode, const_sect, data_sect, reloc_sect, import_sect, export_sect, map_sect);

	list_free(&myaddr, cb_addr_free);

lb_exit2:
	//free_str(lppxhost);
	free_str(bind_addressport);
	free_str(proxy_script);
	free_str(ini_path);
	free_str(log_path);

	ini_free(&ini_node);

	//CloseHandle(hFile);
	_closelog(hFile, 1);

lb_exit1:
	WSACleanup();

	return 0;
}
