<?php

 // --
 function iquerydatabase($link,$sql_command) {
  $sql_result = mysqli_query($link,$sql_command);
  if ( !$sql_result ) {
   echo "FATAL ERROR: Malfunction in 'mysqli_query($link,$sql_command)': ".mysqli_error($link);
   return 0;
  }
  return $sql_result;
 } 

 function ifetchObject($link,$sql_command) {
  // echo mysqli_get_host_info($link)."<br>";
  $sqlresult = iquerydatabase($link,$sql_command);
  if ( $sqlresult ) {
   return mysqli_fetch_object($sqlresult);
  }
  return 0;
 }

 function ifetchObjects($link,$sql_command) {
  $sqlresult = iquerydatabase($link,$sql_command);
  $results = array();
  if ( $sqlresult ) {
   while ( $row = mysqli_fetch_object($sqlresult) ) {
    $results[] = $row; 
   }
  }
  return $results;
 }
 
?>
