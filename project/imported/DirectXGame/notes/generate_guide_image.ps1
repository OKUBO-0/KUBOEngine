Add-Type -AssemblyName System.Drawing

$width = 1280
$height = 720
$bitmap = New-Object System.Drawing.Bitmap($width, $height)
$g = [System.Drawing.Graphics]::FromImage($bitmap)
$g.SmoothingMode = [System.Drawing.Drawing2D.SmoothingMode]::AntiAlias
$g.TextRenderingHint = [System.Drawing.Text.TextRenderingHint]::AntiAliasGridFit

$bg = New-Object System.Drawing.Drawing2D.LinearGradientBrush(
    ([System.Drawing.Rectangle]::new(0, 0, $width, $height)),
    ([System.Drawing.Color]::FromArgb(255, 8, 11, 18)),
    ([System.Drawing.Color]::FromArgb(255, 20, 24, 34)),
    90.0
)
$g.FillRectangle($bg, 0, 0, $width, $height)

$gridPen = New-Object System.Drawing.Pen([System.Drawing.Color]::FromArgb(18, 255, 255, 255), 1)
for ($x = 0; $x -lt $width; $x += 40) {
    $g.DrawLine($gridPen, ([System.Drawing.Point]::new($x, 0)), ([System.Drawing.Point]::new($x, $height)))
}
for ($y = 0; $y -lt $height; $y += 40) {
    $g.DrawLine($gridPen, ([System.Drawing.Point]::new(0, $y)), ([System.Drawing.Point]::new($width, $y)))
}

function New-Brush([int]$r, [int]$g, [int]$b, [int]$a = 255) {
    return New-Object System.Drawing.SolidBrush([System.Drawing.Color]::FromArgb($a, $r, $g, $b))
}

$white = New-Brush 245 247 250
$muted = New-Brush 170 181 199
$cyan = New-Brush 76 221 255
$orange = New-Brush 255 174 92
$panelBrush = New-Brush 15 20 31 220
$footerBrush = New-Brush 140 152 171
$panelBorder = New-Object System.Drawing.Pen([System.Drawing.Color]::FromArgb(160, 74, 92, 120), 2)
$accentPen = New-Object System.Drawing.Pen([System.Drawing.Color]::FromArgb(220, 76, 221, 255), 5)
$accentPen2 = New-Object System.Drawing.Pen([System.Drawing.Color]::FromArgb(220, 255, 174, 92), 5)

$fontTitle = New-Object System.Drawing.Font('Yu Gothic', 34, [System.Drawing.FontStyle]::Bold)
$fontSection = New-Object System.Drawing.Font('Yu Gothic', 20, [System.Drawing.FontStyle]::Bold)
$fontBody = New-Object System.Drawing.Font('Yu Gothic', 14, [System.Drawing.FontStyle]::Bold)
$fontSmall = New-Object System.Drawing.Font('Yu Gothic', 11, [System.Drawing.FontStyle]::Bold)
$fontChip = New-Object System.Drawing.Font('Yu Gothic', 18, [System.Drawing.FontStyle]::Bold)

$format = New-Object System.Drawing.StringFormat
$format.Alignment = [System.Drawing.StringAlignment]::Near
$format.LineAlignment = [System.Drawing.StringAlignment]::Near

function Draw-StringRect {
    param(
        [string]$Text,
        [System.Drawing.Font]$Font,
        [System.Drawing.Brush]$Brush,
        [float]$X,
        [float]$Y,
        [float]$W,
        [float]$H
    )

    $rect = [System.Drawing.RectangleF]::new($X, $Y, $W, $H)
    $g.DrawString($Text, $Font, $Brush, $rect, $format)
}

function Draw-Section {
    param(
        [int]$X,
        [int]$Y,
        [int]$W,
        [int]$H,
        [string]$Title,
        [System.Drawing.Pen]$Pen,
        [System.Drawing.Brush]$TitleBrush
    )

    $g.FillRectangle($panelBrush, $X, $Y, $W, $H)
    $g.DrawRectangle($panelBorder, $X, $Y, $W, $H)
    $g.DrawLine($Pen, ([System.Drawing.Point]::new($X + 18, $Y + 24)), ([System.Drawing.Point]::new($X + 118, $Y + 24)))
    Draw-StringRect $Title $fontSection $TitleBrush ($X + 18) ($Y + 32) ($W - 36) 32
}

$g.FillRectangle($panelBrush, 36, 28, 360, 58)
$g.DrawRectangle($panelBorder, 36, 28, 360, 58)
Draw-StringRect '閉じる  ESC / B' $fontChip $white 52 38 316 32

Draw-StringRect 'GAME GUIDE' $fontSmall $cyan 56 108 180 24
Draw-StringRect '操作ガイド' $fontTitle $white 52 126 360 48
Draw-StringRect 'ゲーム中に必要な操作と強化内容をまとめたガイド' $fontBody $muted 56 176 520 28

Draw-Section 52 218 546 222 '操作方法' $accentPen $cyan
Draw-Section 52 458 546 210 '攻撃の種類' $accentPen2 $orange
Draw-Section 624 218 604 450 'レベルアップ' $accentPen $cyan

$controls = @(
    '移動        WASD / 左スティック',
    '照準        マウス / 右スティック',
    '決定        Space / Enter / A',
    '戻る        ESC / B',
    'ポーズ      ESC / Start'
)
$cy = 292
foreach ($line in $controls) {
    Draw-StringRect '■' $fontBody $cyan 74 $cy 18 24
    Draw-StringRect $line $fontBody $white 104 ($cy - 2) 430 26
    $cy += 30
}

$attacks = @(
    '通常弾      向いている方向へ自動で連射',
    '衛星弾      プレイヤーの周囲を回転して攻撃',
    'ドローン    敵を狙って自動で弾を発射'
)
$ay = 532
foreach ($line in $attacks) {
    Draw-StringRect '●' $fontBody $orange 74 $ay 18 24
    Draw-StringRect $line $fontBody $white 104 ($ay - 2) 430 34
    $ay += 46
}

Draw-StringRect 'レベルアップ時は3つの候補から1つを選択' $fontBody $muted 646 292 520 28

$upgrades = @(
    '通常弾強化      発射間隔が短くなる',
    '衛星弾強化      未所持なら追加、所持済みなら弾数増加',
    'ドローン        未所持なら追加、所持済みなら攻撃間隔短縮',
    '攻撃力 +1       すべての攻撃ダメージが上がる',
    '移動速度アップ  移動速度が上昇する',
    'HP回復          HPを1回復する',
    '最大HP増加      最大HPが1上がり、HPも全回復する'
)
$uy = 346
foreach ($line in $upgrades) {
    Draw-StringRect '◆' $fontBody $cyan 648 $uy 18 24
    Draw-StringRect $line $fontBody $white 676 ($uy - 2) 520 34
    $uy += 45
}

Draw-StringRect 'タイトル画面とポーズ画面の両方で表示される共通ガイド' $fontSmall $footerBrush 52 688 520 20

$outPath = Join-Path $PSScriptRoot '..\\Resources\\ui\\title\\guideUI.png'
$resolved = [System.IO.Path]::GetFullPath($outPath)
$bitmap.Save($resolved, [System.Drawing.Imaging.ImageFormat]::Png)

$fontTitle.Dispose()
$fontSection.Dispose()
$fontBody.Dispose()
$fontSmall.Dispose()
$fontChip.Dispose()
$white.Dispose()
$muted.Dispose()
$cyan.Dispose()
$orange.Dispose()
$panelBrush.Dispose()
$footerBrush.Dispose()
$panelBorder.Dispose()
$accentPen.Dispose()
$accentPen2.Dispose()
$gridPen.Dispose()
$bg.Dispose()
$format.Dispose()
$g.Dispose()
$bitmap.Dispose()

Write-Output $resolved

