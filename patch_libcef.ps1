## Make sure NVIDIA GeForce Experience overlay is disabled before running this script.
## PowerShell must be run as Administrator.
## This was tested against GeForce Experience:
##  - 3.20.5.70
##  - 3.20.5.83

$path = Split-Path ((Get-ItemProperty -Path Registry::HKEY_CLASSES_ROOT\GeForceExperience\Shell\Open\Command).'(default)' -replace '"',"")
$file = [System.IO.File]::ReadAllBytes("$path\libcef.dll")
$offset = 0x61e0ae8
if($file[$offset] -eq 4 -and $file[$offset+2] -eq 5 -and $file[$offset+4] -eq 8) {
	Stop-Process -Name "NVIDIA GeForce Experience" -ErrorAction SilentlyContinue
	Stop-Process -Name "NVIDIA Share" -ErrorAction SilentlyContinue
	[System.IO.File]::WriteAllBytes("$path\libcef.dll.bak", $file)
	$file[$offset] = 6    # Replace usage 0x04 (Joystick) with keyboard
	$file[$offset+2] = 6  # Replace usage 0x05 (Gamepad) with keyboard
	$file[$offset+4] = 6  # Replace usage 0x08 (Multi-axis Controller) with keyboard
	[System.IO.File]::WriteAllBytes("$path\libcef.dll", $file)
} else {
	Write-Output "Your libcef.dll doesn't seem to match the version this file was tested against."
	Write-Output "Maybe you want to try https://github.com/nuzayets/rawinput-debug/blob/master/universal_patch_libcef.ps1"
}
