// script only for me mcep.ru - uri method
// require proxy_rd build 941

import __cdecl dup(1);
import __cdecl add(2);
import __cdecl free(1);
import __cdecl encode(1);
import __cdecl msg(2);
import __cdecl isempty(1);

function main()
{
	// initialise global variables here
	//msg("proxy_rd","Hello, world!");
}

function proxy_rd(
	unsigned long *options, char **rdhost, char **rdport,
	char *method, char *protocol, char *uri_full, char *uri_relative, char *uri_full_encoded,
	char *host, char *build, char *h_hostport, char *h_unknown, char *h_cookie, char *h_accept,
	char *h_accept_lang, char *h_accept_charset, char *h_if_mod_since, char *h_if_none_match,
	char *h_referer, char *h_user_agent, char *h_proxy_connection, char *h_keep_alive,
	char *h_authorization, char *h_cache_control, char *h_pragma, char *h_range, char *h_content_type,
	char *h_content_length, char *port, char *query, char *scheme, char *authority, char *path, char *fragment
)
{
	char *header;
	char *temp;

	header = NULL;

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

	add(&header, method);
	//add(&header, " http://www.pfzim.ru/funcs/redir_nx.php?id=");
	//add(&header, " http://localhost/lab/redir_nx.php?id=");
	add(&header, " http://intweb/bitrix/tools/redir_nx.php?id=");
	add(&header, uri_full_encoded);
	add(&header, "&md=");
	add(&header, temp = encode(method)); free(temp);
	add(&header, "&pl=");
	add(&header, temp = encode(protocol)); free(temp);
	add(&header, "&log=off&co=on&filelog=off HTTP/1.0\r\n");

	//add(&header, "Host: www.pfzim.ru\r\n");
	add(&header, "Host: intweb\r\n");
	//add(&header, "Host: localhost\r\n");

	//add(&header, "Proxy-Connection: close\r\n");
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

	return header;
}
