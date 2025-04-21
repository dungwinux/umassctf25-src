# Pandora

- `static/`: deployed file
- `generate_filelist.ps1`: Create file list for renaming using `7z rn archive.zip @filelist.txt`
- `generate.ps1`: Script for creating each entry (NTFS/ReFS and Windows required)
- `gutenberg.txt`: List of gutenberg ebooks used
- `partial_solve.py`: Python script to cat zip output until NULL
- `solve.ps1`: PowerShell script to get flag (ziptool, busybox needed)
- `solve.sh`: Bash script to get flag (ziptool needed)
