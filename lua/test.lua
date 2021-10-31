param="on -l a 1 2 3"
d, m, n, o = string.match(param,"-l (%w+) (%d+) (%d+) (%d+)")
print(d,m,n,o)
t={}
t[1]="a"
t[2]="b"
t[3]="c"
local m = nil
    for  n , value in pairs(t) do
      if value == "a" then
        m =  n
      end
    end
    print(m)
