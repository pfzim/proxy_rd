// proxy_rd script for nautilus2001 - file POST method with crypt - beta - part 1
// require proxy_rd build 945

import __cdecl msg(2);
import __cdecl isempty(1);
import __cdecl dup(1);

function main()
{
	msg("proxy_rd: Информация","Вы запустили Модуль 1.\nТеперь запустите Модуль 2!\n\nproxy_rd работает в медленном, но более защищенном режиме!\n");
}

function proxy_rd(
	unsigned long *header_len, char **post_data, unsigned long *post_data_len, char **footer, unsigned long *footer_len,
	unsigned long *options, char **rdhost, char **rdport,
	char *method, char *protocol, char *uri_full, char *uri_relative, char *uri_full_encoded,
	char *host, char *build, char *h_hostport, char *h_unknown, char *h_cookie, char *h_accept,
	char *h_accept_lang, char *h_accept_charset, char *h_if_mod_since, char *h_if_none_match,
	char *h_referer, char *h_user_agent, char *h_proxy_connection, char *h_keep_alive,
	char *h_authorization, char *h_cache_control, char *h_pragma, char *h_range, char *h_content_type,
	char *h_content_length, char *port, char *query, char *scheme, char *authority, char *path, char *fragment
)
{
	*rdhost = dup("localhost");
	*rdport = dup("3129");

	if(*options & 0x04)
	{
		// connect method no need header - exit
		// return NULL;

		// here we say:
		// don't answer on CONNECT header
		*options &= ~0x00000004;
	}

	return NULL;
}
