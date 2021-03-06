// script only for me mcep.ru secondary - transite
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
	header = NULL;

	*rdhost = dup(host);
	if(!isempty(port))
	{
		*rdport = dup(port);
	}

	if(*options & 0x04)
	{
		// connect method no need header - exit
		// return NULL;

		// here we say:
		// don't answer on CONNECT header
		// and don't resend CONNECT header
		*options &= ~(0x00000004 | 0x00000080);
	}

	return header;
}
