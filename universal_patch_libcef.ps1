## This will patch raw controller input out of libcef on both 32bit and 64bit Windows 10.
## We do this by downloading a version of libcef that is similar in age to the one that ships with GFE.
## Make sure NVIDIA GeForce Experience overlay is disabled before running this script.
## PowerShell must be run as Administrator.

if ($Host.UI.PromptForChoice('Replacing libcef with patched known build',
	'Are you sure you want to proceed?', @('&Yes'; '&No'), 1) -eq 0) {

	# do everything in a temporary directory
	New-TemporaryFile | %{ rm $_; mkdir $_; cd $_; $tempdir = $_ } 

	$x64 = [Environment]::Is64BitOperatingSystem

	# get libcef of approximately the right vintage
	Write-Output "Downloading ..."
	$wc = New-Object System.Net.WebClient

	$wc.DownloadFile("https://cef-builds.spotifycdn.com/cef_binary_73.1.13%2Bg6e3c989%2Bchromium-73.0.3683.75_windows$(('32', '64')[$x64])_minimal.tar.bz2", "$pwd\libcef.tar.bz2")

	# get bzip2 because Microsoft doesn't ship it
	$wc.DownloadFile("https://github.com/philr/bzip2-windows/releases/download/v1.0.8.0/bzip2-1.0.8.0-win-$(('x86', 'x64')[$x64]).zip", "$pwd\bzip2.zip")
	Expand-Archive bzip2.zip

	Write-Output "Decompressing ..."
	bzip2\bzip2.exe -d libcef.tar.bz2
	tar -xf libcef.tar

	Write-Output "Patching ..."
	$file = [System.IO.File]::ReadAllBytes("$pwd\cef_binary_73.1.13+g6e3c989+chromium-73.0.3683.75_windows$(('32', '64')[$x64])_minimal\Release\libcef.dll")
	$offset = (0x53896fc, 0x60900b8)[$x64]

	$file[$offset] = 6    # Replace usage 0x04 (Joystick) with keyboard
	$file[$offset+2] = 6  # Replace usage 0x05 (Gamepad) with keyboard
	$file[$offset+4] = 6  # Replace usage 0x08 (Multi-axis Controller) with keyboard

	Write-Output "Saving ..."
	$path = Split-Path ((Get-ItemProperty -Path Registry::HKEY_CLASSES_ROOT\GeForceExperience\Shell\Open\Command).'(default)' -replace '"',"")
	Move "$path\libcef.dll" "$path\libcef.dll.bak"
	[System.IO.File]::WriteAllBytes("$path\libcef.dll", $file)

	# clean up
	Write-Output "Cleaning up."
	cd ~; del $tempdir -Recurse -Force
}

