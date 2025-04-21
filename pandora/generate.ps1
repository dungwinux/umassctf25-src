function Craft-SparseToSize {
    param (
        [string] $FileName,
        [int] $Size=1099511627776
    )
    <#
    .SYNOPSIS
    Turn file into sparse file of specific size
    .DESCRIPTION
    Turn file into sparse file of specific size. The filesystem must support sparse file.
    .EXAMPLE
    Craft-SparseToSize -FileName pg100.txt
    #>
    
    $ErrorActionPreference = 'Stop'
    $OutName = $FileName + '.sparse'
    $CurrentSize = (gci -LiteralPath $FileName).Length
    $d='04/01/2025 13:37:00'
    
    if ($CurrentSize -gt $Size) {
        Write-Error "Size must be larger than original size"
        return
    }

    cp $FileName $OutName
    Test-Path -LiteralPath $OutName -PathType Leaf
    $OutFile = gci -LiteralPath $OutName
    
    fsutil sparse setflag $OutName
    fsutil sparse setrange $OutName $CurrentSize ($Size - $CurrentSize)
    fsutil file seteof $OutName $Size
    $OutFile.CreationTime=$OutFile.LastAccessTime=$OutFile.LastWriteTime=$d
    return $OutFile
}
