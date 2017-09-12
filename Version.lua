local function Version(major, minor, patch, note)
	local v = {}
	
	v.major = major or 0
	v.minor = minor or 0
	v.patch = patch or 1
	
	if note then
		v.note = "-" .. note
	else
		v.note = ""
	end
	
	return setmetatable(v,
	{
		__tostring = function(v)
			return v.major .. "." .. v.minor .. "." .. v.patch .. v.note
		end,
	})
end

return Version
