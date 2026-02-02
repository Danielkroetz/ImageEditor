<?php

 session_start();

 // --
 require_once('./localsetup.inc.php');
 require_once('./imagetools.inc.php');
 require_once('./mydatabaseconnect.inc.php');
 require_once('./database-core.inc.php');

 // --
 function endsWith($haystack,$needle) {
  $length = strlen($needle);
  if( !$length ) {
   return true;
  }
  return substr($haystack,-$length) === $needle;
 }

 // --
 function countupimage($brain,$modality,$orientation,$section) {
  // open database
  $connId = iconnect_to_mydatabase();
  if ( !$connId ) {
   return 0;
  } else {
   $sql = "SELECT id,counts FROM bigbrainviewer.downloads WHERE brain='$brain' AND section='$section' AND modality='$modality' AND orientation='$orientation'";
   $result = ifetchObject($connId,$sql);
   if ( $result && $result->id && $result->counts ) { // updating
    $ncounts = $result->counts;
    $ncounts += 1;
    $sql = "UPDATE bigbrainviewer.downloads SET counts='$ncounts' WHERE id='$result->id'";
   } else { // create new entry
    $sql = "INSERT INTO bigbrainviewer.downloads (brain,section,modality,orientation,counts)";
    $sql .= " VALUES ('$brain','$section','$modality','$orientation','1')";
   }
   iquerydatabase($connId,$sql);
   iclose_mydatabase($connId);
  }
  return 1;
 }
 
 // --
 function getCorrectFileName($filename) {
  $infilename = $filename."x.png";
  if ( !file_exists($infilename) ) {
   $infilename = $filename."r.png";
   if ( !file_exists($infilename) ) {
    $infilename = $filename."o.png";
   }
  }
  return $infilename;
 }

 // >>>
 $brainName = isset($_GET['brain'])?$_GET['brain']:"pm49692";
 $section = isset($_GET['section'])?$_GET['section']:"3736";
 $orientation = isset($_GET['orientation'])?$_GET['orientation']:"coronal";
 $path = isset($_GET['path'])?$_GET['path']:"tissuescope/resampled/100micron";
 $suffix = isset($_GET['suffix'])?$_GET['suffix']:"tissuescope";
 $modality = isset($_GET['modality'])?$_GET['modality']:"unknown";
 $countdownloads = isset($_GET['nocount'])?!$_GET['nocount']:1;
 $checksum = isset($_GET['checksum'])?$_GET['checksum']:0;
 $user = isset($_GET['user'])?$_GET['user']:"HM";
 $debug = isset($_GET['debug'])?$_GET['debug']:0;

 // >>>
 // JuBrain: '/localStore/JuBrain/data/brains/postmortem/pm49692/pics/tissuescope/resampled/100micron'
 // $image = $baseURL.'/brains/postmortem/'.$brainName.'/pics/tissuescope/resampled/100micron';
 $imagepath = $baseURL.'/brains/postmortem/'.$brainName.'/pics';
 $sectionname = sprintf("%04d",$section);
 $isEdited = -1;
 if ( isset($_GET['modality']) && $modality == 'edited' ) {
  $isEdited = ( endsWith($section,"1") || endsWith($section,"6") ) ? 1 : 0;
  $modpath = $isEdited == 1 ? "edited" : "improved";
  $prepath = $isEdited == 1 ? "clean_pm" : "pm";
  $image = $imagepath."/frame_7000clean/".$modpath."/".$prepath.$sectionname."o.png";
  if ( !file_exists($image) ) {
   $image = $imagepath."/frame_7000clean/".$modpath."/".$prepath.$sectionname."x.png";
   if ( !file_exists($image) ) {
    $image = $imagepath."/frame_7000clean/".$modpath."/".$prepath.$sectionname."r.png";
   }
  }
 } else if ( isset($_GET['modality']) && $modality == 'preimproved' ) {
  $image = getCorrectFilename($imagepath."/frame_7000clean/preimproved/".$user."/pm".$sectionname);
  if ( !file_exists($image) ) {
   $image = getCorrectFilename("/var/www/upload/bigbrainviewer/user/".$user."/original/pm".$sectionname);
  }
 } else {
  $image = $imagepath."/".$path."/".$brainName."histo_section".$sectionname."_".$suffix.".png";
 }
 if ( $debug == 1 ) {
  echo "Image: '".$image."', edited=".$isEdited.", modificationTime=".date("F d Y H:i:s.",filemtime($image))."<br>";
 } else if ( $checksum == 1 ) {
  echo is_file($image) ? md5_file($image) : 0; 
 } else {
  if ( !file_exists($image) ) {
   $image = "../pics/noimage.png";
  }
  if ( $debug == 0 ) {
   if ( $countdownloads == 1 ) {
    countupimage($brainName,$modality,$orientation,$section);
   }
   header('Content-type: image/png');
   $handle = fopen($image,"rb");
    fpassthru($handle);
   fclose($handle);
  } else {
   echo "Missing image '".$image."'<br";
  }
 }

?>
