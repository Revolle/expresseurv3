function dump(o, indent)
    indent = indent or ""
    if type(o) == "table" then
        local s = "{\n"
        for k, v in pairs(o) do
            s = s .. indent .. "  [" .. tostring(k) .. "] = " .. dump(v, indent .. "  ") .. ",\n"
        end
        return s .. indent .. "}"
    else
        return tostring(o)
    end
end


local t = {}
print(t[3])
t[3] = {}
t[3].i = 5
t[3].p = 8
t[3]["c"] = 5
print(dump(t))


