10 REM INPUT/OUTPUT AND FORMATTING TESTS
20 PRINT "=== I/O TESTS ==="
30 REM Test PRINT formatting
40 PRINT "Basic print"
50 PRINT "Number: "; 42
60 PRINT "String: "; "HELLO"
70 PRINT "Mixed: "; 42; " and "; "HELLO"
80 REM Test semicolon vs comma
90 PRINT "Semi"; "colon"; "test"
100 PRINT "Comma", "test", "here"
110 REM Test TAB function
120 PRINT "TAB test:"
130 PRINT TAB(5); "5"
140 PRINT TAB(10); "10"
150 PRINT TAB(15); "15"
160 REM Test multiple TABs
170 PRINT "A"; TAB(10); "B"; TAB(20); "C"
180 REM Test PRINT with no arguments
190 PRINT
200 PRINT "After blank line"
210 REM Test DATA and READ
220 PRINT "=== DATA/READ TESTS ==="
230 READ A, B$, C
240 PRINT "Read: "; A; " "; B$; " "; C
250 READ D, E$
260 PRINT "Read: "; D; " "; E$
270 REM Test RESTORE
280 RESTORE
290 READ X
300 PRINT "After RESTORE: "; X
310 REM Test INPUT (commented out for automated testing)
320 REM PRINT "Enter a number: ";
330 REM INPUT N
340 REM PRINT "You entered: "; N
350 END
1000 DATA 123, "TEST", 456.789
1010 DATA 999, "LAST"
