local s1 = "#abc"
local s2 = "/abc"
local s3 = "-abc"
local s4 = " ab/c"
local s5 = "ab/c"
print(string.find(s1,"^%p"))
print(string.find(s2,"^%p"))
print(string.find(s3,"^%p"))
print(string.find(s4,"^%p"))
print(string.find(s5,"^%p"))
  