<?php

 // >>>
 function getLoginData() {
  $mysqluser = "WWWWWWWW";
  $mysqlpasswd = "YYYYYYYY";
  return array($mysqluser,$mysqlpasswd);
 }

 // >>>
 function iconnect_to_mydatabase($database='atlas') {
  $logindata = getLoginData();
  $mysqluser = $logindata[0];
  $mysqlpasswd = $logindata[1];
  $mysqlhost = "localhost";
  $link = mysqli_connect($mysqlhost,$mysqluser,$mysqlpasswd,$database);
  if ( !$link ) {
   echo "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\">M
               <html><head><title>Sorry, no connection to host '".PHP_OS."'</title>
                   <body><p>Sorry, no connection to database</body>
                   </html>\n";
   return 0;
  }
  // mysqli_query($link,"SET NAMES 'utf8'");
  return $link;
 }
 function iclose_mydatabase($link) {
  mysqli_close($link);
 }

?>
