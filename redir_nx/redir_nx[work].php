<?php
   /*
    *	free.net url redirecter v0.001b (c)oded by dEmon in 2002 [unix]
	*/
	
	// sample script for download files with resume!

	error_reporting (E_ALL);

function print_error($hdr, $title, $text, $info, $st)
{
	$string = "<!-- ".$info." --><html><head><title>".$title."</title></head><body><p><strong>".$text."</strong></p></body></html>";
	if($st == "on")
	{
		@header ($hdr);
		@header ("Content-Type: text/html");
		@header ("Content-Length: ".strlen($string));
	}
	else
	{
		echo $hdr;
		echo "Content-Type: text/html";
		echo "Content-Length: ".strlen($string);
	}
	
	echo $string;
}

function create_header($method, $host, $uri, $aut, $useaut)
{
	if(@$method)
	{
		$hd = $method;
	}
	else if(@$_SERVER["REQUEST_METHOD"])
	{
    	$hd = 	$_SERVER["REQUEST_METHOD"];
	}
	else
	{
		$hd = 	"GET";
	}
	// old
	//$hd .= 	" ".$uri." ";
	
	// new
	$uri_arr = parse_url($uri);
	$hd .= 	" ".$uri_arr["path"].(isset($uri_arr["query"])?("?".$uri_arr["query"]):"")." ";
	
	if(@$_SERVER["SERVER_PROTOCOL"])
	{
    	//$hd .= 	$_SERVER["SERVER_PROTOCOL"];
		$hd .= 	"HTTP/1.1";
	}
	else
	{
		$hd .= 	"HTTP/1.0";
	}
	$hd .= 	"\r\nHost: ".$host."\r\n";
	if(@$_SERVER["HTTP_RANGE"])
	{
		$hd .=	"Range: ".$_SERVER["HTTP_RANGE"]."\r\n";
	}
	if(@$_SERVER["HTTP_USER_AGENT"])
	{
		$hd .=	"User-Agent: ".$_SERVER["HTTP_USER_AGENT"]."\r\n";
	}
	if(@$_SERVER["HTTP_ACCEPT"])
	{
		$hd .=	"Accept: ".$_SERVER["HTTP_ACCEPT"]."\r\n";
	}
	if(@$_SERVER["HTTP_ACCEPT_CHARSET"])
	{
		$hd .=	"Accept-Charset: ".$_SERVER["HTTP_ACCEPT_CHARSET"]."\r\n";
	}
	if(@$_SERVER["HTTP_ACCEPT_ENCODING"])
	{
		//$hd .=	"Accept-Encoding: ".$_SERVER["HTTP_ACCEPT_ENCODING"]."\r\n";
	}
	if(@$_SERVER["HTTP_ACCEPT_LANGUAGE"])
	{
		$hd .=	"Accept-Language: ".$_SERVER["HTTP_ACCEPT_LANGUAGE"]."\r\n";
	}
	if(@$_SERVER["HTTP_REFERER"])
	{
		$hd .=	"Referer: ".$_SERVER["HTTP_REFERER"]."\r\n";
	}
	if(@$_SERVER["HTTP_VIA"])
	{
		//$hd .=	"Via: ".$_SERVER["HTTP_VIA"]."\r\n";
	}
	if(@$_SERVER["HTTP_X_FORWARDED_FOR"])
	{
		//$hd .=	"X-Forwarded-For: ".$_SERVER["HTTP_X_FORWARDED_FOR"]."\r\n";
	}
	if(@$_SERVER["HTTP_IF_MODIFIED_SINCE"])
	{
		$hd .=	"If-Modified-Since: ".$_SERVER["HTTP_IF_MODIFIED_SINCE"]."\r\n";
	}
	if(@$_SERVER["HTTP_IF_NONE_MATCH"])
	{
		$hd .=	"If-None-Match: ".$_SERVER["HTTP_IF_NONE_MATCH"]."\r\n";
	}
	if(@$_SERVER["HTTP_IF_RANGE"])
	{
		$hd .=	"If-Range: ".$_SERVER["HTTP_IF_RANGE"]."\r\n";
	}
	if(@$_SERVER["HTTP_IF_UNMODIFIED_SINCE"])
	{
		$hd .=	"If-Unmodified-Since: ".$_SERVER["HTTP_IF_UNMODIFIED_SINCE"]."\r\n";
	}
	if(@$_SERVER["HTTP_CONTENT_TYPE"])
	{
		$hd .=	"Content-Type: ".$_SERVER["HTTP_CONTENT_TYPE"]."\r\n";
	}
	if(@$_SERVER["HTTP_CONTENT_LENGTH"])
	{
		$hd .=	"Content-Length: ".$_SERVER["HTTP_CONTENT_LENGTH"]."\r\n";
	}
	if(@$_SERVER["HTTP_CACHE_CONTROL"])
	{
		$hd .=	"Cache-Control: ".$_SERVER["HTTP_CACHE_CONTROL"]."\r\n";
	}
	if(@$_SERVER["HTTP_PRAGMA"])
	{
		$hd .=	"Pragma: ".$_SERVER["HTTP_PRAGMA"]."\r\n";
	}
	if(@$_SERVER["HTTP_COOKIE"])
	{
		$hd .=	"Cookie: ".$_SERVER["HTTP_COOKIE"]."\r\n";
	}
	if(@$aut)
	{
		$hd .=	"Authorization: ".$aut."\r\n";
	}
	else if(@$useaut)
	{
		if(@$_SERVER["HTTP_AUTHORIZATION"])
		{
			$hd .=	"Authorization: ".$_SERVER["HTTP_AUTHORIZATION"]."\r\n";
		}
		else if(@$_SERVER["PHP_AUTH_USER"])
		{
			$hd .=	"Authorization: Basic ".base64_encode($_SERVER["PHP_AUTH_USER"].":".@$_SERVER["PHP_AUTH_PW"])."\r\n";
		}
	}
	//if(@$_SERVER["HTTP_PROXY_CONNECTION"])
	//{
	//	$hd .=	"Proxy-Connection: ".$_SERVER["HTTP_PROXY_CONNECTION"]."\r\n";
	//}

	$post = "";
	if(@$_POST)
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

			if(@!$_SERVER["HTTP_CONTENT_TYPE"])
			{
				$hd .=	"Content-Type: application/x-www-form-urlencoded\r\n";
			}
			if(@!$_SERVER["HTTP_CONTENT_LENGTH"])
			{
				$hd .=	"Content-Length: ".strlen($post)."\r\n";
			}
		}
	}

	$hd	.=	"\r\n".$post;
	
	return $hd;
}

	if(isset($en)) unset($en);
	if(isset($px)) unset($px);
	if(isset($id)) unset($id);
	if(isset($ua)) unset($ua);
	if(isset($au)) unset($au);
	if(isset($ps)) unset($ps);
	if(isset($st)) unset($st);
	if(isset($md)) unset($md);
	if(isset($debug)) unset($debug);

	parse_str($_SERVER['QUERY_STRING']);

	if(@!$id)
	{
		print_error("HTTP/1.1 503 Service Temporarily Unavailable", "Error: HTTP/1.1 404 Not Found", "File Not Found", "no id defined", @$st);
		exit;
	}

	if(@$en != "no")
	{
		$id = base64_decode($id);
		if(@$px)
		{
			$px = base64_decode($px);
		}
		if(@$au)
		{
			$au = base64_decode($au);
		}
	}

	$aid = parse_url($id);
	if(@!$aid["host"])
	{
		print_error("HTTP/1.1 503 Service Temporarily Unavailable", "Error: HTTP/1.1 404 Not Found", "File Not Found", "url parse error, no host defined: ".$id." [".$_SERVER['QUERY_STRING']."]", @$st);
		exit;
	}

	if(@$px)
	{
		//$apx = parse_url($px);
		@list($px_host, $px_port) = explode(":", $px, 2);
		if(@!$px_host)
		{
			print_error("HTTP/1.1 503 Service Temporarily Unavailable", "Error: HTTP/1.1 404 Not Found", "File Not Found", "no px_host defined: ".$px, @$st);
			exit;
		}

		if(@$px_port)
		{
			$service_port = $px_port;
		}
		else
		{
			$service_port = 8080;
		}

		$address = gethostbyname($px_host);
	}
	else
	{
		if(@$aid["port"])
		{
			$service_port = $aid["port"];
		}
		else if(@$aid["scheme"] == "ftp")
		{
			$service_port = 21;
		}
		else
		{
			$service_port = 80;
		}

		$address = gethostbyname ($aid["host"]);
	}
	

	if((@$px) || (@$aid["scheme"] == "http"))
	{
		@$socket = fsockopen($address, $service_port, $errno, $errstr, 120);
		if (!$socket)
		{
			print_error("HTTP/1.1 503 Service Temporarily Unavailable", "Error: HTTP/1.1 404 Not Found", "File Not Found", "can't connect with fsockopen(): ".$errstr." (".$errno.")", @$st);
			exit;
		}

		$in	 = create_header(@$md, $aid["host"], $id, @$au, (@$ua == "on"));
		$out = "";
		
		//packet size
		if(@!$ps || (@$ps < 1) || (@$ps > 65535))
		{
			$ps = 2048;
		}

		if(!fwrite($socket, $in, strlen ($in)))
		{
			print_error("HTTP/1.1 503 Service Temporarily Unavailable", "Error: HTTP/1.1 404 Not Found", "File Not Found", "can't write with fwrite()", @$st);
			fclose($socket);
			exit;
		}

		//translate header
		if(@$st == "on") //small trafic
		{
			while (!feof($socket))
			{
				$out = fgets($socket, 4096);
				//$out = trim($out);
				//is end of header?
				if($out == "\r\n")
				{
					break;
				}
				@header($out);
			}
		}
		else
		{
			@header("Content-Type: application/octet-stream");
		}

		//translate data
		while (!feof($socket))
		{
	    	$out = fread($socket, $ps);
		    echo $out;
		}

		fclose($socket);

		if(@$debug == "on") echo $in;  //debug
	}

	exit;
	
   /*
	* 	hey, what are you looking here? better enable sockets functions ie. socket_select()!
	*/
?>
