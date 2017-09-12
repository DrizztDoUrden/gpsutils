local function LibHelper(name)
	return {
		Include = function()
			includedirs(srcRoot .. "/" .. name .. "/include")
		end,
		Link = function(self)
			self:Include()
			links(name)
		end,
	}
end

return LibHelper
