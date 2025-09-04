10 REM Comprehensive string array test
20 DIM A$(3)
30 DIM B(3)
40 LET L$="ABC"
50 FOR I=1 TO 3
60 LET B(I)=I
70 A$(I)=MID$(L$,B(I),1)
80 NEXT I
90 REM Test string array access in expressions
100 IF A$(1)="A" THEN PRINT "String array comparison works"
110 REM Test printing string arrays
120 FOR I=1 TO 3
130 PRINT A$(I);
140 NEXT I
150 PRINT
160 REM Test assignment from string array to string variable
170 LET X$=A$(2)
180 PRINT "X$=";X$
190 END
