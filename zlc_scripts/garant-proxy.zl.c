// script special for proxy.garant.ru
// require proxy_rd build 890

// in parameters
extern char *method, *protocol, *uri_full, *uri_relative, *uri_full_encoded;
extern char *host, *port, *build, *h_hostport, *h_unknown, *h_cookie, *h_accept;
extern char *h_accept_lang, *h_accept_charset, *h_if_mod_since, *h_if_none_match;
extern char *h_referer, *h_user_agent, *h_proxy_connection, *h_keep_alive;
extern char *h_authorization, *h_cache_control, *h_pragma, *h_range, *h_content_type;
extern char *h_content_length, *scheme, *authority, *path, *query, *fragment;
extern unsigned long options;

// out parameters
extern char *header, *rdhost, *rdport;

if(isempty(host))
{
	msg("proxy_rd debug msg", "strange: host not defined!");
}

// connect to host trough other proxy with modified uri
rdhost = dup("proxy.garant.ru");
rdport = dup("8080");

add(&header, method);
add(&header, " ");
add(&header, uri_full);
if(isempty(fragment))
{
	if(!isempty(query))
	{
		add(&header, "&");
	}
	else
	{
		add(&header, "?");
	}
}
add(&header, "microsoft ");  // magic keyword allowed by proxy firewall
add(&header, protocol);
add(&header, "\r\n");

if(options & 0x04)
{
	// connect method no need header - exit
	// via over proxy - need!
	// goto lb_exit;
}

if(!isempty(h_hostport))
{
	add(&header, "Host: ");
	add(&header, h_hostport);
	add(&header, "\r\n");
}

add(&header, "Proxy-Connection: close\r\n");
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

lb_exit:
