# Draw the launcher icon from vector primitives; independent of game textures.
$ErrorActionPreference = 'Stop'
Add-Type -AssemblyName System.Drawing
$bitmap = New-Object System.Drawing.Bitmap 512,512
$graphics = [System.Drawing.Graphics]::FromImage($bitmap)
$graphics.SmoothingMode = [System.Drawing.Drawing2D.SmoothingMode]::AntiAlias
$graphics.TextRenderingHint = [System.Drawing.Text.TextRenderingHint]::AntiAliasGridFit
$background = New-Object System.Drawing.SolidBrush ([System.Drawing.Color]::FromArgb(8,17,31))
$silver = New-Object System.Drawing.SolidBrush ([System.Drawing.Color]::FromArgb(216,230,242))
$cyan = New-Object System.Drawing.SolidBrush ([System.Drawing.Color]::FromArgb(93,203,230))
$muted = New-Object System.Drawing.SolidBrush ([System.Drawing.Color]::FromArgb(105,140,166))
$pen = New-Object System.Drawing.Pen ([System.Drawing.Color]::FromArgb(40,72,98)),2
$font = New-Object System.Drawing.Font 'Arial',33,([System.Drawing.FontStyle]::Bold),([System.Drawing.GraphicsUnit]::Pixel)
$small = New-Object System.Drawing.Font 'Arial',16,([System.Drawing.FontStyle]::Regular),([System.Drawing.GraphicsUnit]::Pixel)
$format = New-Object System.Drawing.StringFormat
$format.Alignment = [System.Drawing.StringAlignment]::Center
try {
    $graphics.FillRectangle($background,0,0,512,512)
    for($i=0;$i -lt 45;$i++) {
        $x=($i*137+31)%512; $y=($i*83+19)%512
        $graphics.FillEllipse($muted,$x,$y,2,2)
    }
    $graphics.DrawEllipse($pen,103,66,306,306)
    $points = [System.Drawing.PointF[]]@(
        [System.Drawing.PointF]::new(256,104),[System.Drawing.PointF]::new(382,307),
        [System.Drawing.PointF]::new(276,261),[System.Drawing.PointF]::new(256,293),
        [System.Drawing.PointF]::new(236,261),[System.Drawing.PointF]::new(130,307))
    $graphics.FillPolygon($silver,$points)
    $graphics.FillPolygon($cyan,[System.Drawing.PointF[]]@(
        [System.Drawing.PointF]::new(256,167),[System.Drawing.PointF]::new(269,246),[System.Drawing.PointF]::new(243,246)))
    $graphics.DrawString('BATTLESTAR',$font,$silver,[System.Drawing.RectangleF]::new(0,388,512,48),$format)
    $graphics.DrawString('COMBAT PROTOTYPE',$small,$cyan,[System.Drawing.RectangleF]::new(0,443,512,30),$format)
    $destination=Join-Path $PSScriptRoot 'sce_sys'
    New-Item -ItemType Directory -Path $destination -Force | Out-Null
    $bitmap.Save((Join-Path $destination 'icon0.png'),[System.Drawing.Imaging.ImageFormat]::Png)
} finally {
    $format.Dispose(); $small.Dispose(); $font.Dispose(); $pen.Dispose()
    $muted.Dispose(); $cyan.Dispose(); $silver.Dispose(); $background.Dispose()
    $graphics.Dispose(); $bitmap.Dispose()
}
