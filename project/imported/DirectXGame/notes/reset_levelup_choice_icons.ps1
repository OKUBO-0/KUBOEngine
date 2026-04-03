Add-Type -AssemblyName System.Drawing

$baseDir = Join-Path $PSScriptRoot '..\Resources\ui\game'
$files = @(
    'lvup_normal.png',
    'lvup_orbit_add.png',
    'lvup_orbit_upgrade.png',
    'lvup_drone_add.png',
    'lvup_drone_upgrade.png',
    'lvup_attack.png',
    'lvup_speed.png',
    'lvup_heal.png',
    'lvup_maxhp.png'
)

function Get-IconRect($imgWidth, $imgHeight) {
    $x = [int]($imgWidth * 0.3414)
    $y = [int]($imgHeight * 0.2792)
    $w = [int]($imgWidth * 0.0461)
    $h = [int]($imgHeight * 0.1292)
    return [System.Drawing.Rectangle]::new($x - 10, $y - 6, $w + 20, $h + 12)
}

foreach ($name in $files) {
    $path = Join-Path $baseDir $name
    $bitmap = [System.Drawing.Bitmap]::FromFile($path)
    $g = [System.Drawing.Graphics]::FromImage($bitmap)

    $rect = Get-IconRect $bitmap.Width $bitmap.Height
    $g.FillRectangle([System.Drawing.Brushes]::Black, $rect)

    $g.Dispose()

    $tempPath = "$path.tmp.png"
    $bitmap.Save($tempPath, [System.Drawing.Imaging.ImageFormat]::Png)
    $bitmap.Dispose()
    Move-Item -LiteralPath $tempPath -Destination $path -Force
}

Write-Output 'Level-up choice images reset.'
