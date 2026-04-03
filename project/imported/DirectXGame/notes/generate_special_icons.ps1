Add-Type -AssemblyName System.Drawing

$outDir = Join-Path $PSScriptRoot '..\Resources\ui\game\icons'
if (-not (Test-Path $outDir)) {
    New-Item -ItemType Directory -Path $outDir | Out-Null
}
$templatePath = Join-Path $outDir 'icon_template.png'

$canvasWidth = 1365
$canvasHeight = 768
$boxSize = 58
$boxX = 470
$boxY = 219

function New-PenEx([int]$r, [int]$g, [int]$b, [float]$width) {
    $pen = New-Object System.Drawing.Pen([System.Drawing.Color]::FromArgb(255, $r, $g, $b), $width)
    $pen.LineJoin = [System.Drawing.Drawing2D.LineJoin]::Round
    $pen.StartCap = [System.Drawing.Drawing2D.LineCap]::Round
    $pen.EndCap = [System.Drawing.Drawing2D.LineCap]::Round
    return $pen
}

function New-BrushEx([int]$r, [int]$g, [int]$b, [int]$a = 255) {
    return New-Object System.Drawing.SolidBrush([System.Drawing.Color]::FromArgb($a, $r, $g, $b))
}

function Get-InnerRect {
    $pad = 8.0
    return [System.Drawing.RectangleF]::new($boxX + $pad, $boxY + $pad, $boxSize - $pad * 2.0, $boxSize - $pad * 2.0)
}

function Draw-Base($g) {
    $g.Clear([System.Drawing.Color]::Black)
    $borderPen = New-PenEx 255 240 0 3.0
    $g.DrawRectangle($borderPen, $boxX, $boxY, $boxSize, $boxSize)
    $borderPen.Dispose()
}

function Draw-Sword($g, $rect, $pen, $fill) {
    $white = $fill
    $black = New-BrushEx 0 0 0
    $x = $rect.X + 3
    $y = $rect.Y + 2

    $outline = [System.Drawing.PointF[]]@(
        [System.Drawing.PointF]::new($x + 18, $y + 0),
        [System.Drawing.PointF]::new($x + 24, $y + 5),
        [System.Drawing.PointF]::new($x + 23, $y + 11),
        [System.Drawing.PointF]::new($x + 18, $y + 18),
        [System.Drawing.PointF]::new($x + 12, $y + 26),
        [System.Drawing.PointF]::new($x + 7,  $y + 31),
        [System.Drawing.PointF]::new($x + 5,  $y + 29),
        [System.Drawing.PointF]::new($x + 9,  $y + 24),
        [System.Drawing.PointF]::new($x + 14, $y + 18),
        [System.Drawing.PointF]::new($x + 19, $y + 11),
        [System.Drawing.PointF]::new($x + 20, $y + 7),
        [System.Drawing.PointF]::new($x + 16, $y + 3)
    )
    $g.FillPolygon($white, $outline)

    $inner = [System.Drawing.PointF[]]@(
        [System.Drawing.PointF]::new($x + 16, $y + 6),
        [System.Drawing.PointF]::new($x + 18, $y + 8),
        [System.Drawing.PointF]::new($x + 14, $y + 14),
        [System.Drawing.PointF]::new($x + 10, $y + 19),
        [System.Drawing.PointF]::new($x + 8,  $y + 17),
        [System.Drawing.PointF]::new($x + 12, $y + 12)
    )
    $g.FillPolygon($black, $inner)

    $guard = [System.Drawing.PointF[]]@(
        [System.Drawing.PointF]::new($x + 4,  $y + 24),
        [System.Drawing.PointF]::new($x + 11, $y + 31),
        [System.Drawing.PointF]::new($x + 8,  $y + 34),
        [System.Drawing.PointF]::new($x + 1,  $y + 27)
    )
    $g.FillPolygon($white, $guard)

    $handle = [System.Drawing.PointF[]]@(
        [System.Drawing.PointF]::new($x + 7,  $y + 30),
        [System.Drawing.PointF]::new($x + 10, $y + 33),
        [System.Drawing.PointF]::new($x + 3,  $y + 40),
        [System.Drawing.PointF]::new($x + 0,  $y + 37)
    )
    $g.FillPolygon($white, $handle)
    $black.Dispose()
}

function Draw-Drone($g, $rect, $pen, $fill) {
    $cx = $rect.X + $rect.Width * 0.5
    $cy = $rect.Y + $rect.Height * 0.52
    $arm = 10.0
    $g.DrawLine($pen, $cx - $arm, $cy - $arm, $cx + $arm, $cy + $arm)
    $g.DrawLine($pen, $cx + $arm, $cy - $arm, $cx - $arm, $cy + $arm)
    $g.FillEllipse($fill, [System.Drawing.RectangleF]::new($cx - 5, $cy - 5, 10, 10))
    foreach ($pt in @(
        [System.Drawing.PointF]::new($cx - $arm, $cy - $arm),
        [System.Drawing.PointF]::new($cx + $arm, $cy - $arm),
        [System.Drawing.PointF]::new($cx - $arm, $cy + $arm),
        [System.Drawing.PointF]::new($cx + $arm, $cy + $arm)
    )) {
        $g.DrawEllipse($pen, $pt.X - 3.5, $pt.Y - 3.5, 7, 7)
    }
}

function Draw-Heart($g, $rect, $pen, $fill) {
    $cx = $rect.X + $rect.Width * 0.5
    $cy = $rect.Y + $rect.Height * 0.48
    $g.FillEllipse($fill, $cx - 11, $cy - 9, 12, 12)
    $g.FillEllipse($fill, $cx - 1, $cy - 9, 12, 12)
    $lower = [System.Drawing.PointF[]]@(
        [System.Drawing.PointF]::new($cx - 13, $cy - 1),
        [System.Drawing.PointF]::new($cx + 13, $cy - 1),
        [System.Drawing.PointF]::new($cx, $cy + 17)
    )
    $g.FillPolygon($fill, $lower)
}

function Draw-Burst($g, $rect, $pen, $fill) {
    $cx = $rect.X + $rect.Width * 0.5
    $cy = $rect.Y + $rect.Height * 0.5
    $r1 = 6.0
    $r2 = 15.0
    $points = New-Object 'System.Collections.Generic.List[System.Drawing.PointF]'
    for ($i = 0; $i -lt 8; $i++) {
        $angleOuter = (-90 + $i * 45) * [Math]::PI / 180.0
        $angleInner = (-67.5 + $i * 45) * [Math]::PI / 180.0
        $points.Add([System.Drawing.PointF]::new($cx + [Math]::Cos($angleOuter) * $r2, $cy + [Math]::Sin($angleOuter) * $r2))
        $points.Add([System.Drawing.PointF]::new($cx + [Math]::Cos($angleInner) * $r1, $cy + [Math]::Sin($angleInner) * $r1))
    }
    $array = $points.ToArray()
    $g.FillPolygon($fill, $array)
    $g.DrawPolygon($pen, $array)
}

function Draw-Orb($g, $rect, $pen) {
    $cx = $rect.X + $rect.Width * 0.5
    $cy = $rect.Y + $rect.Height * 0.5
    $orbRect = [System.Drawing.RectangleF]::new($cx - 12, $cy - 12, 24, 24)
    $grad = New-Object System.Drawing.Drawing2D.PathGradientBrush(([System.Drawing.Drawing2D.GraphicsPath]::new()))
}

function Save-Icon($name, $drawer) {
    $bitmap = [System.Drawing.Bitmap]::FromFile($templatePath)
    $g = [System.Drawing.Graphics]::FromImage($bitmap)
    $g.SmoothingMode = [System.Drawing.Drawing2D.SmoothingMode]::AntiAlias

    $rect = Get-InnerRect
    $whitePen = New-PenEx 250 250 250 3.0
    $whiteBrush = New-BrushEx 250 250 250

    & $drawer $g $rect $whitePen $whiteBrush

    $path = Join-Path $outDir $name
    $bitmap.Save($path, [System.Drawing.Imaging.ImageFormat]::Png)

    $whitePen.Dispose()
    $whiteBrush.Dispose()
    $g.Dispose()
    $bitmap.Dispose()
}

$templateBitmap = New-Object System.Drawing.Bitmap($canvasWidth, $canvasHeight)
$templateGraphics = [System.Drawing.Graphics]::FromImage($templateBitmap)
$templateGraphics.SmoothingMode = [System.Drawing.Drawing2D.SmoothingMode]::AntiAlias
Draw-Base $templateGraphics
$templateBitmap.Save($templatePath, [System.Drawing.Imaging.ImageFormat]::Png)
$templateGraphics.Dispose()
$templateBitmap.Dispose()

function Draw-OrbIcon($g, $rect, $pen, $fill) {
    $cx = $rect.X + $rect.Width * 0.5
    $cy = $rect.Y + $rect.Height * 0.5
    $outer = [System.Drawing.RectangleF]::new($cx - 12, $cy - 12, 24, 24)
    $inner = [System.Drawing.RectangleF]::new($cx - 6, $cy - 6, 12, 12)
    $g.DrawEllipse($pen, $outer)
    $g.FillEllipse($fill, $inner)
    $g.DrawArc($pen, $outer, 210, 120)
    $g.DrawArc($pen, $outer, 20, 120)
}

Save-Icon 'icon_sword.png' ${function:Draw-Sword}
Save-Icon 'icon_drone.png' ${function:Draw-Drone}
Save-Icon 'icon_heart.png' ${function:Draw-Heart}
Save-Icon 'icon_burst.png' ${function:Draw-Burst}
Save-Icon 'icon_orb.png' ${function:Draw-OrbIcon}

Write-Output $outDir
