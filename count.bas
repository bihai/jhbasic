def printone(n)
 print n; " ";
end def

print "Count to 100"
print
for n = 1 to 100
 printone(n)
 if n mod 10 = 0 then
   print
 end if
next n
print
print "DONE!"
