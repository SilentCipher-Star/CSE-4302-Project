$src = "e:\dbms practice\CSE-4302-Project\src\appmanager_new.cpp"
$dst = "e:\dbms practice\CSE-4302-Project\src\appmanager.cpp"
$content = [System.IO.File]::ReadAllBytes($src)
$fs = [System.IO.FileStream]::new($dst, [System.IO.FileMode]::Create, [System.IO.FileAccess]::Write, [System.IO.FileShare]::ReadWrite)
$fs.Write($content, 0, $content.Length)
$fs.Close()

$src2 = "e:\dbms practice\CSE-4302-Project\src\mainwindow_new.cpp"
$dst2 = "e:\dbms practice\CSE-4302-Project\src\mainwindow.cpp"
$content2 = [System.IO.File]::ReadAllBytes($src2)
$fs2 = [System.IO.FileStream]::new($dst2, [System.IO.FileMode]::Create, [System.IO.FileAccess]::Write, [System.IO.FileShare]::ReadWrite)
$fs2.Write($content2, 0, $content2.Length)
$fs2.Close()

Write-Host "Done"
