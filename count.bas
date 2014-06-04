def printone(n)
 print n; " ";
end def

print "Count to 100"
for n = 0 to 100
 if n mod 10 = 0 then
   print
 end if
 printone(n)
next n
print
