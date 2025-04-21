7z l pandora.zip -ba -slt | sls "Path = (.+)" | % {$_.Matches.Groups.Value[1]} | % {"$_`r`n`u{200b}"} | Out-File filelist.txt
