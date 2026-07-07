$ErrorActionPreference = "Stop"

$repoRoot = Split-Path -Parent $PSScriptRoot
$source = Join-Path $repoRoot "assets\pet\dog_medium.png"
$output = Join-Path $repoRoot "main\pet_dog_sprites.c"

Add-Type -AssemblyName System.Drawing

$bmp = [System.Drawing.Bitmap]::FromFile($source)
$frameWidth = 60
$frameHeight = 38

$frames = @(
    @{ Name = "pet_dog_bark_0"; Row = 0; Col = 0 }, @{ Name = "pet_dog_bark_1"; Row = 0; Col = 1 },
    @{ Name = "pet_dog_bark_2"; Row = 0; Col = 2 }, @{ Name = "pet_dog_bark_3"; Row = 0; Col = 3 },
    @{ Name = "pet_dog_bark_4"; Row = 0; Col = 4 }, @{ Name = "pet_dog_bark_5"; Row = 0; Col = 5 },
    @{ Name = "pet_dog_walk_0"; Row = 1; Col = 0 }, @{ Name = "pet_dog_walk_1"; Row = 1; Col = 1 },
    @{ Name = "pet_dog_walk_2"; Row = 1; Col = 2 }, @{ Name = "pet_dog_walk_3"; Row = 1; Col = 3 },
    @{ Name = "pet_dog_walk_4"; Row = 1; Col = 4 }, @{ Name = "pet_dog_walk_5"; Row = 1; Col = 5 },
    @{ Name = "pet_dog_run_0"; Row = 2; Col = 0 }, @{ Name = "pet_dog_run_1"; Row = 2; Col = 1 },
    @{ Name = "pet_dog_run_2"; Row = 2; Col = 2 }, @{ Name = "pet_dog_run_3"; Row = 2; Col = 3 },
    @{ Name = "pet_dog_run_4"; Row = 2; Col = 4 }, @{ Name = "pet_dog_run_5"; Row = 2; Col = 5 },
    @{ Name = "pet_dog_sit_0"; Row = 4; Col = 0 }, @{ Name = "pet_dog_sit_1"; Row = 4; Col = 1 },
    @{ Name = "pet_dog_sit_2"; Row = 4; Col = 2 }, @{ Name = "pet_dog_sit_3"; Row = 4; Col = 3 },
    @{ Name = "pet_dog_sit_4"; Row = 4; Col = 4 }, @{ Name = "pet_dog_sit_5"; Row = 4; Col = 5 },
    @{ Name = "pet_dog_idle_0"; Row = 5; Col = 0 }, @{ Name = "pet_dog_idle_1"; Row = 5; Col = 1 },
    @{ Name = "pet_dog_idle_2"; Row = 5; Col = 2 }, @{ Name = "pet_dog_idle_3"; Row = 5; Col = 3 },
    @{ Name = "pet_dog_idle_4"; Row = 5; Col = 4 }, @{ Name = "pet_dog_idle_5"; Row = 5; Col = 5 }
)

function Get-Rgb565([System.Drawing.Color]$color) {
    $r = ($color.R -shr 3) -band 0x1f
    $g = ($color.G -shr 2) -band 0x3f
    $b = ($color.B -shr 3) -band 0x1f
    return ($r -shl 11) -bor ($g -shl 5) -bor $b
}

function Write-FrameData([System.IO.StreamWriter]$writer, [hashtable]$frame, [bool]$swap) {
    $items = New-Object System.Collections.Generic.List[string]

    for ($y = 0; $y -lt $frameHeight; $y++) {
        $items.Clear()

        for ($x = 0; $x -lt $frameWidth; $x++) {
            $color = $bmp.GetPixel($frame.Col * $frameWidth + $x, $frame.Row * $frameHeight + $y)
            $rgb565 = Get-Rgb565 $color
            $hi = ($rgb565 -shr 8) -band 0xff
            $lo = $rgb565 -band 0xff

            if ($swap) {
                [void]$items.Add(("0x{0:x2}" -f $lo))
                [void]$items.Add(("0x{0:x2}" -f $hi))
            }
            else {
                [void]$items.Add(("0x{0:x2}" -f $hi))
                [void]$items.Add(("0x{0:x2}" -f $lo))
            }

            [void]$items.Add(("0x{0:x2}" -f $color.A))
        }

        $writer.WriteLine("  " + (($items.ToArray()) -join ", ") + ",")
    }
}

$encoding = New-Object System.Text.UTF8Encoding($false)
$writer = New-Object System.IO.StreamWriter($output, $false, $encoding)

try {
    $writer.WriteLine("#ifdef __has_include")
    $writer.WriteLine("    #if __has_include(""lvgl.h"")")
    $writer.WriteLine("        #ifndef LV_LVGL_H_INCLUDE_SIMPLE")
    $writer.WriteLine("            #define LV_LVGL_H_INCLUDE_SIMPLE")
    $writer.WriteLine("        #endif")
    $writer.WriteLine("    #endif")
    $writer.WriteLine("#endif")
    $writer.WriteLine("")
    $writer.WriteLine("#if defined(LV_LVGL_H_INCLUDE_SIMPLE)")
    $writer.WriteLine("    #include ""lvgl.h""")
    $writer.WriteLine("#else")
    $writer.WriteLine("    #include ""lvgl/lvgl.h""")
    $writer.WriteLine("#endif")
    $writer.WriteLine("")
    $writer.WriteLine("#ifndef LV_ATTRIBUTE_MEM_ALIGN")
    $writer.WriteLine("#define LV_ATTRIBUTE_MEM_ALIGN")
    $writer.WriteLine("#endif")
    $writer.WriteLine("")
    $writer.WriteLine("#ifndef LV_ATTRIBUTE_IMG_PET_DOG")
    $writer.WriteLine("#define LV_ATTRIBUTE_IMG_PET_DOG")
    $writer.WriteLine("#endif")
    $writer.WriteLine("")
    $writer.WriteLine("/*")
    $writer.WriteLine(" * Generated from assets/pet/dog_medium.png.")
    $writer.WriteLine(" * Source: https://opengameart.org/content/dog-3")
    $writer.WriteLine(" * Author: rmazanek, License: CC0.")
    $writer.WriteLine(" * Frame size: 60 x 38 px, 6 columns x 6 rows.")
    $writer.WriteLine(" */")
    $writer.WriteLine("")

    foreach ($frame in $frames) {
        $mapName = "$($frame.Name)_map"
        $writer.WriteLine("const LV_ATTRIBUTE_MEM_ALIGN LV_ATTRIBUTE_LARGE_CONST LV_ATTRIBUTE_IMG_PET_DOG uint8_t $mapName[] = {")
        $writer.WriteLine("#if LV_COLOR_DEPTH == 16 && LV_COLOR_16_SWAP == 0")
        $writer.WriteLine("  /*Pixel format: Alpha 8 bit, Red: 5 bit, Green: 6 bit, Blue: 5 bit*/")
        Write-FrameData $writer $frame $false
        $writer.WriteLine("#endif")
        $writer.WriteLine("#if LV_COLOR_DEPTH == 16 && LV_COLOR_16_SWAP != 0")
        $writer.WriteLine("  /*Pixel format: Alpha 8 bit, Red: 5 bit, Green: 6 bit, Blue: 5 bit BUT the 2 color bytes are swapped*/")
        Write-FrameData $writer $frame $true
        $writer.WriteLine("#endif")
        $writer.WriteLine("};")
        $writer.WriteLine("")
        $writer.WriteLine("const lv_img_dsc_t $($frame.Name) = {")
        $writer.WriteLine("  .header.cf = LV_IMG_CF_TRUE_COLOR_ALPHA,")
        $writer.WriteLine("  .header.always_zero = 0,")
        $writer.WriteLine("  .header.reserved = 0,")
        $writer.WriteLine("  .header.w = $frameWidth,")
        $writer.WriteLine("  .header.h = $frameHeight,")
        $writer.WriteLine("  .data_size = $($frameWidth * $frameHeight) * LV_IMG_PX_SIZE_ALPHA_BYTE,")
        $writer.WriteLine("  .data = $mapName,")
        $writer.WriteLine("};")
        $writer.WriteLine("")
    }
}
finally {
    $writer.Dispose()
    $bmp.Dispose()
}

Get-Item $output | Select-Object FullName, Length, LastWriteTime
