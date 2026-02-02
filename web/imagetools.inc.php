<?php

 // creates a dummy image 312x200
 function createDummyImage($xsize,$ysize,$text) {
  header("Content-type: image/png");
  $img = @ImageCreate($xsize,$ysize) or die("Kann keinen neuen GD-Bild-Stream erzeugen");
  $background_color = ImageColorAllocate($img,102,102,102);
  $text_color = ImageColorAllocate($img,233,14,91);
  ImageString($img,3,5,5,$text,$text_color);
  ImagePNG($img);
 }
 function createEmptyImage($xsize,$ysize) {
  header("Content-type: image/png");
  $img = imagecreate($xsize,$ysize);
  ImageColorAllocate($img,255,255,255);
  imagepng($img);
 }

 // watermarking input image
 //  * random positioning of three small watermarks
 function createWatermarkedImage($imagefile,$iTransparency=95) {
  $watermarkfile = $_SERVER[DOCUMENT_ROOT].'/data/pics/jubrain_watermark2b.png';
  $watermark = imagecreatefrompng($watermarkfile);
  list($iWatermarkWidth,$iWatermarkHeight) = getimagesize($watermarkfile);
  $tileimage = imagecreatefrompng($imagefile);
  list($iTileWidth,$iTileHeight) = getimagesize($imagefile);
  $iPositionX = ceil(($iTileWidth/2));
  $iPositionX -= ceil(($iWatermarkWidth/2));
  $iPositionY = ceil(($iTileHeight/2));
  $iPositionY -= ceil(($iWatermarkHeight/2));
  $iTransparency = 100-$iTransparency;
  imagecopymerge($tileimage,$watermark,$iPositionX,$iPositionY,0,0,$iWatermarkWidth,$iWatermarkHeight,$iTransparency);
  imagepng($tileimage);
  imagedestroy($watermark);
  imagedestroy($tileimage);
 }

?>
