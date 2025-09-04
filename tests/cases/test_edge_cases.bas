10 REM EDGE CASE TESTS
20 PRINT "=== EDGE CASE TESTS ==="
30 REM Test zero division
40 ON ERROR GOTO 100
50 PRINT "Testing division by zero..."
60 X = 5/0
70 PRINT "This should not print"
80 GOTO 110
100 PRINT "Division by zero handled"
110 REM Test array bounds
120 DIM A(5)
130 A(0) = 1: A(5) = 6
140 PRINT "A(0)="; A(0); " A(5)="; A(5)
150 REM Test empty string operations
160 A$ = ""
170 PRINT "Empty string length: "; LEN(A$)
180 REM Test very large numbers
190 X = 999999
200 Y = X * X
210 PRINT "Large number test: "; Y
220 REM Test negative array indices (should error)
230 REM A(-1) = 5
240 REM Test string concatenation edge cases
250 A$ = "A": B$ = "B"
260 C$ = A$ + B$
270 PRINT "Concatenation: "; C$
280 REM Test multiple statements per line
290 A=1: B=2: C=A+B: PRINT "Multiple statements: "; C
300 REM Test GOTO to non-existent line
310 REM GOTO 9999
320 END
