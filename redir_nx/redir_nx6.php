<?php
   /*
    *	free.net url redirecter v0.001b (c)oded by dEmon in 2002 [unix]
	*/
	
	// sample script for download files with resume!

	error_reporting (E_ALL);

function print_error($hdr, $title, $text, $info, $st)
{
	$string = "<!-- ".htmlspecialchars($info)." --><html><head><title>".htmlspecialchars($title)."</title></head><body><p><strong>".htmlspecialchars($text)."</strong></p></body></html>";
	/*
	if($st)
	{
	    if(!headers_sent())
	    {
			header ($hdr);
			header ("Content-Type: text/html");
			header ("Content-Length: ".strlen($string));
		}
	}
	else
	{
	//*/
		echo $hdr;
		echo "Content-Type: text/html";
		echo "Content-Length: ".strlen($string);
	//}
	
	echo $string;
}

function create_header($method, $host, $uri, $protocol, $aut, $useaut)
{
	if(empty($method))
	{
		if(!empty($_SERVER["REQUEST_METHOD"]))
		{
    		$method = $_SERVER["REQUEST_METHOD"];
		}
		else
		{
			$method = "GET";
		}
	}
	
	if(empty($protocol))
	{
		if(!empty($_SERVER["SERVER_PROTOCOL"]))
		{
    		$protocol = $_SERVER["SERVER_PROTOCOL"];
		}
		else
		{
			$protocol = "HTTP/1.0";
		}
	}

	// new
	$uri_arr = parse_url($uri);
	$uri = $uri_arr["path"].(isset($uri_arr["query"])?("?".$uri_arr["query"]):"");

	$hd = $method." ".$uri." ".$protocol."\r\n";

	// general header
	if(!empty($_SERVER["HTTP_CACHE_CONTROL"]))
	{
		$hd .=	"Cache-Control: ".$_SERVER["HTTP_CACHE_CONTROL"]."\r\n";
	}
	$hd .=	"Connection: close\r\n";
	// Date
	if(!empty($_SERVER["HTTP_PRAGMA"]))
	{
		$hd .=	"Pragma: ".$_SERVER["HTTP_PRAGMA"]."\r\n";
	}
	// Transfer-Encoding
	// Upgrade
	if(!empty($_SERVER["HTTP_VIA"]))
	{
		$hd .=	"Via: ".$_SERVER["HTTP_VIA"]."\r\n";
	}
	// Warning

	// request header
	if(!empty($_SERVER["HTTP_ACCEPT"]))
	{
		$hd .=	"Accept: ".$_SERVER["HTTP_ACCEPT"]."\r\n";
	}
	if(!empty($_SERVER["HTTP_ACCEPT_CHARSET"]))
	{
		$hd .=	"Accept-Charset: ".$_SERVER["HTTP_ACCEPT_CHARSET"]."\r\n";
	}
	if(!empty($_SERVER["HTTP_ACCEPT_ENCODING"]))
	{
		$hd .=	"Accept-Encoding: ".$_SERVER["HTTP_ACCEPT_ENCODING"]."\r\n";
	}
	if(!empty($_SERVER["HTTP_ACCEPT_LANGUAGE"]))
	{
		$hd .=	"Accept-Language: ".$_SERVER["HTTP_ACCEPT_LANGUAGE"]."\r\n";
	}
	if($useaut)
	{
		if(!empty($aut))
		{
			$hd .=	"Authorization: ".$aut."\r\n";
		}
		else if(!empty($_SERVER["HTTP_AUTHORIZATION"]))
		{
			$hd .=	"Authorization: ".$_SERVER["HTTP_AUTHORIZATION"]."\r\n";
		}
		else if(!empty($_SERVER["PHP_AUTH_USER"]))
		{
			$hd .=	"Authorization: Basic ".base64_encode($_SERVER["PHP_AUTH_USER"].":".(empty($_SERVER["PHP_AUTH_PW"])?"":$_SERVER["PHP_AUTH_PW"]))."\r\n";
		}
	}
	// Expect
	// From
	$hd .= "Host: ".$host."\r\n";
	// If-Match
	if(!empty($_SERVER["HTTP_IF_MODIFIED_SINCE"]))
	{
		$hd .=	"If-Modified-Since: ".$_SERVER["HTTP_IF_MODIFIED_SINCE"]."\r\n";
	}
	if(!empty($_SERVER["HTTP_IF_NONE_MATCH"]))
	{
		$hd .=	"If-None-Match: ".$_SERVER["HTTP_IF_NONE_MATCH"]."\r\n";
	}
	if(!empty($_SERVER["HTTP_IF_RANGE"]))
	{
		$hd .=	"If-Range: ".$_SERVER["HTTP_IF_RANGE"]."\r\n";
	}
	if(!empty($_SERVER["HTTP_IF_UNMODIFIED_SINCE"]))
	{
		$hd .=	"If-Unmodified-Since: ".$_SERVER["HTTP_IF_UNMODIFIED_SINCE"]."\r\n";
	}
	// Max-Forwards
	// Proxy-Authorization
	if(!empty($_SERVER["HTTP_RANGE"]))
	{
		$hd .=	"Range: ".$_SERVER["HTTP_RANGE"]."\r\n";
	}
	if(!empty($_SERVER["HTTP_REFERER"]))
	{
		$hd .=	"Referer: ".$_SERVER["HTTP_REFERER"]."\r\n";
	}
	// TE
	if(!empty($_SERVER["HTTP_USER_AGENT"]))
	{
		$hd .=	"User-Agent: ".$_SERVER["HTTP_USER_AGENT"]."\r\n";
	}

	// entity header
	// Content-Encoding
	// Content-Language
	// Content-Length
	// Content-Location
	// Content-MD5
	// Content-Range
	// Content-Type
	// Expires
	// Last-Modified

	$post = "";
	if(!empty($_POST))
	{
		$fp = @fopen("php://input", "r");
		if($fp)
		{
			while(!feof($fp))
			{
				$post .= fread($fp, 2048);
			}
			fclose($fp);
		}
		else if(isset($HTTP_RAW_POST_DATA))
		{
			$post = $HTTP_RAW_POST_DATA;
		}
		else
		{
			$size = count($_POST);
			if($size)
			{
				$i = 0;
				foreach($_POST as $var => $value)
				{
					$post .= $var."=".urlencode($value);
					if($i++ < ($size-1))
					{
						$post .= "&";
					}
				}
			}
		}

			$hd .=	"Content-Type: application/x-www-form-urlencoded\r\n";
			$hd .=	"Content-Length: ".strlen($post)."\r\n";
			/*
			if(!empty($_SERVER["HTTP_CONTENT_TYPE"]))
			{
				$hd .=	"Content-Type: ".$_SERVER["HTTP_CONTENT_TYPE"]."\r\n";
			}
			if(!empty($_SERVER["HTTP_CONTENT_LENGTH"]))
			{
				$hd .=	"Content-Length: ".$_SERVER["HTTP_CONTENT_LENGTH"]."\r\n";
			}
			// else
			if(empty($_SERVER["HTTP_CONTENT_TYPE"]))
			{
				$hd .=	"Content-Type: application/x-www-form-urlencoded\r\n";
			}
			if(empty($_SERVER["HTTP_CONTENT_LENGTH"]))
			{
				$hd .=	"Content-Length: ".strlen($post)."\r\n";
			}
			//*/
	}

	// extension header
	if(!empty($_SERVER["HTTP_X_FORWARDED_FOR"]))
	{
		$hd .=	"X-Forwarded-For: ".$_SERVER["HTTP_X_FORWARDED_FOR"]."\r\n";
	}
	if(!empty($_SERVER["HTTP_COOKIE"]))
	{
		$hd .=	"Cookie: ".$_SERVER["HTTP_COOKIE"]."\r\n";
	}
	//if(!empty($_SERVER["HTTP_PROXY_CONNECTION"]))
	//{
	//	$hd .=	"Proxy-Connection: close\r\n";
	//}

	$hd	.=	"\r\n".$post;

	return $hd;
}
    /*
	if(isset($en)) unset($en);
	if(isset($px)) unset($px);
	if(isset($id)) unset($id);
	if(isset($ua)) unset($ua);
	if(isset($au)) unset($au);
	if(isset($ps)) unset($ps);
	if(isset($st)) unset($st);
	if(isset($md)) unset($md);
	if(isset($pl)) unset($pl);
	if(isset($debug)) unset($debug);
	if(isset($log)) unset($log);
	//*/

	$en = "";
	$px = "";
	$ua = "";
	$au = "";
	$md = "";
	$pl = "";
	$st = "";
	$debug = "";
	$log = "";
	$filelog = "";
	$co = "";

	if(!empty($_SERVER["QUERY_STRING"]))
	{
		parse_str($_SERVER["QUERY_STRING"]);
	}

	if($en !== "off")
	{
		$en = 1;
	}
	else
	{
		$en = 0;
	}

	if($st === "on")
	{
		$st = 1;
	}
	else
	{
		$st = 0;
	}

	if($ua === "on")
	{
		$ua = 1;
	}
	else
	{
		$ua = 0;
	}

	if($debug === "on")
	{
		$debug = 1;
	}
	else
	{
		$debug = 0;
	}

	if($log === "off")
	{
		$log = 0;
	}
	else
	{
		$log = 1;
	}

	if($filelog === "on")
	{
		$filelog = 1;
	}
	else
	{
		$filelog = 0;
	}

	if($co === "on")
	{
		$co = 1;
	}
	else
	{
		$co = 0;
	}

	if(empty($id))
	{
		print_error("HTTP/1.1 503 Service Temporarily Unavailable", "Error: HTTP/1.1 404 Not Found", "File Not Found", "no id defined", $st);
		exit;
	}

	if($en)
	{
		$id = base64_decode($id);
		if(!empty($px))
		{
			$px = base64_decode($px);
		}
		if(!empty($au))
		{
			$au = base64_decode($au);
		}
		if(!empty($pl))
		{
			$pl = base64_decode($pl);
		}
		if(!empty($md))
		{
			$md = base64_decode($md);
		}

		if($id === FALSE)
		{
			print_error("HTTP/1.1 503 Service Temporarily Unavailable", "Error: HTTP/1.1 404 Not Found", "File Not Found", "incorrect id defined: ".$id." [".$_SERVER["QUERY_STRING"]."]", $st);
			exit;
		}
	}

	$aid = parse_url($id);
	if(empty($aid["host"]))
	{
		print_error("HTTP/1.1 503 Service Temporarily Unavailable", "Error: HTTP/1.1 404 Not Found", "File Not Found", "url parse error, no host defined: ".$id." [".$_SERVER["QUERY_STRING"]."]", $st);
		exit;
	}

	$hash =	'erfVBWRE#@214354SDFGJOuyklghjmDxVDSGFVAdFVfdahbgfswRT4wTRhbGFhjnDfvbdefW@'.
			'Rey6i987098OLYIKj<GFdsFa!@#$fdsfa*#@44325ergtffhU&^9o7pOuyjk<MfJdsAdVxzad'.
			'fgqetrwq!$%hhsR!#tCVXBeryt2wt14^%@43^%15!43654876ytIJI%ueGThnszSghAyhtyWT'.
			'RY@$%uytrHJDEtjU&TEiTykuiol67987oklui';

	if(!empty($px))
	{
		//$apx = parse_url($px);
		@list($px_host, $px_port) = explode(":", $px, 2);
		if(empty($px_host))
		{
			print_error("HTTP/1.1 503 Service Temporarily Unavailable", "Error: HTTP/1.1 404 Not Found", "File Not Found", "no px_host defined: ".$px, $st);
			exit;
		}

		if(empty($px_port))
		{
			$service_port = 8080;
		}
		else
		{
			$service_port = $px_port;
		}

		$address = gethostbyname($px_host);
	}
	else
	{
		if(!empty($aid["port"]))
		{
			$service_port = $aid["port"];
		}
		else if(!empty($aid["scheme"]) && ($aid["scheme"] === "ftp"))
		{
			$service_port = 21;
		}
		else
		{
			$service_port = 80;
		}

		$address = gethostbyname ($aid["host"]);
	}
	

	if(!empty($px) || (!empty($aid["scheme"]) && ($aid["scheme"] === "http")))
	{
		$socket = @fsockopen($address, $service_port, $errno, $errstr, 120);
		if (!$socket)
		{
			print_error("HTTP/1.1 503 Service Temporarily Unavailable", "Error: HTTP/1.1 404 Not Found", "File Not Found", "can't connect with fsockopen(): ".$errstr." (".$errno.")", $st);
			exit;
		}

		$in	 = create_header($md, $aid["host"], $id, $pl, $au, $ua);
		$out = "";
		
		// debug log here

		if($log)
		{
			$link = @mysql_connect("db.pfzim.mass.hc.ru", "pfzim", base64_decode("aGF1NFZveWk="));
			if(!$link)
			{
				$error_msg = @mysql_error();
			}
			else if(!@mysql_select_db("wwwpfzimru", $link))
			{
				$error_msg = @mysql_error();
				@mysql_close($link);
				$link = NULL;
			}
			else
			{
				if(!@mysql_query("INSERT INTO pfz_proxy_rd (date, ip, header) VALUES (NOW(), '".addslashes($_SERVER["REMOTE_ADDR"])."', '".base64_encode($in)."')", $link))
				{
					$error_msg = @mysql_error();
				}
				@mysql_close($link);
			}
		}
	
		//packet size
		if(empty($ps) || ($ps < 1) || ($ps > 65535))
		{
			$ps = 2048;
		}

		if(!fwrite($socket, $in, strlen($in)))
		{
			print_error("HTTP/1.1 503 Service Temporarily Unavailable", "Error: HTTP/1.1 404 Not Found", "File Not Found", "can't write with fwrite()", $st);
			fclose($socket);
			exit;
		}

		//translate header
		if($st) //small trafic
		{
			while(!feof($socket))
			{
				$out = fgets($socket, 4096);
				//$out = trim($out);
				//is end of header?
				if($out == "\r\n")
				{
					break;
				}
				if(!headers_sent())
				{
					header($out);
				}
			}
		}
		else
		{
			if(!headers_sent())
			{
				header("Content-Type: application/octet-stream");
			}
		}

		$encode_pos = 0;

		//translate data
		while(!feof($socket))
		{
	    	if($co)
			{
				$out = fgetc($socket);
				echo chr(ord($out[0]) ^ ord($hash[$encode_pos++]));
				if($encode_pos == 256)
				{
					$encode_pos = 0;
				}
			}
			else
			{
				$out = fread($socket, $ps);
			    echo $out;
			}
		}

		fclose($socket);

		if($debug) echo $in;  //debug
		if($filelog)
		{
			$fp = @fopen("redir_nx.log", "a");
			if($fp)
			{
				fwrite($fp, $in, strlen($in));
				fclose($fp);
			}
		}
	}

	exit;
	
   /*
	* 	hey, what are you looking here? better enable sockets functions ie. socket_select()!
	*/
?>
