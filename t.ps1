ForEach ($question in Get-ChildItem ".\questions" -Filter *.txt) {
  $time = Measure-Command { Get-Content($question.FullName) | x64\Release\tiler.exe | Out-File("answers\" + $question.Name) }

  # Write-Output($time.TotalSeconds)
}
