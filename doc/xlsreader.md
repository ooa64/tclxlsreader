# tclxlsreader

```
package require xlsreader
foreach sheet [xlsreader read [lindex $argv 0]] {
    puts "# sheet [incr sheetno]"
    foreach row $sheet {
        puts [join $row ";"]
    }
}
```