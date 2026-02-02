<?php

$headertext = "JulichBrain BigBrainViewer Admin Tools: Labteam work overview";
$baseDataDir = "/var/www/upload/bigbrainviewer/user";
$allowedExtensions = ['png', 'xcf'];
$skipuser = ["all", "temp", "backup"];

// 1. User-Liste generieren
$users = [];
if (is_dir($baseDataDir)) {
    $dirs = array_diff(scandir($baseDataDir), array('.', '..'));
    foreach ($dirs as $dir) {
        if (is_dir("$baseDataDir/$dir") && !in_array($dir, $skipuser)) {
            $users[] = $dir;
        }
    }
}
sort($users);

$allNumbers = [];
$userFiles = [];

// 2. Daten einlesen
foreach ($users as $user) {
    $userFiles[$user] = ['original' => [], 'repaired' => []];
    foreach (['original', 'repaired'] as $type) {
        $path = "$baseDataDir/$user/$type";
        if (is_dir($path)) {
            $files = array_diff(scandir($path), array('.', '..'));
            foreach ($files as $f) {
                if (preg_match('/(?<!\d)(\d{4})(?!\d)/', $f, $matches)) {
                    $ext = strtolower(pathinfo($f, PATHINFO_EXTENSION));
                    if (in_array($ext, $allowedExtensions)) {
                        $imgId = $matches[1];
                        $userFiles[$user][$type][$imgId] = true;
                        $allNumbers[$imgId] = true;
                    }
                }
            }
        }
    }
}
ksort($allNumbers);

// 3. CSS
echo '<style>
    :root { --bg: #f1f5f9; --blue: #3b82f6; --green: #10b981; --border: #e2e8f0; --top-header-bg: #5D7691; }
    body, html { margin: 0; padding: 0; height: 100vh; overflow: hidden; font-family: "Inter", sans-serif; background: var(--bg); }
    
    /* Der neue Top-Header */
    .top-header { 
        background-color: var(--top-header-bg); 
        color: white; 
        padding: 10px 20px; 
        width: 100%; 
        box-sizing: border-box;
        font-familiy: arial, sans-serif;
        font-weight: 500;
        font-size: 20px;
        flex: 0 0 auto;
    }

    .dashboard { display: flex; flex-direction: column; height: 100vh; box-sizing: border-box; }
    .content-area { display: flex; flex-direction: column; flex: 1; padding: 15px; overflow: hidden; }

    .header-controls { flex: 0 0 auto; display: flex; justify-content: space-between; align-items: center; margin-bottom: 10px; }
    .table-section { flex: 1; overflow: hidden; background: white; border-radius: 8px; border: 1px solid var(--border); display: flex; flex-direction: column; }
    .scroll-container { flex: 1; overflow: auto; }
    
    table { border-collapse: separate; border-spacing: 0; width: 100%; }
    th { background: #f8fafc; padding: 10px; position: sticky; top: 0; z-index: 20; font-size: 11px; border-bottom: 2px solid var(--border); }
    .id-col { position: sticky; left: 0; background: white; z-index: 10; font-weight: bold; width: 70px; padding-left: 10px; border-right: 1px solid var(--border); }
    th.id-col { z-index: 30; background: #f8fafc; }
    td { padding: 8px; border-bottom: 1px solid var(--border); text-align: center; }
    
    .dot { height: 10px; width: 10px; border-radius: 50%; display: inline-block; background: #e2e8f0; }
    .dot.blue { background: var(--blue); }
    .dot.green { background: var(--green); }
    
    .footer { flex: 0 0 auto; display: flex; gap: 10px; margin-top: 10px; overflow-x: auto; padding-bottom: 5px; }
    .card { background: white; padding: 10px; border-radius: 6px; min-width: 140px; box-shadow: 0 1px 3px rgba(0,0,0,0.1); border-left: 4px solid var(--blue); }
    .card h4 { margin: 0; font-size: 12px; color: #64748b; }
    .card .val { font-size: 16px; font-weight: bold; margin: 2px 0; }
    #searchInput { padding: 6px 12px; width: 200px; border-radius: 4px; border: 1px solid var(--border); }
</style>';

// 4. Struktur
echo '<div class="dashboard">';
    // Globaler Top-Header
    echo '<div class="top-header">' . htmlspecialchars($headertext) . '</div>';

    echo '<div class="content-area">';
        echo '<div class="header-controls">
                <h3 style="margin:0;">Status Monitor</h3>
                <div>
                    <input type="text" id="searchInput" placeholder="ID filtern..." onkeyup="updateView()">
                    <span id="rowCount" style="font-size:12px; margin-left:10px; color:#64748b;"></span>
                </div>
              </div>';

        echo '<div class="table-section">
                <div class="scroll-container">
                    <table id="statusTable">
                        <thead><tr><th class="id-col">ID</th>';
                        foreach ($users as $user) echo "<th>$user</th>";
        echo '          </tr></thead>
                        <tbody>';
                        foreach ($allNumbers as $id => $dummy) {
                            echo "<tr><td class='id-col'>#$id</td>";
                            foreach ($users as $user) {
                                $type = '';
                                if (isset($userFiles[$user]['repaired'][$id])) $type = 'green';
                                elseif (isset($userFiles[$user]['original'][$id])) $type = 'blue';
                                echo "<td data-user='$user' data-type='$type'><span class='dot $type'></span></td>";
                            }
                            echo "</tr>";
                        }
        echo '          </tbody>
                    </table>
                </div>
              </div>';

        echo '<div class="footer" id="statsFooter">';
        foreach ($users as $user) {
            echo "<div class='card' id='card-$user'>
                    <h4>$user</h4>
                    <div class='val' id='total-$user'>0</div>
                    <div style='font-size:10px;'><span style='color:var(--blue)'>●</span> <span id='blue-$user'>0</span> | <span style='color:var(--green)'>●</span> <span id='green-$user'>0</span></div>
                  </div>";
        }
        echo '</div>';
    echo '</div>'; // End content-area
echo '</div>'; // End dashboard
?>

<script>
function updateView() {
    const filter = document.getElementById("searchInput").value.toUpperCase();
    const rows = document.getElementById("statusTable").getElementsByTagName("tbody")[0].rows;
    const stats = {};
    <?php foreach ($users as $u) echo "stats['$u'] = {blue:0, green:0};\n"; ?>

    let visibleCount = 0;
    for (let i = 0; i < rows.length; i++) {
        const idCell = rows[i].cells[0];
        const match = idCell.textContent.toUpperCase().indexOf(filter) > -1;
        if (match) {
            rows[i].style.display = "";
            visibleCount++;
            for (let j = 1; j < rows[i].cells.length; j++) {
                const cell = rows[i].cells[j];
                const user = cell.getAttribute('data-user');
                const type = cell.getAttribute('data-type');
                if (type === 'blue') stats[user].blue++;
                if (type === 'green') stats[user].green++;
            }
        } else {
            rows[i].style.display = "none";
        }
    }

    document.getElementById("rowCount").innerText = "Sichtbar: " + visibleCount;
    for (const user in stats) {
        document.getElementById('blue-' + user).innerText = stats[user].blue;
        document.getElementById('green-' + user).innerText = stats[user].green;
        document.getElementById('total-' + user).innerText = stats[user].blue + stats[user].green;
    }
}
window.onload = updateView;
</script>
