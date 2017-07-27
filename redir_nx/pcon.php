<?php
	header("Content-Type: text/plain; charset=windows-1251");

	echo "Creating...\r\n";
	flush();

	$fp = pfsockopen("ya.ru", 80, $errn, $err);
	if($fp)
	{
		stream_set_timeout($fp, 1, 0);
		echo $fp." - ".ftell($fp)." bytes\r\n";
		flush();
		if($m === "w")
		{
			echo "fwrite: ".fwrite($fp, "GET / HTTP/1.0\r\nHost: ya.ru\r\nConnection: close\r\n\r\n")."\r\n";
			flush();
		}
		else if($m === "r")
		{			
			do
			{
				echo "fread: ".fread($fp, 1024)."\r\n";
				flush();
			}
			while(1);
			//while(!feof($fp));
		}
		else if($m === "c")
		{
			echo "Close... ".fclose($fp)."\r\n";
			flush();
		}
	}
	else
	{
		echo "error: ".$err;
		flush();
	}
?>
