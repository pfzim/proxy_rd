// *** sample code **********************************************************
#ifdef zlc_demo_script_code

// proxy_rd demo script
// require proxy_rd build 945

import __cdecl dup(1);		// char *dup(char *src);
import __cdecl add(2);		// char *add(char **dst, char *src);
import __cdecl free(1);		// void free(char *src);
import __cdecl encode(1);	// char *encode(char *src);
import __cdecl encoden(2);	// char *encode(char *src, unsigned long len);
import __cdecl msg(2);		// void msg(char *title, char *text);
import __cdecl yes(2);		// bool yes(char *title, char *text);
import __cdecl isempty(1);	// bool isempty(char *src);
import __cdecl isset(1);	// bool isset(char *src);
import __cdecl strstr(2);	// char *strstr(char *src, char *key);
import __cdecl strstri(2);	// char *strstri(char *src, char *key);
import __cdecl dupn(1);		// char *dupn(unsigned long n);
import __cdecl crypt(3);	// char *crypt(char *buf, unsigned long len, unsigned long en_pos);

import __cdecl strcmp(2);	// bool strcmp(char *src1, char *src2);
import __cdecl stricmp(2);	// bool stricmp(char *src1, char *src2);
import __cdecl htmlencode(1);	// char *htmlencode(char *src);
import __cdecl atoi(1);		// int atoi(char *src);
import __cdecl zalloc(1);	// void *zalloc(unsigned long size);
import __cdecl zrealloc(2);	// void *zrealloc(void *data, unsigned long size);
import __cdecl zfree(1);	// void zfree(void *data);
import __cdecl pcre_match_all(7);	// int pcre_match_all(char *pattern, char *buffer, int buf_size, int options, int exoptions, int find_all, char ***res[]);
import __cdecl pcre_match_free(1);	// void pcre_match_free(char ***res);
import __cdecl strlen(1);	// int strlen(char *src);

/*
// in parameters
extern char *method, *protocol, *uri_full, *uri_relative, *uri_full_encoded;
extern char *host, *port, *build, *h_host_port, *h_unknown, *h_cookie, *h_accept;
extern char *h_accept_lang, *h_accept_charset, *h_if_mod_since, *h_if_none_match;
extern char *h_referer, *h_user_agent, *h_proxy_connection, *h_keep_alive;
extern char *h_authorization, *h_cache_control, *h_pragma, *h_range, *h_content_type;
extern char *h_content_length, *scheme, *authority, *path, *query, *fragment;
extern unsigned long options;

// out parameters
extern char *header, *rdhost, *rdport;
*/

function main()
{
	// initialise global variables here
	//msg("proxy_rd","Hello, world!");

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

	// temporary variables
	unsigned long blocked_site, hack_method, use_proxy;
	char **site;
	char *temp;

	header = NULL;
	hack_method = 2;	// can be 1, 2 or 3
	use_proxy = 0;		// can be 0 or 1

	char *blocked_sites_list[] =
	{
		"odnoklassniki.ru",
		"vkontakte.ru",
		"mail.ru",
		NULL
	};

	if(isempty(host))
	{
		msg("proxy_rd debug msg", "strange: host not defined!");
	}

	blocked_site = 0;
	site = blocked_sites_list;

	while(*site)
	{
		if(strstri(host, *site))
		{
			blocked_site = 1;
			break;
		}
		site++;
	}

	if(blocked_site)
	{
		if(hack_method == 1)
		{
			// direct connect to host with modified uri
			*rdhost = dup(host);
			if(!isempty(port))
			{
				*rdport = dup(port);
			}

			add(&header, method);
			add(&header, " ");
			add(&header, uri_relative);
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
			//add(&header, "analit ");
			add(&header, protocol);
			add(&header, "\r\n");
		}
		else if(hack_method == 2)
		{
			// connect to host through proxy script
			*rdhost = dup("www.pfzim.ru");

			add(&header, method);
			add(&header, " http://www.pfzim.ru/funcs/redir_nx.php?id=");
			add(&header, uri_full_encoded);
			add(&header, "&md=");
			add(&header, temp = encode(method)); free(temp);
			add(&header, "&pl=");
			add(&header, temp = encode(protocol)); free(temp);
			add(&header, "&st=on HTTP/1.0\r\n");
		}
		else if(hack_method == 3)
		{
			// connect to host through other proxy
			*rdhost = dup("proxyserver");
			*rdport = dup("3128");

			add(&header, method);
			add(&header, " ");
			add(&header, uri_relative);
			add(&header, " ");
			add(&header, protocol);
			add(&header, "\r\n");
		}
	}
	else
	{
		// connect to not bocked sites
		if(use_proxy)
		{
			// connect through other proxy
			*rdhost = dup("proxyserver");
			*rdport = dup("3128");
		}
		else
		{
			// direct connect to host
			*rdhost = dup(host);
			if(!isempty(port))
			{
				*rdport = dup(port);
			}

			if(*options & 0x04)
			{
				// connect method no need header - exit
				return NULL;
			}
		}

		add(&header, method);
		add(&header, " ");
		add(&header, uri_relative);
		add(&header, " ");
		add(&header, protocol);
		add(&header, "\r\n");
	}

	if(!isempty(h_host_port))
	{
		add(&header, "Host: ");
		if(blocked_site && (hack_method == 2))
		{
			add(&header, *rdhost);
			if(!isempty(*rdport))
			{
				add(&header, ":");
				add(&header, *rdport);
			}
		}
		else
		{
			add(&header, h_host_port);
		}
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

	if(header)
	{
		*header_len = strlen(header);
	}
	return header;
}

#endif
// *** sample code **********************************************************

