<?php

 // Hints:
 //  Sample calls:
 //   User list: 'https://jubrain.fz-juelich.de/apps/bigbrainviewer/scripts/getImageForRepair.php'
 //   User data: 'https://jubrain.fz-juelich.de/apps/bigbrainviewer/scripts/getImageForRepair.php?user=SW'
 //  Dynamic checkerboard tile layout:
 //   See 'https://www.geeksforgeeks.org/create-a-tiles-layout-template-using-html-and-css/'
 //  HTML colors:
 //   See 'https://www.w3schools.com/tags/ref_colornames.asp'
 //  Set download destination path: 
 //   Chrome: By default downloads are placed into the Downloads folder. Go to Chrome.Settings.Downloads to change it.

 // >>>
 require_once('./localsetup.inc.php');
 require_once('./mydatabaseconnect.inc.php');
 require_once('./database-core.inc.php');

 // defaults 
 $DefaultHTMLColors = array("red","green","blue","magenta","cyan","teal","gold","purple",
                   "sienna","salmon","tan","thistle","plum","peru","orchid","navy","orange",
                   "yellow","khaki","peru","coral","violet","magenta","lime","olive","teal");
 $nDefaultHTMLColors = count($DefaultHTMLColors);

 $ImageViewer = "./getImageData.php";
 $BaseToolLink = "./getImageForRepair.php";

 // >>>
 function getFileName($datapath,$section) {
  $filename = $datapath."/clean_pm".$section."o.png";
  if ( !is_file($filename) ) {
   $filename = $datapath."/clean_pm".$section."r.png";
   if ( !is_file($filename) ) {
    $filename = $datapath."/clean_pm".$section."x.png";
    if ( !is_file($filename) ) {
     return "";
    }
   }
  }
  return $filename;
 }
 function getimagecounts($connId,$brain,$section,$modality,$orientation,$datapath) {
  $sql = "SELECT id,counts,reg_date FROM bigbrainviewer.downloads WHERE brain='$brain' AND section='$section' AND modality='$modality' AND orientation='$orientation'";
  // echo $sql."<br>";
  $result = ifetchObject($connId,$sql);
  if ( $result->id && $result->counts ) {
   $filename = getFileName($datapath,$section);
   if ( strlen($filename)>0 ) {
    // echo $filename." - ".strtotime($result->reg_date)." - ".filemtime($filename)."<br>";
    return filemtime($filename)>strtotime($result->reg_date)?0:$result->counts;
   }
   return $result->counts;
  }
  return 0;
 }
 /**
  * Functions to handle comment
  */
 function readExcelFile($filename) {
  $data = [];
  if ( ($handle = fopen($filename, "r")) !== FALSE ) {
   while (($row = fgetcsv($handle, 1000, ",")) !== FALSE) {
    $data[] = $row;
   }
   fclose($handle);
  } else {
   echo "WARNING: Cannot open status sheet file $filename<br>";
  }
  return $data;
 }
 function getThirdEntryByNumber($data,$searchNumber) {
  foreach ( $data as $row ) {
   if ( isset($row[0]) && $row[0] == $searchNumber ) {
    return isset($row[2]) ? $row[2] : null;
   }
  }
  return null;
 }
 function getimagecomment($statusSheetData,$connId,$brain,$section,$modality,$orientation) {
  if ( $connId ) {
   $sql = "SELECT id,comment,userId FROM bigbrainviewer.comments WHERE brain='$brain' AND section='$section' AND orientation='$orientation' AND modality='$modality'";
   $result = iquerydatabase($connId,$sql);
   while ( $row=mysqli_fetch_row($result) ) {
    $data = json_decode($row[1]);
    return isset($data->{'info'})?$data->{'info'}:"NaN"; 
   }
  }
  $result = getThirdEntryByNumber($statusSheetData,$section);
  if ( $result ) return $result;
  return $section; 
 }
 /**
  * get image creator name from database
  */
 function getimagecreator($connId,$brain,$section,$modality,$orientation) {
  $sql = "SELECT id,creator FROM bigbrainviewer.creators WHERE brain='$brain' AND section='$section' AND orientation='$orientation' AND modality='$modality'";
  $result = ifetchObject($connId,$sql);
  if ( $result->id && $result->creator ) {
    return $result->creator;
  } 
  return "HM";
 }
 function getimagestatus($connId,$brain,$section,$modality,$orientation,$name) {
  $sql = "SELECT id FROM bigbrainviewer.uploads WHERE brain='$brain' AND section='$section' AND orientation='$orientation' AND modality='$modality' AND shortname='$name'";
  $result = ifetchObject($connId,$sql);
  if ( $result->id ) {
   return "Uploaded";
  }
  return "New";
 }
 function getRealUsername($connId,$name) {
  $sql = "SELECT id,name,vorname FROM bigbrainviewer.user where shortname='$name'";
  $result = ifetchObject($connId,$sql);
  if ( $result->id ) {
   return $result->vorname." ".$result->name;
  } 
  return "unknown"; 
 } 
 // building methods
 function buildHeader($title) {
  echo "<html>";
  echo "<head>";
  echo '<link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/font-awesome/4.7.0/css/font-awesome.min.css">';
  echo "<style>";
  echo " .tiles-container { display: flex; flex-wrap: wrap; gap: 20px;}";
  echo " .usertile { display: flex; width: 196px; height: 196px; font-size: 62px; align-items: center; justify-content: center; color: black; background-color: Linen;}";
  echo " .userTileFooter { display: flex; height: 28px; align-items: center; justify-content: center; background-color: lightgray; }";
  echo " .lbl { display: inline-flex; height: 14px; border: none; background-color: DodgerBlue; color: white; padding: 6px 6px; cursor: pointer; }";
  echo " .lbl:hover { background-color: RoyalBlue; }";
  echo " .btn { border: none; background-color: DodgerBlue; color: white; padding: 6px 6px; margin-right: 2px; cursor: pointer; }";
  echo " .btn:disabled { background-color: #e0e0e0 !important; cursor: not-allowed; filter: grayscale(100%); opacity: 0.6; }";
  echo " .btn:hover { background-color: RoyalBlue; }";
  echo " .ctext { padding-left: 5px; text-align: left; }";
  echo " .headertable { border: 0px solid black; border-collapse: collapse; background-color: #5D7691; color: white; }";
  echo " p { width: 60px; height: 16px; padding: 1px; padding-left: 5px; margin: 1px; border-style: solid; border-width: 3px; border-color: transparent; }";
  echo " table { font-familiy: arial, sans-serif; border-collapse: collapse; width: 100%; border: 1px solid black; }";
  echo " td, th { border: 1px solid black; text-align: center; padding: 2px; }";
  echo "</style>";
  echo ' <script type="text/javascript" src="include/comment.inc.js"></script>';
  echo ' <script type="text/javascript" src="include/download.inc.js"></script>';
  echo ' <script type="text/javascript" src="include/upload.inc.js"></script>';
  echo "</head>";
  echo '<body onload="init()">';
  echo '<hr><div id="topline" style="width: 100%; height: 50px;">';
  echo '<table class="headertable" style="width: 100%; height: 90%;"><tr>';
  echo '<td style="text-align:left; font-size: 20px; padding-left: 10px;">'.$title.'</td>';
  echo '</tr></table></div><hr>';
 }
 function buildFooter() {
  echo "</body></html>";
 }
function buildUserTile($shortname,$fullname) {
 echo '<div class="userTileMain">';
 echo ' <div class="userTile">';
 echo '  <a href="'.$BaseToolLink.'?user='.$shortname.'"><span>'.$shortname.'</span></a>';
 echo ' </div>';
 echo ' <div class="userTileFooter">';
 echo '  <span>'.$fullname.'</span>';
 echo ' </div>';
 echo '</div>';
}

// open database
$connId = iconnect_to_mydatabase();

// >>>
if ( !isset($_GET['user']) ) {
 buildHeader("JulichBrain BigBrainViewer Admin Tool: Users");

 echo '<section class="tiles-container">';
 $sql = "SELECT id,name,vorname,shortname FROM bigbrainviewer.user WHERE status='2'";
 $result = iquerydatabase($connId,$sql);
 while ( $row=mysqli_fetch_row($result) ) {
  $fullname = $row[2]." ".$row[1];
  buildUserTile($row[3],$fullname);
 }
 echo '</section>';

 buildFooter();
 return;
}

 // processing
 $brainName = isset($_GET['brain'])?$_GET['brain']:"pm49692";
 $orientation = isset($_GET['orientation'])?$_GET['orientation']:"coronal";
 $section = isset($_GET['section'])?$_GET['section']:0;
 $modality = isset($_GET['modality'])?$_GET['modality']:"preimproved";
 $latest = isset($_GET['latest'])?$_GET['latest']:-1;
 $user = isset($_GET['user'])?$_GET['user']:"HM";
 $suffix = isset($_GET['suffix'])?$_GET['suffix']:"png";

 // >>>
 if ( 1 == 1 ) {
  $datapath = $basepath.'/'.$user.'/original';
  if ( !is_dir($datapath) ) {
   echo $datapath;
  } else {
   if ( 1 == 1 ) {
    buildHeader("JulichBrain BigBrainViewer Admin Tools: Images to repair for user ".getRealUserName($connId,$user)."...");
    if ( 1 == 1  ) {
     // read status sheet file
     $statusSheetFilename = "../data/bigbrain2_status_01022026.csv";
     $statusSheetData = readExcelFile($statusSheetFilename);
     // read results
     $result = [];
     $result_map = [];
     foreach (glob($datapath . '/*.{xcf,png}', GLOB_BRACE) as $file) {
      $path_info = pathinfo($file);
      $filename = $path_info['filename'];
      $extension = strtolower($path_info['extension']);
      $mtime = filemtime($file);
      if (!isset($result_map[$filename]) || $extension === 'xcf') {
        $result_map[$filename] = [$mtime, $file];
      }
     }
     $result = array_values($result_map);
     // finalize
     sort($result);
     $rresult = array_reverse($result);
     $bordercolor = $DefaultHTMLColors[0];
     $oldfiletime = "";
     $n = 0;
     $ncolor = 0;
     $tablestring = "";
     $tablestring .= "<table>";
     $tablestring .= '<tr><th style="width: 180px;">Filename</th><th style="width: 240px;">Latest modification</th><th style="width: 70px;">Creator</th><th style="width: 100px;">Status</th><th style="width: 120px">Actions</th><th>Comment</th></tr>';
     foreach ( $rresult as $filetime => $filename ) {
      $filetime = date("F d Y",filemtime($filename[1]));
      if ( $n == 0 ) {
       $oldfiletime = $filetime;
      }
      if ( strcmp($filetime,$oldfiletime) != 0 ) {
       $ncolor += 1;
       // if ( $latest < 0 && $ncolor >= abs($latest) ) { break; }
       $bordercolor = $DefaultHTMLColors[$ncolor%$nDefaultHTMLColors];
       $oldfiletime = $filetime;
      }
      $basefilename = basename($filename[1]);
      $pngfilename = str_replace("xcf","png",basename($filename[1]));
      $section = preg_replace('/[^0-9]/','',$basefilename);
      $imagecomment = getimagecomment($statusSheetData,$connId,$brainName,$section,$modality,$orientation);
      $creator = $user; // $connId?getimagecreator($connId,$brainName,$section,$modality,$orientation):"Unknown"; 
      $status = getimagestatus($connId,$brainName,$section,$modality,$orientation,$creator);
      $name = "file-".$creator."-".$basefilename."-".$n;
      $tablestring .= '<tr><td><p style="border-left-color: '.$bordercolor.';">'.$basefilename.'</p></td><td>'.date("F d Y H:i:s",filemtime($filename[1])).'</td>';
      $tablestring .= '<td id="creator-'.$n.'">'.$creator.'</td>';
      $tablestring .= '<td id="status-'.$n.'">'.$status.'</td>';
      $tablestring .= '<td><a href="'.$ImageViewer.'?brain='.$brainName.'&modality=preimproved&user='.$user.'&section='.$section.'&nocount=1" target="_blank"><button class="btn" title="Show section '.$section.' in a new tab"><i class="fa fa-eye"></i></button></a>';
      $xcfDisabledAttr = ( pathinfo($basefilename,PATHINFO_EXTENSION) === 'xcf' ) ? '' : 'disabled';
      $pngDisabledAttr = ( pathinfo($basefilename,PATHINFO_EXTENSION) === 'png' ) ? '' : 'disabled';
      $tablestring .= '<button class="btn" '.$xcfDisabledAttr.' id="'.$user.'_'.$brainName.'_'.$section.'_preimproved" title="Download xcf section '.$section.'" onclick="downloadImage(this.id,\''.$basefilename.'\')"><i class="fa fa-download"></i></button>';
      $tablestring .= '<button class="btn" '.$pngDisabledAttr.' id="'.$user.'_'.$brainName.'_'.$section.'_preimproved" title="Download png section '.$section.'" onclick="downloadImage(this.id,\''.$pngfilename.'\')"><i class="fa fa-download"></i></button>';
      $tablestring .= '<label class="lbl" id="'.$user.'_'.$brainName.'_'.$section.'_preimproved" title="Upload edited section"><i class="fa fa-upload"></i><input class="addfile-input" type="file" accept=".png,.xcf" name="'.$name.'" style="display: none;"></label></td>';
      $tablestring .= '<td class="ctext" id="ctl_'.$brainName.'_'.$section.'_edited">'.$imagecomment.'</td></tr>';
      // echo '<p style="border-left-color: '.$bordercolor.';">'.$basefilename." - ".date("F d Y H:i:s",filemtime($filename[1]))." - ".$ndownloads."</p>";
      $n += 1;
     }
     $tablestring .= "</table>";
     // create output
     echo "Listing ".$n." sections in '".$datapath."'...<br><hr>";
     echo $tablestring;
     echo "</body></html>";
    } else {
     $cdir = scandir($datapath);
     foreach ( $cdir as $key => $value ) {
      if ( !in_array($value,array(".","..")) ) {
       $filename = $datapath."/".$value;
       $filetime = date("F d Y H:i:s.",filemtime($filename));
       echo $value." - ".$filetime."<br>";
      }
     }
    } 
   }
  }
 }

 // >>>
 if ( $connId ) {
  iclose_mydatabase($connId);
 }

?>
