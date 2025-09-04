10 REM Test MID$() function
20 LET A$ = "HELLO"
30 LET B$ = "WORLD"
40 PRINT "Testing MID$() function:"
50 PRINT "A$ = "; A$
60 PRINT "B$ = "; B$
70 PRINT "MID$(A$,2,3) = "; MID$(A$,2,3)
80 PRINT "MID$(B$,1,2) = "; MID$(B$,1,2)
90 IF MID$(A$,1,1) = "H" THEN PRINT "First char of A$ is H - CORRECT"
100 IF MID$(B$,5,1) = "D" THEN PRINT "Last char of B$ is D - CORRECT"
110 IF MID$(A$,2,1) = MID$(B$,2,1) THEN PRINT "2nd chars match"
115 IF MID$(A$,2,1) <> MID$(B$,2,1) THEN PRINT "2nd chars don't match"
120 END
