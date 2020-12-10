using namespace System.Collections.Generic

## This will patch raw controller input out of libcef on both 32bit and 64bit Windows 10.
## We do this by magic. It's known to work with libcef.dll newer than Feb 2018 or so. 
## Make sure NVIDIA GeForce Experience overlay is disabled before running this script.
## PowerShell must be run as Administrator.

$path = Split-Path ((Get-ItemProperty -Path Registry::HKEY_CLASSES_ROOT\GeForceExperience\Shell\Open\Command).'(default)' -replace '"',"")
$filepath = "$path\libcef.dll"
Stop-Process -Name "NVIDIA GeForce Experience" -ErrorAction SilentlyContinue
Stop-Process -Name "NVIDIA Share" -ErrorAction SilentlyContinue
$s = 0x100000
$al =  [List[byte]]::new([System.IO.File]::ReadAllBytes($filepath))
$ff = $al.Count
$hl = [List[byte]]::new($s)
$e = [System.Text.Encoding]::GetEncoding(28591)

function ran {
    param($n)
    $rhb = $e.GetString($hl)
    $rt = $e.GetString($n)
    $lx = [Regex]::Match($rhb, ([Regex]::Escape($rt)))
    return $lx
}

function run {
    param($j,$k)
    for ($p=($al.Count - $s - 1);$p -ge 0; $p = $p - $s) {
        $hl.AddRange([Linq.Enumerable]::Select($al.GetRange($p,$s-1), $k))
        $y = ran $j
        $g = ($ff - $p) / [double]$ff * 100
        Write-Progress -Activity "Running..." -Status "$g% Complete:" -PercentComplete $g;
        if ($y.Success -ne $false) { return $y.Index + $p }
        $hl.Clear()
    }
    return $null
}

function runs {
    param($zz, $kk)
    $stopwatch =  [system.diagnostics.stopwatch]::StartNew()
    $l1 = run $zz $kk
    $stopwatch.Stop()
    if ($l1 -ne $null) { Write-Information "$([math]::Round(($ff - $l1)/1024.0/$stopwatch.Elapsed.TotalSeconds,0))kB/s" -InformationAction Continue }
    return $l1
}


[Byte[]]$a = (58,61,35,38,39,12,55,50,39,50,12,53,54,39,48,59,54,33,12,36,58,61,125,48,48,79,20,54,39,1,50,36,26,61,35,38,39,23,54,37,58,48,54,31,58,32,39,123,122,115,53,50,58,63,54,55,79) 
[Func[Byte,Byte]] $b = { param([Byte]$d); return (0x4F, ($d, [Byte]($d -bxor 0x53))[[bool][Byte]($d -bxor 83)])[[bool]$d] };
$retries = 1
while ($retries -ge 0) {
    $l1 = runs $a $b
    if ($l1 -ne $null) {
        Write-Information ("FOUND l1: 0x{0,0:x}" -f $l1) -InformationAction Continue
        break
    } else {
        $s = $s * 2
        $retries = $retries - 1
        if ($retries -lt 0) {
            throw "We tried..."
        }
    }
}


$l1 = $l1 + (4 - ($l1 % 0x4))
$k = ((31, 137, 248),(115, 253, 195))

function annoy {
    param ($a, $c)
    [Func[Byte,Byte]]$b = { param([byte]$d); return ($d -bxor $c[0][0]) -bxor ($d -bxor $c[0][1]) -bxor ($d -bxor $c[0][2]) }
    [byte[]]$d1 = [Linq.Enumerable]::Select([byte[]]$a[0..2], $b)
    [Func[Byte,Byte]]$b = { param([byte]$d); return ($d -bxor $c[1][0]) -bxor ($d -bxor $c[1][1]) -bxor ($d -bxor $c[1][2]) }
    [byte[]]$d2 = [Linq.Enumerable]::Select([byte[]]$a[3..5], $b)
    return ,@($d1,$d2 | % {$_})
}

function conquer {
    for ($i=0;$i -le 100;$i++) {
        $l2 = $l1 - ($i*4)
        $b = $al[$l2..($l2+6)]
        if ($e.getString((annoy $b $k)) -eq 'jnkMEM') {
            return $l2
        }
    }
    return $null
}

$l2 = conquer
if ($l2 -ne $null) {
    Write-Information ("FOUND l2: 0x{0,0:x}" -f $l2) -InformationAction Continue   
    $al[$l2] = 6    # Replace usage 0x04 (Joystick) with keyboard
    $al[$l2+2] = 6  # Replace usage 0x05 (Gamepad) with keyboard
    $al[$l2+4] = 6  # Replace usage 0x08 (Multi-axis Controller) with keyboard
    Move "$path\libcef.dll" "$path\libcef.dll.bak"
    [System.IO.File]::WriteAllBytes("$path\libcef.dll", $al)
    Write-Output "All done."
} else {
    throw "No bueno."
}

