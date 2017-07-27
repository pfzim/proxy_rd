<?php
	header("Content-Type: text/plain; charset=windows-1251");

	echo "Creating...\r\n";
	flush();
	$fp = socket_create(AF_UNIX, SOCK_STREAM, 0);
	if($fp)
	{
		if($m === "r")
		{
			socket_bind($fp, "my.sock");
			socket_listen($fp);
			$sk = socket_accept($fp);
			echo "socket_read: ".socket_read($sk, 1024, PHP_BINARY_READ);
			socket_close($sk);
		}
		else if($m === "w")
		{
			socket_connect($fp, "my.sock");
			echo "socket_write: ".socket_write($fp, "1024, PHP_BINARY_READ");
		}
		socket_close($fp);
		if($m === "r")
		{
			unlink("my.sock");
		}
	}
	else
	{
		echo "error";
	}
?>
