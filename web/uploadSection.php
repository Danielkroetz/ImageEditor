<?php
session_start();

// Basis-Pfad auf dem Server
require_once('./localsetup.inc.php');

// 1. Alle User-Ordner einlesen
$userFolders = [];
if ( file_exists($basePath) ) {
    $userFolders = array_diff(scandir($basePath), array('.', '..'));
}

// 2. Gewählten User setzen (aus Formular oder Session)
if (isset($_POST['select_user'])) {
    $_SESSION['current_user'] = $_POST['selected_user'];
}

$currentUser = isset($_SESSION['current_user']) ? $_SESSION['current_user'] : null;
$targetDir = $currentUser ? $basePath . $currentUser . "/original/" : null;
?>

<!DOCTYPE html>
<html lang="de">
<head>
    <meta charset="UTF-8">
    <title>Multi-User Dateimanager</title>
    <style>
        body { font-family: sans-serif; max-width: 900px; margin: 20px auto; padding: 20px; background: #f9f9f9; }
        .box { background: white; padding: 20px; border-radius: 8px; box-shadow: 0 2px 4px rgba(0,0,0,0.1); margin-bottom: 20px; }
        .status { padding: 10px; margin-bottom: 10px; border-radius: 4px; }
        .success { background: #d4edda; color: #155724; }
        .error { background: #f8d7da; color: #721c24; }
        table { width: 100%; border-collapse: collapse; }
        th, td { text-align: left; padding: 10px; border-bottom: 1px solid #eee; }
        .user-badge { background: #007bff; color: white; padding: 5px 10px; border-radius: 4px; }
    </style>
</head>
<body>

    <h2>BigBrain2 Repair - File-Manager & Storage</h2>

    <div class="box">
        <h2>1. Select user directory</h2>
        <form method="post">
            <select name="selected_user" style="padding: 5px;">
                <option value="">-- Please select --</option>
                <?php foreach ($userFolders as $folder): ?>
                    <option value="<?php echo htmlspecialchars($folder); ?>" <?php if($currentUser == $folder) echo 'selected'; ?>>
                        <?php echo htmlspecialchars($folder); ?>
                    </option>
                <?php endforeach; ?>
            </select>
            <input type="submit" name="select_user" value="Ordner wechseln">
        </form>
    </div>

    <?php if ($currentUser && $targetDir): ?>
        <div class="box">
            <h2>2. Management for user:  <span class="user-badge"><?php echo htmlspecialchars($currentUser); ?></span></h2>
            
            <?php
            // --- LÖSCH-LOGIK ---
            if (isset($_GET['delete'])) {
                $fileToDelete = basename($_GET['delete']);
                $filePath = $targetDir . $fileToDelete;
                if (file_exists($filePath) && unlink($filePath)) {
                    echo "<div class='status success'>Gelöscht: $fileToDelete</div>";
                }
            }

            // --- UPLOAD-LOGIK ---
            if ($_SERVER['REQUEST_METHOD'] == 'POST' && isset($_FILES['fileToUpload'])) {
                $fileName = basename($_FILES["fileToUpload"]["name"]);
                $dest = $targetDir . $fileName;
                if (move_uploaded_file($_FILES["fileToUpload"]["tmp_name"], $dest)) {
                    echo "<div class='status success'>Successful upload in $currentUser/</div>";
                }
            }
            ?>

            <form method="post" enctype="multipart/form-data">
                <input type="file" name="fileToUpload" required>
                <input type="submit" value="In diesen Ordner hochladen">
            </form>

            <h3>Section files for user <?php echo htmlspecialchars($currentUser); ?>:</h3>
            <table>
                <tr>
                  <th>Name</th>
                  <th>Last change date</th>
                  <th>Size</th>
                  <th>Status</th>
                  <th>Actions</th>
                </tr>
                <?php
                 // get list of already repaired files
                 $ufilenames = [];
                 $repairedDir = $basePath."/".$currentUser."/repaired";
                 $files = array_diff(scandir($repairedDir), array('.', '..'));
                 foreach ($files as $file) {
                  $ident = substr(preg_replace('/\D/', '', $file), 0, 4);
                  if ( !empty($ident) ) {
                   $ufilenames[] = $ident;
                  }
                 }
                 $ufilenames = array_unique($ufilenames);
                 // process files in original folder
                 $filenames = [];
                 $files = array_diff(scandir($targetDir), array('.', '..'));
                 foreach ($files as $file) {
                   $filename = $targetDir."/".$file;
                   $filenames[] = [ 'mtime' => filemtime($filename), 'name' => $file ];
                 }
                 usort($filenames, function($a, $b) {
                  return $b['mtime'] <=> $a['mtime'];
                 });
                 foreach ($filenames as $entry):
                   $file = $entry['name'];
                   $ident = substr(preg_replace('/\D/','',$file),0,4);
                   $path = $targetDir . $file;
                   $status = ( in_array($ident,$ufilenames) ? "Processed" : "Unprocessed" );
                   $date = date("d.m.Y H:i", filemtime($path));
                   $size = round(filesize($path) / 1024, 1) . " KB";
                ?>
                    <tr>
                        <td><?php echo htmlspecialchars($file); ?></td>
                        <td class="date-col"><?= $date ?></td>
                        <td><?= $size ?></td>
                        <td><?= $status ?></td>
                        <td>
                            <a href="getImageData.php?modality=preimproved&user=<?php echo htmlspecialchars($currentUser); ?>&section=<?php echo preg_replace('/[^0-9]/','',urlencode($file)); ?>" target="_blank">View</a>
                            <a href="?download=<?= urlencode($file) ?>" class="btn btn-download">Download</a>
                            <a href="uploadSection.php?delete=<?php echo urlencode($file); ?>" 
                               onclick="return confirm('Sure?')" style="color:red;">Delete</a>
                        </td>
                    </tr>
                <?php endforeach; ?>
            </table>
        </div>
    <?php else: ?>
        <p>Bitte wählen Sie oben einen User-Ordner aus, um Dateien zu verwalten.</p>
    <?php endif; ?>

</body>
</html>
