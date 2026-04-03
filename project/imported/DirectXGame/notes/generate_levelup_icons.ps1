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

function Get-IconRect($imgWidth, $imgHeight) {
    $x = [int]($imgWidth * 0.3414)
    $y = [int]($imgHeight * 0.2792)
    $w = [int]($imgWidth * 0.0461)
    $h = [int]($imgHeight * 0.1292)
    return [System.Drawing.RectangleF]::new($x, $y, $w, $h)
}

function Get-InnerRect([System.Drawing.RectangleF]$rect) {
    $pad = [Math]::Max(4.0, [Math]::Min($rect.Width, $rect.Height) * 0.12)
    return [System.Drawing.RectangleF]::new($rect.X + $pad, $rect.Y + $pad, $rect.Width - $pad * 2.0, $rect.Height - $pad * 2.0)
}

function Draw-Plus($g, $cx, $cy, $size, $pen) {
    $g.DrawLine($pen, $cx - $size, $cy, $cx + $size, $cy)
    $g.DrawLine($pen, $cx, $cy - $size, $cx, $cy + $size)
}

function Draw-UpArrow($g, $cx, $cy, $size, $pen) {
    $g.DrawLine($pen, $cx, $cy + $size * 0.8, $cx, $cy - $size * 0.8)
    $g.DrawLine($pen, $cx, $cy - $size * 0.8, $cx - $size * 0.55, $cy - $size * 0.2)
    $g.DrawLine($pen, $cx, $cy - $size * 0.8, $cx + $size * 0.55, $cy - $size * 0.2)
}

function Draw-Bullet($g, [System.Drawing.RectangleF]$rect, $pen, $fill) {
    $cx = $rect.X + $rect.Width * 0.56
    $cy = $rect.Y + $rect.Height * 0.5
    $bodyW = $rect.Width * 0.28
    $bodyH = $rect.Height * 0.24
    $trailLeft = $rect.X + $rect.Width * 0.24
    $trailRight = $cx - $bodyW * 0.82
    $trailPen = $pen.Clone()
    $trailPen.StartCap = [System.Drawing.Drawing2D.LineCap]::Flat
    $trailPen.EndCap = [System.Drawing.Drawing2D.LineCap]::Flat
    $g.DrawLine($trailPen, $trailLeft, $cy - $bodyH * 0.52, $trailRight, $cy - $bodyH * 0.52)
    $g.DrawLine($trailPen, $trailLeft + 1.5, $cy, $trailRight, $cy)
    $g.DrawLine($trailPen, $trailLeft, $cy + $bodyH * 0.52, $trailRight, $cy + $bodyH * 0.52)
    $body = [System.Drawing.RectangleF]::new($cx - $bodyW * 0.55, $cy - $bodyH * 0.5, $bodyW, $bodyH)
    $g.FillEllipse($fill, $body)
    $tip = [System.Drawing.PointF[]]@(
        [System.Drawing.PointF]::new($body.Right + $bodyW * 0.22, $cy),
        [System.Drawing.PointF]::new($body.Right - $bodyW * 0.03, $cy - $bodyH * 0.75),
        [System.Drawing.PointF]::new($body.Right - $bodyW * 0.03, $cy + $bodyH * 0.75)
    )
    $g.FillPolygon($fill, $tip)
    $trailPen.Dispose()
}

function Draw-Orbit($g, [System.Drawing.RectangleF]$rect, $pen, $fill, [string]$badge) {
    $cx = $rect.X + $rect.Width * 0.5
    $cy = $rect.Y + $rect.Height * 0.54
    $planetSize = [Math]::Min($rect.Width, $rect.Height) * 0.22
    $orbit = [System.Drawing.RectangleF]::new($cx - $rect.Width * 0.32, $cy - $rect.Height * 0.2, $rect.Width * 0.64, $rect.Height * 0.4)
    $g.DrawEllipse($pen, $orbit)
    $g.FillEllipse($fill, [System.Drawing.RectangleF]::new($cx - $planetSize * 0.5, $cy - $planetSize * 0.5, $planetSize, $planetSize))
    $g.FillEllipse($fill, [System.Drawing.RectangleF]::new($orbit.X + 2, $cy - 3, 6, 6))
    $g.FillEllipse($fill, [System.Drawing.RectangleF]::new($orbit.Right - 8, $cy - 3, 6, 6))
    if ($badge -eq 'plus') {
        Draw-Plus $g ($rect.Right - 6) ($rect.Y + 6) 3.5 $pen
    } elseif ($badge -eq 'up') {
        Draw-UpArrow $g ($rect.Right - 6) ($rect.Y + 8) 5 $pen
    }
}

function Draw-Drone($g, [System.Drawing.RectangleF]$rect, $pen, $fill, [string]$badge) {
    $cx = $rect.X + $rect.Width * 0.5
    $cy = $rect.Y + $rect.Height * 0.54
    $arm = [Math]::Min($rect.Width, $rect.Height) * 0.26
    $g.DrawLine($pen, $cx - $arm, $cy - $arm, $cx + $arm, $cy + $arm)
    $g.DrawLine($pen, $cx + $arm, $cy - $arm, $cx - $arm, $cy + $arm)
    $body = [System.Drawing.RectangleF]::new($cx - 6, $cy - 6, 12, 12)
    $g.FillEllipse($fill, $body)
    foreach ($pt in @(
        [System.Drawing.PointF]::new($cx - $arm, $cy - $arm),
        [System.Drawing.PointF]::new($cx + $arm, $cy - $arm),
        [System.Drawing.PointF]::new($cx - $arm, $cy + $arm),
        [System.Drawing.PointF]::new($cx + $arm, $cy + $arm)
    )) {
        $g.DrawEllipse($pen, $pt.X - 3, $pt.Y - 3, 6, 6)
    }
    if ($badge -eq 'plus') {
        Draw-Plus $g ($rect.Right - 6) ($rect.Y + 6) 3.5 $pen
    } elseif ($badge -eq 'up') {
        Draw-UpArrow $g ($rect.Right - 6) ($rect.Y + 8) 5 $pen
    }
}

function Draw-Starburst($g, [System.Drawing.RectangleF]$rect, $pen, $fill) {
    $cx = $rect.X + $rect.Width * 0.5
    $cy = $rect.Y + $rect.Height * 0.54
    $r1 = [Math]::Min($rect.Width, $rect.Height) * 0.14
    $r2 = [Math]::Min($rect.Width, $rect.Height) * 0.36
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

function Draw-Speed($g, [System.Drawing.RectangleF]$rect, $pen, $fill) {
    $points = [System.Drawing.PointF[]]@(
        [System.Drawing.PointF]::new($rect.X + $rect.Width * 0.22, $rect.Y + $rect.Height * 0.74),
        [System.Drawing.PointF]::new($rect.X + $rect.Width * 0.5, $rect.Y + $rect.Height * 0.16),
        [System.Drawing.PointF]::new($rect.X + $rect.Width * 0.48, $rect.Y + $rect.Height * 0.46),
        [System.Drawing.PointF]::new($rect.X + $rect.Width * 0.8, $rect.Y + $rect.Height * 0.28),
        [System.Drawing.PointF]::new($rect.X + $rect.Width * 0.56, $rect.Y + $rect.Height * 0.82),
        [System.Drawing.PointF]::new($rect.X + $rect.Width * 0.56, $rect.Y + $rect.Height * 0.54)
    )
    $g.FillPolygon($fill, $points)
    $g.DrawPolygon($pen, $points)
}

function Draw-Heart($g, [System.Drawing.RectangleF]$rect, $pen, $fill, [string]$badge) {
    $cx = $rect.X + $rect.Width * 0.44
    $cy = $rect.Y + $rect.Height * 0.48
    $r = [Math]::Min($rect.Width, $rect.Height) * 0.17
    $g.FillEllipse($fill, [System.Drawing.RectangleF]::new($cx - $r * 1.6, $cy - $r, $r * 2, $r * 2))
    $g.FillEllipse($fill, [System.Drawing.RectangleF]::new($cx - $r * 0.2, $cy - $r, $r * 2, $r * 2))
    $tri = [System.Drawing.PointF[]]@(
        [System.Drawing.PointF]::new($cx - $r * 2.0, $cy + $r * 0.05),
        [System.Drawing.PointF]::new($cx + $r * 1.8, $cy + $r * 0.05),
        [System.Drawing.PointF]::new($cx - $r * 0.1, $cy + $r * 2.4)
    )
    $g.FillPolygon($fill, $tri)
    if ($badge -eq 'plus') {
        Draw-Plus $g ($rect.Right - 6) ($rect.Y + 6) 3.5 $pen
    } elseif ($badge -eq 'shield') {
        $shield = [System.Drawing.PointF[]]@(
            [System.Drawing.PointF]::new($rect.Right - 14, $rect.Y + 4),
            [System.Drawing.PointF]::new($rect.Right - 2, $rect.Y + 4),
            [System.Drawing.PointF]::new($rect.Right - 2, $rect.Y + 12),
            [System.Drawing.PointF]::new($rect.Right - 8, $rect.Y + 18),
            [System.Drawing.PointF]::new($rect.Right - 14, $rect.Y + 12)
        )
        $g.DrawPolygon($pen, $shield)
    }
}

foreach ($name in $files) {
    $path = Join-Path $baseDir $name
    $bitmap = [System.Drawing.Bitmap]::FromFile($path)
    $g = [System.Drawing.Graphics]::FromImage($bitmap)
    $g.SmoothingMode = [System.Drawing.Drawing2D.SmoothingMode]::AntiAlias

    $iconRect = Get-IconRect $bitmap.Width $bitmap.Height
    $innerRect = Get-InnerRect $iconRect
    $background = New-BrushEx 0 0 0
    $outline = New-PenEx 255 240 0 ([Math]::Max(2.0, $bitmap.Width / 600.0))
    $pen = New-PenEx 250 250 250 ([Math]::Max(2.2, $bitmap.Width / 520.0))
    $fill = New-BrushEx 250 250 250

    $g.FillRectangle($background, $iconRect)
    $g.DrawRectangle($outline, $iconRect.X, $iconRect.Y, $iconRect.Width, $iconRect.Height)

    switch ($name) {
        'lvup_normal.png' { Draw-Bullet $g $innerRect $pen $fill }
        'lvup_orbit_add.png' { Draw-Orbit $g $innerRect $pen $fill 'plus' }
        'lvup_orbit_upgrade.png' { Draw-Orbit $g $innerRect $pen $fill 'up' }
        'lvup_drone_add.png' { Draw-Drone $g $innerRect $pen $fill 'plus' }
        'lvup_drone_upgrade.png' { Draw-Drone $g $innerRect $pen $fill 'up' }
        'lvup_attack.png' { Draw-Starburst $g $innerRect $pen $fill }
        'lvup_speed.png' { Draw-Speed $g $innerRect $pen $fill }
        'lvup_heal.png' { Draw-Heart $g $innerRect $pen $fill 'plus' }
        'lvup_maxhp.png' { Draw-Heart $g $innerRect $pen $fill 'shield' }
    }

    $background.Dispose()
    $outline.Dispose()
    $pen.Dispose()
    $fill.Dispose()
    $g.Dispose()

    $tempPath = "$path.tmp.png"
    $bitmap.Save($tempPath, [System.Drawing.Imaging.ImageFormat]::Png)
    $bitmap.Dispose()
    Move-Item -LiteralPath $tempPath -Destination $path -Force
}

Write-Output 'Level-up icons updated.'
