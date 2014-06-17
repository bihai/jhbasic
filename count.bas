def printone(n)
 print n; " ";
end def

print "Count to 10000"
for n = 0 to 10000
 if n mod 100 = 0 then
   print
 end if
 printone(n)
next n
print
print "DONE!"