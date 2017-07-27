// proxy_rd script for nautilus2001 - unsafe
// require proxy_rd build 945

import __cdecl dup(1);
import __cdecl add(2);
import __cdecl free(1);
import __cdecl encode(1);
import __cdecl msg(2);
import __cdecl isempty(1);
import __cdecl isset(1);
import __cdecl strstr(2);
import __cdecl strstri(2);
import __cdecl dupn(1);

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
	msg("proxy_rd: Внимание!","Внимание!\nВы запустили proxy_rd в быстром, но НЕ БЕЗОПАСНОМ режиме!\nРекомендую воздержаться от посещения сайтов с личной информацией\n(таких как mail.ru, odnoklassniki.ru и другие).");

	/*
	char ***res;
	char *test;
	test = "http://www.pfzim.ru/download.php?id=9";

	if(pcre_match_all("^(?:http://)?([^/]+)", test, strlen(test), 0, 0, 0, &res) > 0)
	{
		msg("proxy_rd: pcre test", res[0][0]);
		pcre_match_free(res);
	}
	*/
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

	header = NULL;

	if(isempty(host))
	{
		msg("proxy_rd debug msg", "strange: host not defined!");
	}

	// connect to host through other proxy
	//*rdhost = dup("172.30.6.171");
	//*rdhost = dup("172.30.2.161");		// Sera is 8445 - unlim
	//*rdhost = dup("172.30.4.245");		// Kaplya is 0510 - optmalny
	*rdhost = dup("172.30.5.178");		// Zadorina is 5555 - unlim
	//*rdhost = dup("172.30.7.76");			// Lity is 6677 - ulnim 3048

	*rdport = dup("3128");

	/*
	if(*options & 0x04)
	{
		// connect method no need header - exit
		// return NULL;
		//*options &= ~0x00000004;
	}
	*/

	add(&header, method);
	add(&header, " ");
	add(&header, uri_relative);
	add(&header, " ");
	add(&header, protocol);
	add(&header, "\r\n");

	if(!isempty(h_host_port))
	{
		add(&header, "Host: ");
		add(&header, h_host_port);
		add(&header, "\r\n");
	}

	add(&header, "Connection: close\r\n");

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

	//msg("proxy_rd: debug message", header);

	*header_len = strlen(header);
	return header;
}
