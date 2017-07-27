<?php
   /*
    *	free.net url redirecter v0.001b (c)oded by dEmon in 2002 [unix]
	*/
	
	// sample script for download files with resume!

	error_reporting (E_ALL);

function print_error($hdr, $title, $text, $info, $st)
{
	$string = "<!-- ".htmlspecialchars($info)." --><html><head><title>".htmlspecialchars($title)."</title></head><body><p><strong>".htmlspecialchars($text)."</strong></p></body></html>";

	if($st)
	{
	    if(!headers_sent())
	    {
			header($hdr);
			header("Content-Type: text/html");
			header("Content-Length: ".strlen($string));
		}
	}
	else
	{
		echo $hdr;
		echo "Content-Type: text/html";
		echo "Content-Length: ".strlen($string);
	}
	
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
	if((!empty($_SERVER["CONTENT_TYPE"]) && (strncmp($_SERVER["CONTENT_TYPE"], "multipart/form-data", 19) == 0)) || !empty($_FILES))
	{
		if((strlen($_SERVER["CONTENT_TYPE"]) > 19) && (strncmp(substr($_SERVER["CONTENT_TYPE"], 19), "; boundary=", 11) == 0))
		{
			$bou = substr($_SERVER["CONTENT_TYPE"], 30);
			$hd .= "Content-Type: ".$_SERVER["CONTENT_TYPE"]."\r\n";
		}
		else
		{
			$bou = md5(uniqid(rand(), true));
			$hd .= "Content-Type: multipart/form-data; boundary=".$bou."\r\n";
		}
		
		$size = count($_POST);
		if($size)
		{
			foreach($_POST as $var => $value)
			{
				// here you can make support for array values
				if(is_array($value))
				{
					foreach($value as $val)
					{
						if(get_magic_quotes_gpc())
						{
							$val = stripslashes($val);
						}

						$post .= "--".$bou."\r\nContent-Disposition: form-data; name=\"".$var."[]\"\r\n\r\n".$val."\r\n";
					}
				}
				else
				{
					if(get_magic_quotes_gpc())
					{
						$value = stripslashes($value);
					}

					$post .= "--".$bou."\r\nContent-Disposition: form-data; name=\"".$var."\"\r\n\r\n".$value."\r\n";
				}
			}
		}
		
		$size = count($_FILES);
		if($size)
		{
			foreach($_FILES as $name => $var)
			{
				if(is_array($var['name']))
				{
					foreach($var['name'] as $key => $value)
					{
						// only one level array support
						if(!is_array($var['name'][$key]))
						{	
							$post .= "--".$bou."\r\nContent-Disposition: form-data; name=\"".$name."[]\"; filename=\"".$var['name'][$key]."\"\r\n";
							$post .= "Content-Type: ".(empty($var['type'][$key])?"application/octet-stream":$var['type'][$key])."\r\n\r\n";
							//$post .= "Error code: ".$var['error'][$key]."\r\n";
							if($var['error'][$key] == 0)
							{
								$fp = @fopen($var['tmp_name'][$key], "r");
								if($fp)
								{
									while(!feof($fp))
									{
										$post .= fread($fp, 2048);
									}
									fclose($fp);
									unlink($var['tmp_name'][$key]);
								}
							}
							$post .= "\r\n";
						}
					}
				}
				else
				{
					$post .= "--".$bou."\r\nContent-Disposition: form-data; name=\"".$name."\"; filename=\"".$var['name']."\"\r\n";
					$post .= "Content-Type: ".(empty($var['type'])?"application/octet-stream":$var['type'])."\r\n\r\n";
					//$post .= "Error code: ".$var['error']."\r\n";
					if($var['error'] == 0)
					{
						$fp = @fopen($var['tmp_name'], "r");
						if($fp)
						{
							while(!feof($fp))
							{
								$post .= fread($fp, 2048);
							}
							fclose($fp);
							unlink($var['tmp_name']);
						}
					}
					$post .= "\r\n";
				}
			}
		}
		$post .= "--".$bou."--\r\n";
		$hd .=	"Content-Length: ".strlen($post)."\r\n";
	}
	else if(!empty($_POST))
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
					// here you can make support for array values
					if(is_array($value))
					{
						$ssize = count($value);
						$j = 0;
						foreach($value as $val)
						{
							if(get_magic_quotes_gpc())
							{
								$val = stripslashes($val);
							}

							$post .= "--".$bou."\r\nContent-Disposition: form-data; name=\"".$var."[]\"\r\n\r\n".$val."\r\n";

							if($j++ < ($ssize-1))
							{
								$post .= "&";
							}
						}
					}
					else
					{
						if(get_magic_quotes_gpc())
						{
							$value = stripslashes($value);
						}
					}
					
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
	$ci = "";
	$co = "";
	$pm = "";
	$test = "";

	if(!empty($_SERVER["QUERY_STRING"]))
	{
		parse_str($_SERVER["QUERY_STRING"]);
	}

	if($test === "me")
	{
		header("Content-Type: text/html; charset=windows-1251");
		echo '<html><body>'."\n";

		echo '<form action="'.$_SERVER["PHP_SELF"].'?test=do&id=aHR0cDovL3d3dy5leGFtcGxlLmNvbS8%3D" method="post" enctype="multipart/form-data">'."\n";
		
		echo '<textarea name="demo.test.dot">Hello, Dolly!</textarea><br />'."\n";
		echo '<textarea name="demo-test">Hello, Dolly!</textarea><br />'."\n";
		echo '<textarea name="demo_test">Hello, Dolly!</textarea><br />'."\n";
		echo '<textarea name="arraytest[]">Hello, Dolly!</textarea><br />'."\n";
		echo '<textarea name="arraytest[]">Hello, Dolly!</textarea><br />'."\n";
		echo '<input type="file" name="fup1" value=""><br />'."\n";
		echo '<input type="file" name="fup2[]" value=""><br />'."\n";
		echo '<input type="file" name="fup2[]" value=""><br />'."\n";
		
		echo '<input type="submit" value="Test!">'."\n";
		echo '</form>'."\n";

		//echo '<form action="'.$_SERVER["PHP_SELF"].'?test=do&en=off&id=http://www.example.com/&px=ya.ru:80&st=on&pm=on&log=off" method="post" enctype="multipart/form-data">'."\n";
		echo '<form action="'.$_SERVER["PHP_SELF"].'?id=aHR0cDovL3d3dy5leGFtcGxlLmNvbS8%3D&px=eWEucnU6ODA%3D&st=on&pm=on&log=off" method="post" enctype="multipart/form-data">'."\n";
		
		echo "<textarea name=\"request\" cols=\"40\" rows=\"10\">R0VUIC9sb2dvLnBuZyBIVFRQLzEuMQ0KSG9zdDogeWEucnUNClVzZXItQWdlbnQ6IE1vemlsbGEvNS4wIChXaW5kb3dzOyBVOyBXaW5kb3dzIE5UIDUuMTsgZW4tVVM7IHJ2OjEuOC4xLjEzKSBHZWNrby8yMDA4MDMxMyBTZWFNb25rZXkvMS4xLjkNCkFjY2VwdDogdGV4dC94bWwsYXBwbGljYXRpb24veG1sLGFwcGxpY2F0aW9uL3hodG1sK3htbCx0ZXh0L2h0bWw7cT0wLjksdGV4dC9wbGFpbjtxPTAuOCxpbWFnZS9wbmcsKi8qO3E9MC41DQpBY2NlcHQtTGFuZ3VhZ2U6IHJ1LGVuLXVzO3E9MC43LGVuO3E9MC4zDQpBY2NlcHQtRW5jb2Rpbmc6IGd6aXAsZGVmbGF0ZQ0KQWNjZXB0LUNoYXJzZXQ6IHdpbmRvd3MtMTI1MSx1dGYtODtxPTAuNywqO3E9MC43DQpDb25uZWN0aW9uOiBjbG9zZQ0KDQo=</textarea><br />"."\n";
		
		echo '<input type="submit" value="Test!">'."\n";
		echo '</form>'."\n";

		echo '</body></html>'."\n";
		exit;
	}

	if($test === "info")
	{
		phpinfo();
		exit;
	}

	if($en === "off")
	{
		$en = 0;
	}
	else
	{
		$en = 1;
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

	if($ci === "on")
	{
		$ci = 1;
	}
	else
	{
		$ci = 0;
	}
	
	if($co === "on")
	{
		$co = 1;
	}
	else
	{
		$co = 0;
	}
	
	if($pm === "on")
	{
		$pm = 1;
	}
	else
	{
		$pm = 0;
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

		$address = gethostbyname($aid["host"]);
	}
	

	if(!empty($px) || (!empty($aid["scheme"]) && ($aid["scheme"] === "http")))
	{
		if($pm)
		{
			if(isset($_FILES['request']) && !empty($_FILES['request']['tmp_name']) && (!isset($_FILES['request']['error']) || ($_FILES['request']['error'] == 0)))
			{
				$fp = @fopen($_FILES['request']['tmp_name'], "r");
				if(!$fp)
				{
					print_error("HTTP/1.1 503 Service Temporarily Unavailable", "Error: HTTP/1.1 404 Not Found", "File Not Found", "can't open request file", $st);
					exit;
				}
				
				$in = "";
				$encode_pos = 0;
				while(!feof($fp))
				{
					if($ci)
					{
						$out = fgetc($fp);
						if($out === false)
						{
							break;
						}
						$in .= chr(ord($out[0]) ^ ord($hash[$encode_pos++]));
						if($encode_pos == 256)
						{
							$encode_pos = 0;
						}
					}
					else
					{
						$in .= fread($fp, 2048);
					}
				}
				fclose($fp);
			}
			else if(!empty($_POST['request']))
			{
				$in = get_magic_quotes_gpc()?stripslashes($_POST['request']):$_POST['request'];
				if($en)
				{
					$in = base64_decode($in);
				}
				//else
				//{
				//	$in = str_replace("\n", "\r\n", $in);
				//}
				if($ci)
				{
					$k = strlen($in);
					for($i = 0; $i < $k; $i++)
					{
						$in[$i] = chr(ord($in[$i]) ^ ord($hash[$encode_pos++]));
						if($encode_pos == 256)
						{
							$encode_pos = 0;
						}
					}
				}
			}
			else
			{
				print_error("HTTP/1.1 503 Service Temporarily Unavailable", "Error: HTTP/1.1 404 Not Found", "File Not Found", "no request defined", $st);
				exit;
			}
		}
		else
		{
			$in	 = create_header($md, $aid["host"], $id, $pl, $au, $ua);
		}

		$out = "";
		
		if($test === "do")
		{
			header("Content-Type: text/plain; charset=windows-1251");
			echo $_SERVER["CONTENT_TYPE"]."\r\n";
			echo $_SERVER["CONTENT_LENGTH"]."\r\n\r\n";

			echo "POST: ";
			print_r($_POST);
			echo "FILES: ";
			print_r($_FILES);
			
			//echo base64_encode($in);
			echo $in;
			exit;
		}

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

		$socket = @fsockopen($address, $service_port, $errno, $errstr, 120);
		if(!$socket)
		{
			print_error("HTTP/1.1 503 Service Temporarily Unavailable", "Error: HTTP/1.1 404 Not Found", "File Not Found", "can't connect with fsockopen(): ".$errstr." (".$errno.")", $st);
			exit;
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
				if($out === false)
				{
					break;
				}
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
