$Repo = "PPeter2/CoreX"
$InstallDir = "$env:USERPROFILE\.corex"
$BinDir = "$InstallDir\bin"
$Asset = "corex-windows-x64.exe"

function Get-LatestReleaseUrl {
    $ApiUrl = "https://api.github.com/repos/$Repo/releases/latest"
    $Response = Invoke-RestMethod -Uri $ApiUrl -Headers @{ "User-Agent" = "corex-installer" }
    $AssetEntry = $Response.assets | Where-Object { $_.name -eq $Asset }

    if (-not $AssetEntry) {
        Write-Host "corex install: could not find a release asset named '$Asset'"
        Write-Host "corex install: check that $Repo has a published release"
        exit 1
    }

    return $AssetEntry.browser_download_url
}

function Install-Binary($DownloadUrl) {
    New-Item -ItemType Directory -Force -Path $BinDir | Out-Null
    Write-Host "corex install: downloading $Asset"
    Invoke-WebRequest -Uri $DownloadUrl -OutFile "$BinDir\corex.exe"
    Write-Host "corex install: installed to $BinDir\corex.exe"
}

function Update-Path {
    $CurrentPath = [Environment]::GetEnvironmentVariable("Path", "User")

    if ($CurrentPath -notlike "*$BinDir*") {
        $NewPath = "$CurrentPath;$BinDir"
        [Environment]::SetEnvironmentVariable("Path", $NewPath, "User")
        Write-Host "corex install: added $BinDir to your user PATH"
        Write-Host "corex install: restart your terminal for this to take effect"
    }
}

$DownloadUrl = Get-LatestReleaseUrl
Install-Binary -DownloadUrl $DownloadUrl
Update-Path

Write-Host "corex install: done. Run 'corex version' after restarting your terminal."
