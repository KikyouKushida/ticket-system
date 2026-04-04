$groups = @(
  @{name='basic_3'; ids=3..7},
  @{name='basic_4'; ids=8..12},
  @{name='basic_5'; ids=13..22},
  @{name='basic_6'; ids=23..32},
  @{name='basic_extra'; ids=33..42},
  @{name='pressure_1_easy'; ids=43..52},
  @{name='pressure_2_easy'; ids=53..62},
  @{name='pressure_3_easy'; ids=63..72},
  @{name='pressure_1_hard'; ids=73..82},
  @{name='pressure_2_hard'; ids=83..92},
  @{name='pressure_3_hard'; ids=93..102}
)
foreach ($g in $groups) {
  Remove-Item -Recurse -Force data -ErrorAction SilentlyContinue
  Write-Output "GROUP $($g.name)"
  foreach ($id in $g.ids) {
    cmd /c "code.exe < ..\testcases\$id.in > out.tmp"
    if ($LASTEXITCODE -ne 0) {
      Write-Output "RUNTIME_FAIL $id"
      exit 1
    }
    cmd /c "fc /N out.tmp ..\testcases\$id.out" > $null
    if ($LASTEXITCODE -ne 0) {
      Write-Output "DIFF_FAIL $id"
      exit 1
    }
  }
  Write-Output "PASS $($g.name)"
}
Write-Output "ALL_PASS"
