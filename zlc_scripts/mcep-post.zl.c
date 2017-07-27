// script only for me mcep.ru - file POST method with crypt - beta
// require proxy_rd build 945

import __cdecl dup(1);
import __cdecl add(2);
import __cdecl free(1);
import __cdecl encode(1);
import __cdecl encoden(2);
import __cdecl msg(2);
import __cdecl yes(2);
import __cdecl isempty(1);
import __cdecl isset(1);
import __cdecl strstr(2);
import __cdecl strstri(2);
import __cdecl dupn(1);
import __cdecl crypt(3);

import __cdecl strcmp(2);
import __cdecl stricmp(2);
import __cdecl htmlencode(1);
import __cdecl atoi(1);
import __cdecl zalloc(1);
import __cdecl zrealloc(2);
import __cdecl zfree(1);
import __cdecl pcre_match_all(7);
import __cdecl pcre_match_free(1);
import __cdecl strlen(1);

function main()
{
	// initialise global variables here
	//msg("proxy_rd","Hello, world!");
}

function proxy_rd(
	unsigned long *header_len, char **post_data, unsigned long *post_data_len, char **footer, unsigned long *footer_len,
	unsigned long *options, char **rdhost, char **rdport,
	char *method, char *protocol, char *uri_full, char *uri_relative, char *uri_full_encoded,
	char *host, char *build, char *h_host_port, char *h_unknown, char *h_cookie, char *h_accept,
	char *h_accept_lang, char *h_accept_charset, char *h_if_mod_since, char *h_if_none_match,
	char *h_referer, char *h_user_agent, char *h_proxy_connection, char *h_keep_alive,
	char *h_authorization, char *h_cache_control, char *h_pragma, char *h_range, char *h_content_type,
	char *h_content_length, char *port, char *query, char *scheme, char *authority, char *path, char *fragment
)
{
	char *header;
	char *header2;
	char *temp;
	char *temp2;
	unsigned long crypt_offset;
	unsigned long crypt_size;

	header = NULL;
	header2 = NULL;

	if(isempty(host))
	{
		msg("proxy_rd debug msg", "strange: host not defined!");
	}

	if(*options & 0x04)
	{
		// connect method no need header - exit
		return NULL;
	}

	// connect to host trough proxy script
	//*rdhost = dup("www.pfzim.ru");
	*rdhost = dup("intweb");
	//*rdhost = dup("localhost");
	//*rdport = dup("3129");

	//add(&header2, method);
	//add(&header2, " http://www.pfzim.ru/funcs/redir_nx.php?id=");
	//add(&header2, " http://localhost/lab/redir_nx.php?id=");
	//add(&header2, " http://intweb/bitrix/tools/redir_nx.php?id=");
	add(&header2, "POST http://intweb/bitrix/tools/redir_nx.php?id=aHR0cDovL3d3dy5leGFtcGxlLmNvbS8%3D&px=");
	temp = dup(host);
	add(&temp, ":");
	add(&temp, isempty(port)?"80":port);
	temp2 = encode(temp);
	add(&header2, temp2);
	free(temp);
	free(temp2);
	add(&header2, "&pm=on&log=off&ci=on&co=on&filelog=off HTTP/1.0\r\n");

	//add(&header2, "Host: www.pfzim.ru\r\n");
	add(&header2, "Host: intweb\r\n");
	//add(&header2, "Host: localhost\r\n");
	//add(&header2, "Proxy-Connection: close\r\n");
	add(&header2, "Connection: close\r\n");
	add(&header2, "Content-Type: multipart/form-data; boundary=-----------------------------e5ab6264db2c4f4\r\n");

	add(&header, "-------------------------------e5ab6264db2c4f4\r\nContent-Disposition: form-data; name=\"request\"; filename=\"request.txt\"\r\nContent-Type: application/octet-stream\r\n\r\n");
	add(footer, "\r\n-------------------------------e5ab6264db2c4f4--\r\n");
	
	crypt_offset = strlen(header);

	//add(&header, temp2);
	
	add(&header, method);
	add(&header, " ");
	add(&header, uri_relative);
	add(&header, " ");
	add(&header, protocol);
	add(&header, "\r\n");

	add(&header, "Connection: close\r\n");

	if(!isempty(h_host_port))
	{
		add(&header, "Host: ");
		add(&header, h_host_port);
		add(&header, "\r\n");
	}

	if(!isempty(h_cache_control))
	{
		add(&header, "Cache-Control: ");
		add(&header, h_cache_control);
		add(&header, "\r\n");
	}

	if(!isempty(h_pragma))
	{
		add(&header, "Pragma: ");
		add(&header, h_pragma);
		add(&header, "\r\n");
	}

	if(!isempty(h_accept))
	{
		add(&header, "Accept: ");
		add(&header, h_accept);
		add(&header, "\r\n");
	}

	if(!isempty(h_accept_lang))
	{
		add(&header, "Accept-Language: ");
		add(&header, h_accept_lang);
		add(&header, "\r\n");
	}

	if(!isempty(h_accept_charset))
	{
		add(&header, "Accept-Charset: ");
		add(&header, h_accept_charset);
		add(&header, "\r\n");
	}

	if(!isempty(h_if_mod_since))
	{
		add(&header, "If-Modified-Since: ");
		add(&header, h_if_mod_since);
		add(&header, "\r\n");
	}

	if(!isempty(h_if_none_match))
	{
		add(&header, "If-None-Match: ");
		add(&header, h_if_none_match);
		add(&header, "\r\n");
	}

	if(!isempty(h_referer))
	{
		add(&header, "Referer: ");
		add(&header, h_referer);
		add(&header, "\r\n");
	}

	if(!isempty(h_user_agent))
	{
		add(&header, "User-Agent: ");
		add(&header, h_user_agent);
		add(&header, "\r\n");
	}

	if(!isempty(h_authorization))
	{
		add(&header, "Authorization: ");
		add(&header, h_authorization);
		add(&header, "\r\n");
	}

	if(!isempty(h_range))
	{
		add(&header, "Range: ");
		add(&header, h_range);
		add(&header, "\r\n");
	}

	if(!isempty(h_content_type))
	{
		add(&header, "Content-Type: ");
		add(&header, h_content_type);
		add(&header, "\r\n");
	}

	if(!isempty(h_content_length))
	{
		add(&header, "Content-Length: ");
		add(&header, h_content_length);
		add(&header, "\r\n");
	}

	if(!isempty(h_cookie))
	{
		add(&header, "Cookie: ");
		add(&header, h_cookie);
		add(&header, "\r\n");
	}

	add(&header, h_unknown);

	add(&header, "\r\n");

	*header_len = strlen(header);
	*footer_len = strlen(*footer);

	add(&header2, "Content-Length: ");
	temp = dupn(*post_data_len + *header_len + *footer_len);
	add(&header2, temp);
	free(temp);
	add(&header2, "\r\n\r\n");

	crypt_size = *header_len - crypt_offset;
	crypt_offset += strlen(header2);
	*header_len += strlen(header2);
	
	add(&header2, header);
	free(header);
	header = header2;

	crypt(header + crypt_offset, crypt_size, 0);
	if(*post_data_len > 0)
	{
		crypt(*post_data, *post_data_len, crypt_size % 256);
	}
	
	//msg("proxy_rd: debug message", header);

	return header;
}
